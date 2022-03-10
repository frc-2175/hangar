#pragma once

#include "raylib.h"

#define MAX_BOXES 128
#define MAX_ATTACHED 16

typedef enum {
    Aluminum,
    Polycarb,
} RobotMaterial;

typedef enum BoxUIField {
    WidthField = 1,
    HeightField,
} BoxUIField;

#define BOX_TEXT_INPUT_MAX 64

typedef struct {
    Vector3 Translation;
    float Angle; // degrees!
    // TODO: Normalize X and Y to the upper left of the entire part?
    // (Float jank is very unlikely to happen in this project...)

    float Width;
    float Height;
    float Depth;
    float WallThickness; // 0 == solid

    RobotMaterial Material;
    float HardcodedMass; // 0 == automatically calculate

    // UI stuff weeeeee!
    BoxUIField selectedField;
    char TextInputBuf[BOX_TEXT_INPUT_MAX];
} Box;

typedef struct {
    Vector3 Origin;
    Vector3 CenterOfMass;

    Vector3 Translation;
    float Angle;

    Box Boxes[MAX_BOXES];
    int NumBoxes;

    struct Part *Attached[MAX_ATTACHED];
    int NumAttached;
} Part;

float BoxMass(Box box);
void UpdatePartCOM(Part *part);
