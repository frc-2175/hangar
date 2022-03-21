#include "serialization.h"

#include <assert.h>
#include <emscripten/emscripten.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "jsmn.h"

#define SERIALIZED_MAX_LENGTH 999999 // I sure hope we stay under this aaaa

char serializeBuf[SERIALIZED_MAX_LENGTH+1];

const char* v2(Vector2 v) {
    return TextFormat("[%f,%f]", v.x, v.y);
}

void write(int *c, const char *str) {
    assert(*c < SERIALIZED_MAX_LENGTH);
    *c += TextCopy(&serializeBuf[*c], str);
}

void writeN(int *c, void *str, int n) {
    assert(*c < SERIALIZED_MAX_LENGTH);
    memcpy(&serializeBuf[*c], str, n);
    *c += n;
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

    return serializeBuf;
}

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

// Loads the old JSON format. Which was massive, and actually a huge mistake. Yay!
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

enum : int {
    V_Initial = 1,
    // Add more versions here whenever the file format changes

    // Don't remove this
    _V_LatestPlusOne
};

#define V_LATEST (_V_LatestPlusOne - 1)

#define ADD(_t, _fieldAdded, _field) \
    if (s->DataVersion >= (_fieldAdded)) { \
        serialize_##_t(s, (_field)); \
    }

typedef struct serializer {
    char *Data;
    int DataVersion;
    bool IsWriting;
    int C;
} serializer;

void swriteN(serializer *s, const void *src, int n) {
    assert(s->C < SERIALIZED_MAX_LENGTH);
    memcpy(&s->Data[s->C], src, n);
    s->C += n;
}

void swrite(serializer *s, const char *str) {
    swriteN(s, str, strlen(str) + 1);
}

void sreadN(serializer *s, void *dst, int n) {
    memcpy(dst, &s->Data[s->C], n);
    s->C += n;
}

void sread(serializer *s, char *dst) {
    sreadN(s, dst, strlen(&s->Data[s->C]) + 1);
}

void serialize_int(serializer *s, int *value) {
    if (s->IsWriting) {
        swriteN(s, value, sizeof(int));
    } else {
        sreadN(s, value, sizeof(int));
    }
}

void serialize_float(serializer *s, float *value) {
    if (s->IsWriting) {
        swriteN(s, value, sizeof(float));
    } else {
        sreadN(s, value, sizeof(float));
    }
}

void serialize_str(serializer *s, char *value) {
    if (s->IsWriting) {
        swrite(s, value);
    } else {
        sread(s, value);
    }
}

void serialize_Vector2(serializer *s, Vector2 *value) {
    serialize_float(s, &value->x);
    serialize_float(s, &value->y);
}

void serialize_RobotMaterial(serializer *s, RobotMaterial *value) {
    serialize_int(s, (int *)value);
}

void serialize_Box(serializer *s, Box *box) {
    ADD(Vector2, V_Initial, &box->Position);
    ADD(float, V_Initial, &box->Angle);
    ADD(float, V_Initial, &box->Width);
    ADD(float, V_Initial, &box->Height);
    ADD(float, V_Initial, &box->Depth);
    ADD(float, V_Initial, &box->WallThickness);
    ADD(int, V_Initial, &box->Quantity);
    ADD(RobotMaterial, V_Initial, &box->Material);
    ADD(float, V_Initial, &box->HardcodedMass);
}

void serialize_Part(serializer *s, Part *part) {
    ADD(int, V_Initial, &part->Depth);
    ADD(str, V_Initial, part->Name);
    ADD(Vector2, V_Initial, &part->Position);
    ADD(float, V_Initial, &part->Angle);
    ADD(Vector2, V_Initial, &part->AttachmentPoint);

    ADD(int, V_Initial, &part->NumBoxes);
    // printf("%s: Part %s has %d boxes.\n", s->IsWriting ? "WRITING" : "READING", part->Name, part->NumBoxes);
    for (int b = 0; b < part->NumBoxes; b++) {
        serialize_Box(s, &part->Boxes[b]);
    }
}

