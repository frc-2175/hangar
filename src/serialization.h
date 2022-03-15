#pragma once

#include "cad.h"

const char* Parts2JSON(Part *parts, int numParts);
void SaveJSON(const char *jsonPtr);
