#pragma once

#include "raylib.h"

#define MAX_BOXES 128

typedef enum {
    Aluminum,
    Polycarb,
} RobotMaterial;

typedef enum BoxDragMode {
    Translating = 1,
    Rotating,
} BoxDragMode;

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
    BoxDragMode DragMode;
    BoxUIField SelectedField;
    char TextInputBuf[BOX_TEXT_INPUT_MAX];
} Box;

typedef struct {
    int Depth; // used to find children without doing annoying tree work
    
    Box Boxes[MAX_BOXES];
    int NumBoxes;

    Vector3 Translation;
    float Angle;

    Vector3 Origin;
    Vector3 CenterOfMass;

    bool LockX;
    bool LockY;
    bool LockZ;
    bool LockRotation;
} Part;

float BoxMass(Box box);
void UpdatePartCOM(Part *part);
