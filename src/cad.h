#pragma once

#include "raylib.h"

typedef enum {
    Aluminum,
    Polycarb,
} RobotMaterial;

typedef struct {
    float X;
    float Y;
    float Z;
    // TODO: Normalize X and Y to the upper left of the entire part?

    float Width;
    float Height;
    float Depth;
    float WallThickness; // 0 == solid

    float Angle; // degrees!

    RobotMaterial Material;
    float HardcodedMass; // 0 == automatically calculate
} Box;

#define MAX_BOXES 128

typedef struct {
    Box Boxes[MAX_BOXES];
    int NumBoxes;

    Vector3 CenterOfMass;
} Part;

float BoxMass(Box box);
void UpdatePartCOM(Part *part);
