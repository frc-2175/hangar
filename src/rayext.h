#pragma once

#include "raylib.h"
#include "raymath.h"

typedef struct RectanglePoints {
    Vector2 TopLeft;
    Vector2 TopRight;
    Vector2 BottomLeft;
    Vector2 BottomRight;

    Vector2 LeftMiddle;
    Vector2 RightMiddle;
    Vector2 TopMiddle;
    Vector2 BottomMiddle;

    Vector2 Center;
} RectanglePoints;

RectanglePoints GetRectanglePointsPro(Rectangle rec, Vector2 origin, float rotation);
bool CheckCollisionPointRecPro(Vector2 point, Rectangle rec, Vector2 origin, float rotation);

#define NUMBER_TEXT_BOX_BUF_LENGTH 32

typedef struct GuiNumberTextBoxExState {
    bool Active;
    char Buf[NUMBER_TEXT_BOX_BUF_LENGTH];
} GuiNumberTextBoxExState;

typedef struct GuiDropdownBoxExState {
    bool Open;
    int Active;
} GuiDropdownBoxExState;

void GuiNumberTextBoxEx(GuiNumberTextBoxExState *state, Rectangle rec, float *numPtr);
int GuiDropdownBoxEx(GuiDropdownBoxExState *state, Rectangle rec, const char* str);
