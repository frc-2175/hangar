#include "serialization.h"

#include <assert.h>
#include <emscripten/emscripten.h>

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

EM_JS(void, SaveJSON, (const char *jsonPtr), {
    const json = UTF8ToString(jsonPtr);
    window.localStorage.setItem('saved', json);
});
