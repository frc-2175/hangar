#pragma once

#include "raylib.h"

#define DRAG_MATCHES_THING 1
#define DRAG_DONE          2
#define DRAG_CANCELED      4

void UpdateDrag();
bool TryToStartDrag(void *thing, bool isColliding, Vector2 objStart);
Vector2 DragOffset();
Vector2 DragMouseStartPosition();
Vector2 DragObjectStartPosition();
Vector2 DragObjectNewPosition();
int DragState(void *thing);
