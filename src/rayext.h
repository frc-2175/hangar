#pragma once

#include "raylib.h"
#include "raymath.h"

typedef struct RectanglePoints {
    Vector2 TopLeft;
    Vector2 TopRight;
    Vector2 BottomLeft;
    Vector2 BottomRight;

    Vector2 Center;
} RectanglePoints;

RectanglePoints GetRectanglePointsPro(Rectangle rec, Vector2 origin, float rotation);
bool CheckCollisionPointRecPro(Vector2 point, Rectangle rec, Vector2 origin, float rotation);

Vector2 V3V2(Vector3 v);
Vector3 V2V3(Vector2 v, float z);
