#pragma once

#include "raylib.h"

#define PART_NAME_LENGTH 128
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

/*
 * Quick rundown of the coordinates used by this app.
 *
 * Part coordinates are absolute; even when one is "nested"
 * under another, its coordinates do not change. We only
 * use this parent/child relationship when interactively
 * moving things.
 * 
 * Part center of rotation / attachment point are relative
 * to the part.
 * 
 * Box coordinates are relative to their part.
 * 
 * Center of mass is always in world space.
 */

typedef struct Box {
    Vector2 Position;
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

    // o no
    struct Part *Part;
} Box;

typedef struct Part {
    int Depth; // used to find children without doing annoying tree work
    
    char Name[PART_NAME_LENGTH];

    Box Boxes[MAX_BOXES];
    int NumBoxes;

    Vector2 Position;
    float Angle;

    Vector2 CenterOfRotation;
    Vector2 CenterOfMass;

    bool LockX;
    bool LockY;
    bool LockZ;
    bool LockRotation;

    char DraggingPosition;
    char DraggingCenterOfRotation;
} Part;

Vector2 World2Part(Part part, Vector2 v);
Vector2 Part2World(Part part, Vector2 v);

float BoxMass(Box box);
void UpdateReferences(Part *part);
void UpdatePartCOM(Part *part);
