#include "rayext.h"

#include <math.h>

// Does the math of DrawRectanglePro but returns the points instead.
RectanglePoints GetRectanglePointsPro(Rectangle rec, Vector2 origin, float rotation) {
    Vector2 topLeft = { 0 };
    Vector2 topRight = { 0 };
    Vector2 bottomLeft = { 0 };
    Vector2 bottomRight = { 0 };

    // Only calculate rotation if needed
    if (rotation == 0.0f)
    {
        float x = rec.x - origin.x;
        float y = rec.y - origin.y;
        topLeft = (Vector2){ x, y };
        topRight = (Vector2){ x + rec.width, y };
        bottomLeft = (Vector2){ x, y + rec.height };
        bottomRight = (Vector2){ x + rec.width, y + rec.height };
    }
    else
    {
        float sinRotation = sinf(rotation*DEG2RAD);
        float cosRotation = cosf(rotation*DEG2RAD);
        float x = rec.x;
        float y = rec.y;
        float dx = -origin.x;
        float dy = -origin.y;

        topLeft.x = x + dx*cosRotation - dy*sinRotation;
        topLeft.y = y + dx*sinRotation + dy*cosRotation;

        topRight.x = x + (dx + rec.width)*cosRotation - dy*sinRotation;
        topRight.y = y + (dx + rec.width)*sinRotation + dy*cosRotation;

        bottomLeft.x = x + dx*cosRotation - (dy + rec.height)*sinRotation;
        bottomLeft.y = y + dx*sinRotation + (dy + rec.height)*cosRotation;

        bottomRight.x = x + (dx + rec.width)*cosRotation - (dy + rec.height)*sinRotation;
        bottomRight.y = y + (dx + rec.width)*sinRotation + (dy + rec.height)*cosRotation;
    }

    return (RectanglePoints) {
        .TopLeft = topLeft,
        .TopRight = topRight,
        .BottomLeft = bottomLeft,
        .BottomRight = bottomRight,

        .Center = Vector2Lerp(topLeft, bottomRight, 0.5),
    };
}

bool CheckCollisionPointRecPro(Vector2 point, Rectangle rec, Vector2 origin, float rotation) {
    RectanglePoints points = GetRectanglePointsPro(rec, origin, rotation);
    return CheckCollisionPointTriangle(point, points.TopLeft, points.BottomLeft, points.TopRight)
        || CheckCollisionPointTriangle(point, points.TopRight, points.BottomLeft, points.BottomRight);
}

Vector2 V3V2(Vector3 v) {
    return (Vector2){ v.x, v.y };
}

Vector3 V2V3(Vector2 v, float z) {
    return (Vector3){ v.x, v.y, z };
}
