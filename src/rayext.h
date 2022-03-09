#pragma once

#include "raylib.h"

typedef struct RectanglePoints {
    Vector2 TopLeft;
    Vector2 TopRight;
    Vector2 BottomLeft;
    Vector2 BottomRight;
} RectanglePoints;

RectanglePoints GetRectanglePointsPro(Rectangle rec, Vector2 origin, float rotation);
bool CheckCollisionPointRecPro(Vector2 point, Rectangle rec, Vector2 origin, float rotation);
