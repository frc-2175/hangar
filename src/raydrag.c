#include "raydrag.h"

#include "raylib.h"
#include "raymath.h"

#include <assert.h>
#include <stdio.h>

bool Dragging = false;
bool DragPending = false;
bool DragCanceled = false;
void *DragThing = 0;
Vector2 DragMouseStart = {0};
Vector2 DragObjStart = {0};

void UpdateDrag() {
    if (IsKeyPressed(KEY_ESCAPE)) {
        Dragging = false;
        DragCanceled = true;
    } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        Dragging = false;
    } else if (IsMouseButtonUp(MOUSE_LEFT_BUTTON)) {
        Dragging = false;
        DragPending = false;
        DragCanceled = true;
        DragThing = false;
        DragMouseStart = (Vector2){};
        DragObjStart = (Vector2){};
    } else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (!Dragging && !DragPending) {
            DragPending = true;
            DragMouseStart = GetMousePosition();
        }
    }
}

bool TryToStartDrag(void *thing, bool isColliding, Vector2 objStart) {
    assert(thing);

    if (Dragging) {
        // can't start a new drag while one is in progress
        return false;
    }

    if (!DragPending) {
        // can't start a new drag with this item unless we have a pending one
		return false;
    }

    if (Vector2Length(Vector2Subtract(GetMousePosition(), DragMouseStart)) < 3) {
        // haven't dragged far enough
		return false;
    }

    if (!isColliding) {
        // drag started in the wrong place
        return false;
    }

    Dragging = true;
    DragPending = false;
    DragCanceled = false;
    DragThing = thing;
    DragObjStart = objStart;

    return true;
}

Vector2 DragOffset() {
    if (!Dragging && DragThing == 0) {
        return (Vector2){};
    }
    return Vector2Subtract(GetMousePosition(), DragMouseStart);
}

Vector2 DragMouseStartPosition() {
    return DragMouseStart;
}

Vector2 DragObjectStartPosition() {
    return DragObjStart;
}

Vector2 DragObjectNewPosition() {
    return Vector2Add(DragObjStart, DragOffset());
}

// Pass in an key and it will tell you the relevant drag state for that thing.
// matchesKey will be true if that object is the one currently being dragged.
// done will be true if the drag is complete this frame.
// canceled will be true if the drag is done but was canceled.
int DragState(void *thing) {
    bool matchesThing = true;
    if (thing) {
        matchesThing = DragThing == thing;
    }

    int result = matchesThing ? DRAG_MATCHES_THING : 0;
    if (!Dragging && DragThing && matchesThing) {
        result |= DRAG_DONE | DRAG_CANCELED;
    } else {
        // nothing to do
    }

    return result;
}
