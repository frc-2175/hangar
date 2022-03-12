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
Part *editablePart = NULL;
Box *selectedBox = NULL; // TODO: Multiple selected boxes

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    parts[0] = (Part) {
        .Name = "Chassis",
        .Boxes = {
            {
                .Position = (Vector3){ 300, 400, 0 },
                .Width = 36, .Height = 2, .Depth = 1,
            },
            {
                .Position = (Vector3){ 300, 200, 0 },
                .Width = 18, .Height = 2, .Depth = 1,
                .Angle = 45,
            },
        },
        .NumBoxes = 2,
    };
    parts[1] = (Part) {
        .Name = "Mid Arm",
        .Boxes = {
            {
                .Position = (Vector3){ 600, 500, 0 },
                .Width = 36, .Height = 2, .Depth = 1,
            },
            {
                .Position = (Vector3){ 600, 300, 0 },
                .Width = 18, .Height = 2, .Depth = 1,
                .Angle = -20,
            },
        },
        .NumBoxes = 2,
        .Depth = 1,
    };
    numParts = 2;

    // Initialization
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "Hangar");
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);

    GuiSetStyle(DEFAULT, TEXT_SIZE, 20); // TODO: y u no work

    return 0;
}

RectanglePoints GetBoxPoints(Box box) {
    Vector2 pos = V3V2(Part2World(*box.Part, box.Position));
    Rectangle rec = (Rectangle){
        pos.x, pos.y,
        box.Width * IN2PX, box.Height * IN2PX,
    };
    Vector2 origin = (Vector2){ (box.Width*IN2PX)/2, (box.Height*IN2PX)/2 };
    float angle = box.Angle;

    return GetRectanglePointsPro(rec, origin, angle);
}

void DrawBox(Box box, Color color) {
    Vector2 pos = V3V2(Part2World(*box.Part, box.Position));
    Rectangle rec = (Rectangle){
        pos.x, pos.y,
        box.Width * IN2PX, box.Height * IN2PX,
    };
    Vector2 origin = (Vector2){ (box.Width*IN2PX)/2, (box.Height*IN2PX)/2 };
    float angle = box.Part->Angle + box.Angle;

    DrawRectanglePro(rec, origin, angle, color);
}

