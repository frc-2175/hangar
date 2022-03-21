#pragma once

#include "cad.h"

typedef struct Serialized {
    const char *Data;
    int Len;
} Serialized;

const char *Parts2JSON(Part *parts, int numParts);
Serialized Serialize(Part *parts, int numParts);

void SaveWIP(Serialized data);
bool HasWIP();
void LoadWIP(Part *parts, int *numParts);
void ClearWIP();

void SaveQuery(Serialized data);
bool HasQuery();
void LoadQuery(Part *parts, int *numParts);
void ClearQuery();