void serialize_Parts(serializer *s, Part *parts, int *numParts) {
    ADD(int, V_Initial, numParts);
    // printf("%s: There are %d parts.\n", s->IsWriting ? "WRITING" : "READING", *numParts);

    for (int i = 0; i < *numParts; i++) {
        serialize_Part(s, &parts[i]);
    }
}

void serializeAll(serializer *s, Part *parts, int *numParts) {
    if (s->IsWriting) {
        s->DataVersion = V_LATEST;
    }

    serialize_int(s, &s->DataVersion);
    assert(s->DataVersion <= V_LATEST); // ensure we are not old code reading a new file
    
    serialize_Parts(s, parts, numParts);
}

Serialized Serialize(Part *parts, int numParts) {
    serializer s = {
        .Data = serializeBuf,
        .IsWriting = true,
    };
    serializeAll(&s, parts, &numParts);

    return (Serialized) {
        .Data = s.Data,
        .Len = s.C,
    };
}

void loadBinary(const char *data, Part *parts, int *numParts) {
    serializer s = {
        .Data = data,
        .IsWriting = false,
    };
    serializeAll(&s, parts, numParts);
}

void loadSerialized(const char *data, Part *parts, int *numParts) {
    if (!data) {
        return;
    }

    if (data[0] == '{') {
        // Load the old JSON format
        loadJSON(data, parts, numParts);
    } else {
        // Load the new binary format
        loadBinary(data, parts, numParts);
    }
}

EM_JS(void, saveWIP, (const char *dataPtr, int len), {
    const b64 = base64EncArr(Module.HEAPU8.subarray(dataPtr, dataPtr + len));
    window.localStorage.setItem('saved', b64);
});

void SaveWIP(Serialized data) {
    saveWIP(data.Data, data.Len);
}

EM_JS(bool, HasWIP, (), {
    return !!window.localStorage.getItem('saved');
});

EM_JS(char *, getWIPData, (), {
    const data = window.localStorage.getItem('saved');
    if (!data) {
        throw new Error('No WIP data was saved');
    }

    // original JSON format
    if (data[0] == '{') {
        return allocateUTF8(data);
    }

    // new binary format
    const wipDataArray = base64DecToArr(data);
    console.log("Loaded this data from WIP:", wipDataArray);
    const buf = Module._malloc(wipDataArray.length * wipDataArray.BYTES_PER_ELEMENT);
    Module.HEAPU8.set(wipDataArray, buf);
    return buf;
});

void LoadWIP(Part *parts, int *numParts) {
    assert(HasWIP());
    char *data = getWIPData();
    loadSerialized(data, parts, numParts);
    free(data);
}

EM_JS(void, ClearWIP, (), {
    window.localStorage.removeItem('saved');
});

EM_JS(void, saveQuery, (const char *dataPtr, int len), {
    const b64 = base64EncArr(Module.HEAPU8.subarray(dataPtr, dataPtr + len));
    const searchParams = new URLSearchParams(window.location.search);
    searchParams.set('robot', b64);
    const path = window.location.pathname + '?' + searchParams.toString();
    window.history.pushState(null, '', path);
});

void SaveQuery(Serialized data) {
    saveQuery(data.Data, data.Len);
}

EM_JS(bool, HasQuery, (), {
    return new URLSearchParams(window.location.search).has('robot');
});

EM_JS(char *, getQueryData, (), {
    const data = new URLSearchParams(window.location.search).get('robot');
    if (data[0] === 'e') {
        // JSON stuff begins with an e in base64. While this is technically possible to get other ways...who cares.
        return allocateUTF8(atob(new URLSearchParams(window.location.search).get('robot')));
    }

    const queryDataArray = base64DecToArr(data);
    console.log("Loaded this data from the query:", queryDataArray);
    const buf = Module._malloc(queryDataArray.length * queryDataArray.BYTES_PER_ELEMENT);
    Module.HEAPU8.set(queryDataArray, buf);
    return buf;
});

void LoadQuery(Part *parts, int *numParts) {
    assert(HasQuery());
    char *data = getQueryData();
    loadSerialized(data, parts, numParts);
    free(data);
}

EM_JS(void, ClearQuery, (), {
    window.history.pushState(null, '', window.location.pathname);
});
