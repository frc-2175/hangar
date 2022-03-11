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

static const int screenWidth = 1200;
static const int screenHeight = 900;

static void UpdateDrawFrame(void);          // Update and draw one frame

#define IN2PX 16
#define MAX_PARTS 64
Part parts[MAX_PARTS];
int numParts;
Part *selectedPart = NULL;
Box *selectedBox = NULL; // TODO: Multiple selected boxes

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    parts[0] = (Part) {
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
    parts[1] = (Part) {
        .Boxes = {
            {
                .Translation = (Vector3){ 600, 500, 0 },
                .Width = 36, .Height = 2, .Depth = 1,
            },
            {
                .Translation = (Vector3){ 600, 300, 0 },
                .Width = 18, .Height = 2, .Depth = 1,
                .Angle = -20,
            },
        },
        .NumBoxes = 2,
    };
    numParts = 2;

    // Initialization
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "Hangar");
    emscripten_set_main_loop(UpdateDrawFrame, 20, 1);

    GuiSetStyle(DEFAULT, TEXT_SIZE, 20); // TODO: y u no work

    return 0;
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

Vector2 BoxRotHandlePos(Box box) {
    RectanglePoints points = GetBoxPoints(box);
    Vector2 rightMiddle = Vector2Lerp(points.TopRight, points.BottomRight, 0.5);
    Vector2 recRight = Vector2Normalize(Vector2Subtract(points.TopRight, points.TopLeft));
    return Vector2Add(rightMiddle, Vector2Scale(recRight, 40));
}

bool CheckCollisionBoxRotHandle(Vector2 point, Box box) {
    return CheckCollisionPointCircle(point, BoxRotHandlePos(box), 20);
}

void DrawBoxRotHandle(Box box) {
    int radius = 35;
    RectanglePoints points = GetBoxPoints(box);
    Vector2 rightMiddle = Vector2Lerp(points.TopRight, points.BottomRight, 0.5);
    Vector2 recRight = Vector2Normalize(Vector2Subtract(points.TopRight, points.TopLeft));
    Vector2 center = Vector2Add(rightMiddle, Vector2Scale(recRight, 10));
    DrawRing(center, radius, radius + 3, -box.Angle + 90 - 20, -box.Angle + 90 + 20, 16, BLACK);
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

    for (int p = 0; p < numParts; p++) {
        Part *part = &parts[p];
        bool thisPartSelected = selectedPart == part;

        for (int i = 0; i < part->NumBoxes; i++) {
            Box *box = &part->Boxes[i];

            if (!thisPartSelected) {
                // no updates unless this part is selected for editing
                continue;
            }

            // Handle clicks / drag starts
            bool overBox = CheckCollisionBox(GetMousePosition(), *box);
            bool translationDragStarted = TryToStartDrag(
                box,
                CheckCollisionBox(DragMouseStartPosition(), *box),
                (Vector2){ box->Translation.x, box->Translation.y }
            );
            bool rotationDragStarted = TryToStartDrag(
                box,
                CheckCollisionBoxRotHandle(DragMouseStartPosition(), *box),
                (Vector2){}
            );
            if (translationDragStarted) {
                selectedBox = box;
                box->DragMode = Translating;
            } else if (rotationDragStarted) {
                selectedBox = box;
                box->DragMode = Rotating;
            } else if (overBox
                    && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)
                    && !IsBoxSelected(box)
            ) {
                selectedBox = box;
            }

            // Handle dragging
            int dragging = DragState(box);
            if (dragging) {
                switch (box->DragMode) {
                case Translating: {
                    Vector2 newPos = DragObjectNewPosition();
                    box->Translation = (Vector3) { newPos.x, newPos.y, box->Translation.z };
                } break;
                case Rotating: {
                    Vector2 mouseOffset = Vector2Subtract(GetMousePosition(), (Vector2){ box->Translation.x, box->Translation.y });
                    float newAngle = atan2f(mouseOffset.y, mouseOffset.x) * RAD2DEG;
                    box->Angle = newAngle;
                } break;
                }
            }
        }
        UpdatePartCOM(part);
    }
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
    {
        ClearBackground(RAYWHITE);

        for (int p = 0; p < numParts; p++) {
            Part *part = &parts[p];

            for (int i = 0; i < part->NumBoxes; i++) {
                Box *box = &part->Boxes[i];

                Color color = BLACK;
                if (IsBoxSelected(box)) {
                    color = BLUE;
                }

                DrawBox(*box, color);
                DrawMeasurementText(TextFormat("%.1f", BoxMass(*box)), (Vector2){ box->Translation.x, box->Translation.y }, box->Angle, WHITE);

                if (IsBoxSelected(box)) {
                    RectanglePoints points = GetBoxPoints(*box);
                    Vector2 bottomMiddle = Vector2Lerp(points.BottomLeft, points.BottomRight, 0.5);
                    Vector2 rightMiddle = Vector2Lerp(points.TopRight, points.BottomRight, 0.5);
                    Vector2 recDown = Vector2Normalize(Vector2Subtract(points.BottomLeft, points.TopLeft));
                    Vector2 recRight = Vector2Normalize(Vector2Subtract(points.TopRight, points.TopLeft));
                    Vector2 widthTextPos = Vector2Add(bottomMiddle, Vector2Scale(recDown, 20));
                    Vector2 heightTextPos = Vector2Add(rightMiddle, Vector2Scale(recRight, 20));
                    DrawMeasurementText(TextFormat("%.1f\"", box->Width), widthTextPos, box->Angle, BLACK);
                    DrawMeasurementText(TextFormat("%.1f\"", box->Height), heightTextPos, box->Angle, BLACK);

                    DrawBoxRotHandle(*box);

                    switch (box->SelectedField) {
                    case WidthField: {
                        if (MeasurementTextBox(widthTextPos, box)) {
                            UpdateMeasurement(&box->Width, box->TextInputBuf);
                            box->SelectedField = 0;
                        }
                    } break;
                    case HeightField: {
                        if (MeasurementTextBox(heightTextPos, box)) {
                            UpdateMeasurement(&box->Height, box->TextInputBuf);
                            box->SelectedField = 0;
                        }
                    } break;
                    }

                    int measurementClickRadius = 20;
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && CheckCollisionPointCircle(GetMousePosition(), widthTextPos, measurementClickRadius)) {
                        box->SelectedField = WidthField;
                        TextCopy(box->TextInputBuf, TextFormat("%.1f", box->Width));
                    }
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && CheckCollisionPointCircle(GetMousePosition(), heightTextPos, measurementClickRadius)) {
                        box->SelectedField = HeightField;
                        TextCopy(box->TextInputBuf, TextFormat("%.1f", box->Height));
                    }
                }
            }

            DrawCircle(part->CenterOfMass.x, part->CenterOfMass.y, 10, BLUE);
        }

        DrawFPS(10, 10);
    }
    EndDrawing();
    //----------------------------------------------------------------------------------
}
