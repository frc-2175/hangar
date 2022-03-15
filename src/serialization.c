#include "serialization.h"

#include <assert.h>
#include <emscripten/emscripten.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "jsmn.h"

#define JSON_MAX_LENGTH 999999 // I sure hope we stay under this aaaa

char jsonBuf[JSON_MAX_LENGTH+1];

const char* v2(Vector2 v) {
    return TextFormat("[%f,%f]", v.x, v.y);
}

void write(int *c, const char* str) {
    assert(*c < JSON_MAX_LENGTH);
    *c += TextCopy(&jsonBuf[*c], str);
}

void writeBox(int *c, Box *box) {
    write(c, "{");

    write(c, TextFormat("\"Position\":%s,", v2(box->Position)));
    write(c, TextFormat("\"Angle\":%f,", box->Angle));
    write(c, TextFormat("\"Width\":%f,", box->Width));
    write(c, TextFormat("\"Height\":%f,", box->Height));
    write(c, TextFormat("\"Depth\":%f,", box->Depth));
    write(c, TextFormat("\"WallThickness\":%f,", box->WallThickness));
    write(c, TextFormat("\"Quantity\":%d,", box->Quantity));
    write(c, TextFormat("\"Material\":%d,", box->Material));
    write(c, TextFormat("\"HardcodedMass\":%f", box->HardcodedMass));

    write(c, "}");
}

void writePart(int *c, Part *part) {
    write(c, "{");
    
    write(c, TextFormat("\"Depth\":%d,", part->Depth));
    write(c, TextFormat("\"Name\":\"%s\",", part->Name));
    write(c, TextFormat("\"Position\":%s,", v2(part->Position)));
    write(c, TextFormat("\"Angle\":%f,", part->Angle));
    write(c, TextFormat("\"AttachmentPoint\":%s,", v2(part->AttachmentPoint)));
    
    write(c, "\"Boxes\":[");
    for (int b = 0; b < part->NumBoxes; b++) {
        if (b > 0) {
            write(c, ",");
        }
        writeBox(c, &part->Boxes[b]);
    }
    write(c, "]");
    
    write(c, "}");
}

const char* Parts2JSON(Part *parts, int numParts) {
    int c = 0;

    write(&c, "{\"parts\":[");
    for (int p = 0; p < numParts; p++) {
        if (p > 0) {
            write(&c, ",");
        }
        writePart(&c, &parts[p]);
    }
    write(&c, "]}");

    return jsonBuf;
}

EM_JS(void, SaveJSONWIP, (const char *jsonPtr), {
    const json = UTF8ToString(jsonPtr);
    window.localStorage.setItem('saved', json);
});

EM_JS(bool, HasWIP, (), {
    return window.localStorage.getItem('saved') !== null;
});

EM_JS(char *, getWIPJSON, (), {
    return allocateUTF8(window.localStorage.getItem('saved'));
});

#define MAX_JSON_TOKENS 2048

typedef struct JSONParser {
    const char *json;
    jsmn_parser p;
    jsmntok_t t[MAX_JSON_TOKENS];
    int c;
} JSONParser;

JSONParser parseJSON(const char *json) {
    JSONParser p = {
        .json = json,
    };
    jsmn_init(&p.p);
    int r = jsmn_parse(&p.p, json, strlen(json), p.t, MAX_JSON_TOKENS);
    assert(r >= 0);
    return p;
}

jsmntok_t expectType(JSONParser *p, jsmntype_t type) {
    jsmntok_t token = p->t[p->c];
    assert(token.type == type);
    p->c++;
    return token;
}

jsmntok_t expectStringLit(JSONParser *p, const char *str) {
    jsmntok_t token = p->t[p->c];
    // printf("expected string, got %s\n", &p->json[token.start]);
    assert(token.type == JSMN_STRING);
    int n = strlen(str);
    int res = strncmp(&p->json[token.start], str, n);
    // printf("%s, got %s\n", str, &p->json[token.start]);
    assert(res == 0);
    p->c++;
    return token;
}

jsmntok_t expectString(JSONParser *p, char *dest) {
    jsmntok_t token = p->t[p->c];
    assert(token.type == JSMN_STRING);
    memcpy(dest, &p->json[token.start], token.end - token.start);
    p->c++;
    return token;
}

int expectInt(JSONParser *p) {
    jsmntok_t token = p->t[p->c];
    assert(token.type == JSMN_PRIMITIVE);
    char first = p->json[token.start];
    assert(first == '-' || ('0' <= first && first <= '9'));
    int res = (int) strtol(&p->json[token.start], NULL, 10);
    p->c++;
    return res;
}

float expectFloat(JSONParser *p) {
    jsmntok_t token = p->t[p->c];
    assert(token.type == JSMN_PRIMITIVE);
    char first = p->json[token.start];
    assert(first == '-' || ('0' <= first && first <= '9'));
    float res = strtof(&p->json[token.start], NULL);
    p->c++;
    return res;
}

Vector2 expectVector2(JSONParser *p) {
    Vector2 res = {0};
    expectType(p, JSMN_ARRAY);
    res.x = expectFloat(p);
    res.y = expectFloat(p);
    return res;
}

void loadJSON(const char *json, Part *parts, int *numParts) {
    JSONParser p = parseJSON(json);

    expectType(&p, JSMN_OBJECT);
    expectStringLit(&p, "parts");
    jsmntok_t partsArray = expectType(&p, JSMN_ARRAY);
    *numParts = partsArray.size;
    for (int i = 0; i < partsArray.size; i++) {
        expectType(&p, JSMN_OBJECT);

        parts[i] = (Part){0};
        Part *part = &parts[i];
        
        // for now we cheat like mad and assume all the fields come in the same order we wrote them
        expectStringLit(&p, "Depth");
        part->Depth = expectInt(&p);
        expectStringLit(&p, "Name");
        expectString(&p, part->Name);
        expectStringLit(&p, "Position");
        part->Position = expectVector2(&p);
        expectStringLit(&p, "Angle");
        part->Angle = expectFloat(&p);
        expectStringLit(&p, "AttachmentPoint");
        part->AttachmentPoint = expectVector2(&p);
        
        expectStringLit(&p, "Boxes");
        jsmntok_t boxArray = expectType(&p, JSMN_ARRAY);
        part->NumBoxes = boxArray.size;
        for (int j = 0; j < boxArray.size; j++) {
            expectType(&p, JSMN_OBJECT);

            part->Boxes[j] = (Box){0};
            Box *box = &part->Boxes[j];

            expectStringLit(&p, "Position");
            box->Position = expectVector2(&p);
            expectStringLit(&p, "Angle");
            box->Angle = expectFloat(&p);
            expectStringLit(&p, "Width");
            box->Width = expectFloat(&p);
            expectStringLit(&p, "Height");
            box->Height = expectFloat(&p);
            expectStringLit(&p, "Depth");
            box->Depth = expectFloat(&p);
            expectStringLit(&p, "WallThickness");
            box->WallThickness = expectFloat(&p);
            expectStringLit(&p, "Quantity");
            box->Quantity = expectInt(&p);
            expectStringLit(&p, "Material");
            box->Material = expectInt(&p);
            expectStringLit(&p, "HardcodedMass");
            box->HardcodedMass = expectFloat(&p);
        }
    }
}

void LoadWIP(Part *parts, int *numParts) {
    assert(HasWIP());
    char *json = getWIPJSON();
    loadJSON(json, parts, numParts);
    free(json);
}
