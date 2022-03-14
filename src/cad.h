#pragma once

#include "raylib.h"
#include "rayext.h"

#define PART_NAME_LENGTH 128
#define MAX_BOXES 128

typedef enum {
    Aluminum,
    Polycarb,
    Steel,
} RobotMaterial;

typedef enum BoxUIField {
    WidthField = 1,
    HeightField,

    HardcodedMassField,
} BoxUIField;

typedef enum PartUIField {
    NameField = 1,
} PartUIField;

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
    GuiDropdownBoxExState MaterialDropdown;
    float HardcodedMass; // 0 == automatically calculate
    GuiNumberTextBoxExState HardcodedMassTextBox;

    // UI stuff weeeeee!
    BoxUIField SelectedField;
    char TextInputBuf[BOX_TEXT_INPUT_MAX];
    char DraggingPosition;
    char DraggingRotation;

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

    Vector2 AttachmentPoint;
    Vector2 CenterOfMass;

    bool LockX;
    bool LockY;
    bool LockZ;
    bool LockRotation;

    PartUIField SelectedField;
    char DraggingPosition;
    char DraggingRotation;
    char DraggingCenterOfRotation;
    char DraggingAttachmentPoint;
} Part;

Vector2 World2Part(Part part, Vector2 v);
Vector2 Part2World(Part part, Vector2 v);
float WorldAngle2PartAngle(Part part, float angle);
float PartAngle2WorldAngle(Part part, float angle);
Vector2 World2Attachment(Vector2 attachPos, Vector2 COM, Vector2 v);
float WorldAngle2AttachmentAngle(Vector2 attachPos, Vector2 COM, float angle);

float BoxMass(Box box);
void UpdateReferences(Part *part);
Vector2 ComputeCOM(Part *parts, int numParts);
void UpdatePartCOM(Part *part);
