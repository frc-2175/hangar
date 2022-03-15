#pragma once

#include "cad.h"

const char* Parts2JSON(Part *parts, int numParts);

void SaveJSONWIP(const char *json);
bool HasWIP();
void LoadWIP(Part *parts, int *numParts);
void ClearWIP();

void SaveJSONQuery(const char *json);
bool HasQuery();
void LoadQuery(Part *parts, int *numParts);
void ClearQuery();
