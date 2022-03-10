/*******************************************************************************************
*
*   raylib game template
*
*   <Game title>
*   <Game description>
*
*   This game has been created using raylib (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2021 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include <emscripten/emscripten.h>
#include <stdio.h>
#include <stdlib.h>

#include "cad.h"
#include "rayext.h"
#include "raydrag.h"
#include "raygui.h"
#include "raymath.h"

//----------------------------------------------------------------------------------
// Local Variables Definition (local to this module)
//----------------------------------------------------------------------------------
static const int screenWidth = 1200;
static const int screenHeight = 900;

//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);          // Update and draw one frame

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "Hangar");
    emscripten_set_main_loop(UpdateDrawFrame, 10, 1);

    GuiSetStyle(DEFAULT, TEXT_SIZE, 20); // TODO: y u no work

    return 0;
}

Part testPart = {
    .Boxes = {
        {
            .Translation = (Vector3){ 300, 400, 0 },
            .Width = 36, .Height = 2, .Depth = 1,
        },
        {
            .Translation = (Vector3){ 300, 200, 0 },
            .Width = 18, .Height = 2, .Depth = 1,
            .Angle = 45,
        },
    },
    .NumBoxes = 2,
};

Box *selectedBox = NULL; // TODO: Multiple selected boxes

#define IN2PX 16

void DrawBox(Box box, Color color) {
    Rectangle rec = (Rectangle){
        box.Translation.x, box.Translation.y,
        box.Width * IN2PX, box.Height * IN2PX,
    };
    Vector2 origin = (Vector2){ (box.Width*IN2PX)/2, (box.Height*IN2PX)/2 };
    float angle = box.Angle;

    DrawRectanglePro(rec, origin, angle, color);
}

bool CheckCollisionBox(Vector2 point, Box box) {
    Rectangle rec = (Rectangle){
        box.Translation.x, box.Translation.y,
        box.Width * IN2PX, box.Height * IN2PX,
    };
    Vector2 origin = (Vector2){ (box.Width*IN2PX)/2, (box.Height*IN2PX)/2 };
    float angle = box.Angle;

    return CheckCollisionPointRecPro(point, rec, origin, angle);
}

RectanglePoints GetBoxPoints(Box box) {
    Rectangle rec = (Rectangle){
        box.Translation.x, box.Translation.y,
        box.Width * IN2PX, box.Height * IN2PX,
    };
    Vector2 origin = (Vector2){ (box.Width*IN2PX)/2, (box.Height*IN2PX)/2 };
    float angle = box.Angle;

    return GetRectanglePointsPro(rec, origin, angle);
}

bool IsBoxSelected(Box *box) {
    return selectedBox == box;
}

bool UpdateMeasurement(float *field, char *buf) {
    float res = strtof(buf, NULL);
    if (res == 0) {
        return false;
    }
    *field = res;
    buf[0] = 0;
    return true;
}

void DrawMeasurementText(const char* text, Vector2 pos, float angle, Color color) {
    int fontSize = 16;
    int spacing = 2;
    Vector2 size = MeasureTextEx(GetFontDefault(), text, fontSize, spacing);
    DrawTextPro(GetFontDefault(), text, pos, Vector2Scale(size, 0.5), angle, fontSize, spacing, color);
}

bool MeasurementTextBox(Vector2 pos, Box *box) {
    int width = 100;
    int height = 20;
    return GuiTextBox((Rectangle){ pos.x - width/2, pos.y - height/2, width, height }, box->TextInputBuf, BOX_TEXT_INPUT_MAX, true);
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
// Update and draw game frame
static void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    UpdateDrag();

    bool boxSelectedThisFrame = false;
    for (int i = 0; i < testPart.NumBoxes; i++) {
        Box *box = &testPart.Boxes[i];

        // Handle clicks / drag starts
        bool overBox = CheckCollisionBox(GetMousePosition(), *box);
        bool dragStarted = TryToStartDrag(
            box,
            CheckCollisionBox(DragMouseStartPosition(), *box),
            (Vector2){ box->Translation.x, box->Translation.y }
        );
        if (dragStarted) {
            selectedBox = box;
            boxSelectedThisFrame = true;
        } else if (overBox
                && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)
                && !IsBoxSelected(box)
        ) {
            selectedBox = box;
            boxSelectedThisFrame = true;
        }

        // Handle dragging
        int dragState = DragState(box);
        if (dragState) {
            Vector2 newPos = DragObjectNewPosition();
            box->Translation = (Vector3) { newPos.x, newPos.y, box->Translation.z };
        }
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)
            && !boxSelectedThisFrame
            && !(DragState(NULL) & (DRAG_DONE|DRAG_CANCELED))) {
        selectedBox = NULL;
    }

    UpdatePartCOM(&testPart);
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
    {
        ClearBackground(RAYWHITE);

        for (int i = 0; i < testPart.NumBoxes; i++) {
            Box *box = &testPart.Boxes[i];

            Color color = BLACK;
            if (IsBoxSelected(box)) {
                color = BLUE;
            }

            DrawBox(*box, color);
            DrawMeasurementText(TextFormat("%.1f", BoxMass(*box)), (Vector2){ box->Translation.x, box->Translation.y }, box->Angle, WHITE);

            RectanglePoints points = GetBoxPoints(*box);
            Vector2 bottomMiddle = Vector2Lerp(points.BottomLeft, points.BottomRight, 0.5);
            Vector2 rightMiddle = Vector2Lerp(points.TopRight, points.BottomRight, 0.5);
            Vector2 recDown = Vector2Normalize(Vector2Subtract(points.BottomLeft, points.TopLeft));
            Vector2 recRight = Vector2Normalize(Vector2Subtract(points.TopRight, points.TopLeft));
            Vector2 widthTextPos = Vector2Add(bottomMiddle, Vector2Scale(recDown, 20));
            Vector2 heightTextPos = Vector2Add(rightMiddle, Vector2Scale(recRight, 20));
            DrawMeasurementText(TextFormat("%.1f\"", box->Width), widthTextPos, box->Angle, BLACK);
            DrawMeasurementText(TextFormat("%.1f\"", box->Height), heightTextPos, box->Angle, BLACK);

            switch (box->selectedField) {
            case WidthField: {
                if (MeasurementTextBox(widthTextPos, box)) {
                    UpdateMeasurement(&box->Width, box->TextInputBuf);
                    box->selectedField = 0;
                }
            } break;
            case HeightField: {
                if (MeasurementTextBox(heightTextPos, box)) {
                    UpdateMeasurement(&box->Height, box->TextInputBuf);
                    box->selectedField = 0;
                }
            } break;
            }

            int measurementClickRadius = 20;
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && CheckCollisionPointCircle(GetMousePosition(), widthTextPos, measurementClickRadius)) {
                box->selectedField = WidthField;
                TextCopy(box->TextInputBuf, TextFormat("%.1f", box->Width));
            }
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && CheckCollisionPointCircle(GetMousePosition(), heightTextPos, measurementClickRadius)) {
                box->selectedField = HeightField;
                TextCopy(box->TextInputBuf, TextFormat("%.1f", box->Height));
            }
        }

        DrawCircle(testPart.CenterOfMass.x, testPart.CenterOfMass.y, 10, BLUE);

        DrawFPS(10, 10);
    }
    EndDrawing();
    //----------------------------------------------------------------------------------
}