bool CheckCollisionBox(Vector2 point, Box box) {
    point = V3V2(World2Part(*box.Part, V2V3(point, 0)));
    Rectangle rec = (Rectangle){
        box.Position.x, box.Position.y,
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

Vector2 CenterOfRotationPos(Part *part) {
    return V3V2(Vector3Add(part->Position, part->CenterOfRotation));
}

void ClearSelected() {
    selectedPart = NULL;
    editablePart = NULL;
    selectedBox = NULL;
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
        UpdateReferences(part);

        // bool thisPartSelected = selectedPart == part;
        bool thisPartEditable = editablePart == part;

        if (thisPartEditable) {
            // Edit center of rotation
            TryToStartDrag(
                &part->DraggingCenterOfRotation,
                CheckCollisionPointCircle(DragMouseStartPosition(), CenterOfRotationPos(part), 30),
                CenterOfRotationPos(part)
            );
            if (DragState(&part->DraggingCenterOfRotation)) {
                Vector2 partPos = (Vector2){ part->Position.x, part->Position.y };
                Vector2 cor2 = Vector2Subtract(DragObjectNewPosition(), partPos);
                part->CenterOfRotation = (Vector3){ cor2.x, cor2.y, part->CenterOfRotation.z };
            }

            // Edit boxes
            for (int i = 0; i < part->NumBoxes; i++) {
                Box *box = &part->Boxes[i];

                // Handle clicks / drag starts
                bool overBox = CheckCollisionBox(GetMousePosition(), *box);
                bool translationDragStarted = TryToStartDrag(
                    box,
                    CheckCollisionBox(DragMouseStartPosition(), *box),
                    (Vector2){ box->Position.x, box->Position.y }
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
                        box->Position = (Vector3) { newPos.x, newPos.y, box->Position.z };
                    } break;
                    case Rotating: {
                        Vector2 mouseOffset = Vector2Subtract(GetMousePosition(), GetBoxPoints(*box).Center);
                        float newAngle = atan2f(mouseOffset.y, mouseOffset.x) * RAD2DEG;
                        box->Angle = newAngle;
                    } break;
                    }
                }
            }
        } else {
            // All clicks and drags move / rotate
            if (!editablePart && selectedPart == part) {
                TryToStartDrag(&part->DraggingPosition, true, V3V2(part->Position));
                if (DragState(&part->DraggingPosition)) {
                    Vector3 startPosThisFrame = part->Position;
                    part->Position = V2V3(DragObjectNewPosition(), part->Position.z);

                    for (int p2 = p + 1; p2 < numParts; p2++) {
                        Part *part2 = &parts[p2];

                        if (part2->Depth <= part->Depth) {
                            break;
                        }

                        Vector3 offset = Vector3Subtract(part2->Position, startPosThisFrame);
                        part2->Position = Vector3Add(part->Position, offset);
                    }
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
                if (editablePart && editablePart != part) {
                    color = GRAY;
                }
                if (IsBoxSelected(box)) {
                    color = BLUE;
                }

                RectanglePoints points = GetBoxPoints(*box);
                DrawBox(*box, color);
                DrawMeasurementText(TextFormat("%.1f", BoxMass(*box)), points.Center, box->Angle, WHITE);

                if (IsBoxSelected(box)) {
                    
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

            DrawCircleV(CenterOfRotationPos(part), 10, RED);
            DrawCircle(part->CenterOfMass.x, part->CenterOfMass.y, 10, BLUE);
        }

        // Draw part lister
        {
            int x;
            int y = 20;
            const int spacing = 24;
            const int depthPx = 28;

            for (int p = 0; p < numParts; p++) {
                Part *part = &parts[p];

                x = 20;
                x += depthPx * part->Depth;

                // move down
                if (GuiButton((Rectangle){ x, y, 19, 19 }, "")) {
                    if (p < numParts-1) {
                        Part tmp = parts[p + 1];
                        parts[p + 1] = *part;
                        parts[p] = tmp;
                        ClearSelected();
                    }
                }
                GuiDrawIcon(116 /* down arrow */, x + 2, y + 2, 1, BLACK);
                x += 21;

                // move up
                if (GuiButton((Rectangle){ x, y, 19, 19 }, "")) {
                    if (p > 0) {
                        Part tmp = parts[p - 1];
                        parts[p - 1] = *part;
                        parts[p] = tmp;
                        ClearSelected();
                    }
                }
                GuiDrawIcon(117 /* up arrow */, x + 2, y + 2, 1, BLACK);
                x += 21;

                // make shallower
                if (GuiButton((Rectangle){ x, y, 19, 19 }, "")) {
                    part->Depth = MAX(part->Depth - 1, 0);
                }
                GuiDrawIcon(114 /* left arrow */, x + 2, y + 2, 1, BLACK);
                x += 21;

                // make deeper
                if (GuiButton((Rectangle){ x, y, 19, 19 }, "")) {
                    part->Depth = MAX(part->Depth + 1, 0);
                }
                GuiDrawIcon(115 /* right arrow */, x + 2, y + 2, 1, BLACK);
                x += 21;

                // spacer to edit button
                x += 4;

                int editableBefore = editablePart == part;
                int editableAfter = GuiToggle((Rectangle){ x, y, 42, 19 }, "Editing", editablePart == part);
                if (!editableBefore && editableAfter) {
                    // just marked this one for editing
                    ClearSelected();
                    editablePart = part;
                }
                if (editableBefore && !editableAfter) {
                    // stopped editing
                    ClearSelected();
                }
                x += 44;

                x += 4;
                DrawText(part->Name, x, y, 20, selectedPart == part ? BLUE : BLACK);
                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)
                        && CheckCollisionPointRec(
                            GetMousePosition(),
                            (Rectangle){ x, y, MeasureText(part->Name, 20), 20 }
                        )
                ) {
                    selectedPart = part;
                }
                y += spacing;
            }
        }
    }
    EndDrawing();
    //----------------------------------------------------------------------------------
}
