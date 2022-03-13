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
