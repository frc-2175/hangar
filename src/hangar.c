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
#include "serialization.h"

static const int screenWidth = 1300;
static const int screenHeight = 810;

static void UpdateDrawFrame(void);

#define IN2PX 8
#define MAX_PARTS 64
Part parts[MAX_PARTS];
int numParts;
Part *editablePart = NULL;
Part *attachmentPart = NULL;
Box *selectedBox = NULL; // TODO: Multiple selected boxes

int floorY = 800;
int midRungX = 1000;

float rungSpacing = 2*12;
float midRungHeightIn = (5*12 + 0.25) - (1.66/2);
float highRungHeightIn = (6*12 + 3.625) - (1.66/2);
float traversalRungHeightIn = (7*12 + 7) - (1.66/2);

Vector2 midRungPos() {
    return (Vector2) {
        midRungX,
        floorY - midRungHeightIn*IN2PX,
    };
}

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
                .Position = { 100, 0 },
                .Width = 36, .Height = 2, .Depth = 1,
                .WallThickness = 0.125,
                .Quantity = 2,
            },
            {
                .Position = { 0, -200 },
                .Angle = 90,
                .Width = 48, .Height = 2, .Depth = 2,
                .WallThickness = 0.125,
                .Quantity = 2,
            },
        },
        .NumBoxes = 2,
        .Position = { 200, 700 },
    };
    parts[1] = (Part) {
        .Name = "Mid Arm",
        .Boxes = {
            {
                .Position = { 120, 0 },
                .Width = 36, .Height = 1, .Depth = 1,
                .WallThickness = 0.125,
                .Quantity = 2,
            },
            {
                .Position = { 260, 20 },
                .Width = 4, .Height = 1, .Depth = 0.25,
                .Angle = 90,
                .Quantity = 4,
                .Material = Polycarb,
            },
        },
        .NumBoxes = 2,
        .Depth = 1,
        .Position = { 200, 450 },
        .AttachmentPoint = { 250, 10 },
    };
    numParts = 2;
    attachmentPart = &parts[1];

    if (HasQuery()) {
        LoadQuery(parts, &numParts);
        ClearQuery();
        ClearWIP();
    } else if (HasWIP()) {
        LoadWIP(parts, &numParts);
    }

    // Initialization
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "Hangar");
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);

    GuiSetStyle(DEFAULT, TEXT_SIZE, 20); // TODO: y u no work

    return 0;
}

float GetBoxAngle(Box box) {
    return PartAngle2WorldAngle(*box.Part, box.Angle);
}

RectanglePoints GetBoxPoints(Box box) {
    Vector2 pos = Part2World(*box.Part, box.Position);
    Rectangle rec = (Rectangle){
        pos.x, pos.y,
        box.Width * IN2PX, box.Height * IN2PX,
    };
    Vector2 origin = (Vector2){ (box.Width*IN2PX)/2, (box.Height*IN2PX)/2 };
    float angle = GetBoxAngle(box);

    return GetRectanglePointsPro(rec, origin, angle);
}

void DrawBox(Box box, Color color) {
    Vector2 pos = Part2World(*box.Part, box.Position);
    Rectangle rec = (Rectangle){
        pos.x, pos.y,
        box.Width * IN2PX, box.Height * IN2PX,
    };
    Vector2 origin = (Vector2){ (box.Width*IN2PX)/2, (box.Height*IN2PX)/2 };
    float angle = GetBoxAngle(box);

    DrawRectanglePro(rec, origin, angle, color);
}

void DrawBoxAttachment(Box box, Vector2 attachPos, Vector2 COM, Color color) {
    Vector2 pos = Vector2Add(
        World2Attachment(
            attachPos, COM,
            Part2World(*box.Part, box.Position)
        ),
        midRungPos()
    );
    Rectangle rec = (Rectangle){
        pos.x, pos.y,
        box.Width * IN2PX, box.Height * IN2PX,
    };
    Vector2 origin = (Vector2){ (box.Width*IN2PX)/2, (box.Height*IN2PX)/2 };
    float angle = WorldAngle2AttachmentAngle(
        attachPos, COM,
        box.Part->Angle + box.Angle
    );

    DrawRectanglePro(rec, origin, angle, color);
}

bool CheckCollisionBox(Vector2 point, Box box) {
    point = World2Part(*box.Part, point); // oh it's checking collision in a different space?? wack
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
    float angle = GetBoxAngle(box);
    DrawRing(center, radius, radius + 3, -angle + 90 - 20, -angle + 90 + 20, 16, DARKGRAY);
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

Vector2 AttachmentPointPos(Part part) {
    return Part2World(part, part.AttachmentPoint);
}

Vector2 PartRotHandlePos(Part part) {
    Vector2 pos = part.Position;
    float radius = 100;
    return (Vector2) {
        .x = pos.x + radius * cosf(part.Angle * DEG2RAD),
        .y = pos.y + radius * sinf(part.Angle * DEG2RAD),
    };
}

bool CheckCollisionPartRotHandle(Vector2 point, Part part) {
    return CheckCollisionPointCircle(point, PartRotHandlePos(part), 20);
}

void DrawPartRotHandle(Part part) {
    int radius = 50;
    Vector2 partRight = (Vector2){ cosf(part.Angle * DEG2RAD), sinf(part.Angle * DEG2RAD) };
    Vector2 center = Vector2Add(PartRotHandlePos(part), Vector2Scale(partRight, -50));
    DrawRing(center, radius, radius + 3, -part.Angle + 90 - 20, -part.Angle + 90 + 20, 16, DARKGRAY);
}

void ClearSelected() {
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

        bool thisPartEditable = editablePart == part;

        if (thisPartEditable) {
            // Edit center of rotation
            TryToStartDrag(
                &part->DraggingCenterOfRotation,
                CheckCollisionPointCircle(DragMouseStartPosition(), part->Position, 30),
                part->Position
            );
            if (DragState(&part->DraggingCenterOfRotation)) {
                Vector2 delta = Vector2Subtract(DragObjectNewPosition(), part->Position);
                part->Position = Vector2Add(part->Position, delta);
                for (int b = 0; b < part->NumBoxes; b++) {
                    Box *box = &part->Boxes[b];
                    Vector2 posWorld = Part2World(*part, box->Position);
                    Vector2 newPosWorld = Vector2Subtract(posWorld, delta);
                    box->Position = World2Part(*part, newPosWorld);
                }
                {
                    Vector2 posWorld = Part2World(*part, part->AttachmentPoint);
                    Vector2 newPosWorld = Vector2Subtract(posWorld, delta);
                    part->AttachmentPoint = World2Part(*part, newPosWorld);
                }
            }

            // Edit attachment point
            TryToStartDrag(
                &part->DraggingAttachmentPoint,
                CheckCollisionPointCircle(DragMouseStartPosition(), AttachmentPointPos(*part), 30),
                AttachmentPointPos(*part)
            );
            if (DragState(&part->DraggingAttachmentPoint)) {
                part->AttachmentPoint = World2Part(*part, DragObjectNewPosition());
            }

            // Edit boxes
            for (int b = 0; b < part->NumBoxes; b++) {
                Box *box = &part->Boxes[b];

                // Handle clicks / drag starts
                bool overBox = CheckCollisionBox(GetMousePosition(), *box);
                bool positionDragStarted = TryToStartDrag(
                    &box->DraggingPosition,
                    CheckCollisionBox(DragMouseStartPosition(), *box),
                    Part2World(*part, box->Position)
                );
                bool rotationDragStarted = TryToStartDrag(
                    &box->DraggingRotation,
                    CheckCollisionBoxRotHandle(DragMouseStartPosition(), *box),
                    (Vector2){}
                );
                if (positionDragStarted || rotationDragStarted) {
                    selectedBox = box;
                } else if (overBox
                        && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)
                        && !IsBoxSelected(box)
                ) {
                    selectedBox = box;
                }

                // Handle dragging
                if (DragState(&box->DraggingPosition)) {
                    box->Position = World2Part(*part, DragObjectNewPosition());
                } else if (DragState(&box->DraggingRotation)) {
                    Vector2 mouseOffset = Vector2Subtract(GetMousePosition(), GetBoxPoints(*box).Center);
                    float newAngle = atan2f(mouseOffset.y, mouseOffset.x) * RAD2DEG;
                    box->Angle = WorldAngle2PartAngle(*part, newAngle);
                }
            }
        } else {
            // Part movement / rotation etc.
            if (!editablePart) {
                TryToStartDrag(
                    &part->DraggingRotation,
                    CheckCollisionPointCircle(DragMouseStartPosition(), PartRotHandlePos(*part), 20),
                    (Vector2){}
                );
                if (DragState(&part->DraggingRotation)) {
                    Vector2 mouseOffset = Vector2Subtract(GetMousePosition(), part->Position);
                    float newAngle = atan2f(mouseOffset.y, mouseOffset.x) * RAD2DEG;
                    float angleDelta = newAngle - part->Angle;

                    // rotate this part (ez)
                    part->Angle = newAngle;

                    // rotate child parts (less ez)
                    for (int p2 = p + 1; p2 < numParts; p2++) {
                        Part *part2 = &parts[p2];

                        if (part2->Depth <= part->Depth) {
                            break;
                        }

                        part2->Position = Vector2Add(Vector2Rotate(Vector2Subtract(part2->Position, part->Position), angleDelta * DEG2RAD), part->Position);
                        part2->Angle += angleDelta;
                    }
                }

                bool dragOverAnyBox = false;
                for (int b = 0; b < part->NumBoxes; b++) {
                    Box box = part->Boxes[b];
                    if (CheckCollisionBox(DragMouseStartPosition(), box)) {
                        dragOverAnyBox = true;
                    }
                }
                TryToStartDrag(&part->DraggingPosition, dragOverAnyBox, part->Position);
                if (DragState(&part->DraggingPosition)) {
                    Vector2 startPosThisFrame = part->Position;
                    part->Position = DragObjectNewPosition();

                    for (int p2 = p + 1; p2 < numParts; p2++) {
                        Part *part2 = &parts[p2];

                        if (part2->Depth <= part->Depth) {
                            break;
                        }

                        Vector2 offset = Vector2Subtract(part2->Position, startPosThisFrame);
                        part2->Position = Vector2Add(part->Position, offset);
                    }
                }

                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)
                        && !(DragState(NULL) & (DRAG_DONE|DRAG_CANCELED))
                        && CheckCollisionPointCircle(GetMousePosition(), Part2World(*part, part->AttachmentPoint), 10)
                ) {
                    attachmentPart = part;
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

            for (int b = 0; b < part->NumBoxes; b++) {
                Box *box = &part->Boxes[b];

                Color color = BLACK;
                if (editablePart && editablePart != part) {
                    color = GRAY;
                }
                if (IsBoxSelected(box)) {
                    color = BLUE;
                }

                RectanglePoints points = GetBoxPoints(*box);
                DrawBox(*box, color);
                DrawMeasurementText(TextFormat("%.1f", BoxMass(*box)), points.Center, GetBoxAngle(*box), WHITE);

                if (IsBoxSelected(box)) {
                    Vector2 recDown = Vector2Normalize(Vector2Subtract(points.BottomLeft, points.TopLeft));
                    Vector2 recRight = Vector2Normalize(Vector2Subtract(points.TopRight, points.TopLeft));
                    Vector2 widthTextPos = Vector2Add(points.BottomMiddle, Vector2Scale(recDown, 20));
                    Vector2 heightTextPos = Vector2Add(points.RightMiddle, Vector2Scale(recRight, 20));
                    DrawMeasurementText(TextFormat("%.1f\"", box->Width), widthTextPos, GetBoxAngle(*box), BLACK);
                    DrawMeasurementText(TextFormat("%.1f\"", box->Height), heightTextPos, GetBoxAngle(*box), BLACK);

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

                    Vector2 deleteButtonPos = Vector2Add(
                        Vector2Add(points.LeftMiddle, Vector2Scale(recRight, -20)),
                        (Vector2){ -10, -10 }
                    );
                    if (GuiButton((Rectangle){ deleteButtonPos.x, deleteButtonPos.y, 20, 20 }, "")) {
                        selectedBox = NULL;
                        for (int b2 = b + 1; b2 < part->NumBoxes; b2++) {
                            part->Boxes[b2-1] = part->Boxes[b2];
                        }
                        part->NumBoxes--;
                    }
                    GuiDrawIcon(128 /* cross */, deleteButtonPos.x + 2, deleteButtonPos.y + 2, 1, BLACK);

                    // Draw detail UI
                    {
                        /*
                         * We draw this in reverse order, because raygui's
                         * dropdowns are _really good_
                         */

                        int x = 20;
                        int y = 20 + 24*(numParts + 1) + 20 + 24*5;
                        int fieldX = 260;

                        // Mass override
                        DrawText("Hardcoded Mass (lbs)", x, y, 20, BLACK);
                        GuiNumberTextBoxEx(&box->HardcodedMassTextBox, (Rectangle){ fieldX, y, 100, 20 }, &box->HardcodedMass);
                        y -= 24;

                        // Material
                        DrawText("Material", x, y, 20, BLACK);
                        switch (GuiDropdownBoxEx(&box->MaterialDropdown, (Rectangle){ fieldX, y, 100, 20 }, "Aluminum;Polycarb;Steel")) {
                        case 0: {
                            box->Material = Aluminum;
                        } break;
                        case 1: {
                            box->Material = Polycarb;
                        } break;
                        case 2: {
                            box->Material = Steel;
                        } break;
                        }
                        y -= 24;

                        // Quantity
                        DrawText("Quantity", x, y, 20, BLACK);
                        GuiIntTextBoxEx(&box->QuantityTextBox, (Rectangle){ fieldX, y, 100, 20 }, &box->Quantity);
                        y -= 24;

                        // Wall thickness
                        DrawText("Wall Thickness (in)", x, y, 20, BLACK);
                        GuiNumberTextBoxEx(&box->WallThicknessTextBox, (Rectangle){ fieldX, y, 100, 20 }, &box->WallThickness);
                        y -= 24;

                        // Width, height, depth
                        DrawText("Dimensions (in)", x, y, 20, BLACK);
                        int dimensionFieldWidth = 60;
                        GuiNumberTextBoxEx(&box->WidthTextBox, (Rectangle){ fieldX, y, 60, 20 }, &box->Width);
                        GuiNumberTextBoxEx(&box->HeightTextBox, (Rectangle){ fieldX + dimensionFieldWidth + 2, y, 60, 20 }, &box->Height);
                        GuiNumberTextBoxEx(&box->DepthTextBox, (Rectangle){ fieldX + 2*(dimensionFieldWidth + 2), y, 60, 20 }, &box->Depth);
                        y -= 24;
                    }
                }
            }

            if (!editablePart || editablePart == part) {
                DrawCircleV(part->Position, 4, RED);
                DrawRing(AttachmentPointPos(*part), 1.66/2*IN2PX - 3, 1.66/2*IN2PX, 0, 360, 36, GREEN);
            }

            if (editablePart == part) {
                DrawCircle(part->CenterOfMass.x, part->CenterOfMass.y, 10, PURPLE);
            }

            if (!editablePart) {
                DrawPartRotHandle(*part);
            }

            if (editablePart == part) {
                if (GuiButton((Rectangle){ GetScreenWidth() - 120, 20, 100, 20 }, "Add Box")) {
                    part->Boxes[part->NumBoxes] = (Box){
                        .Width = 24,
                        .Height = 2,
                        .Depth = 1,
                        .WallThickness = 0.25,
                        .Quantity = 2,
                        .Material = Aluminum,
                        .Position = { 20 * part->NumBoxes, 20 * part->NumBoxes },
                    };
                    Box *newBox = &part->Boxes[part->NumBoxes];
                    part->NumBoxes++;

                    selectedBox = newBox;
                }
            }
        }

        // Draw part lister
        {
            int x;
            int y = 20;
            const int spacing = 24;
            const int depthPx = 28;

            int partToDelete = -1;

            for (int p = 0; p < numParts; p++) {
                Part *part = &parts[p];

                x = 20;
                if (GuiButton((Rectangle){ x, y, 19, 19 }, "")) {
                    partToDelete = p;
                }
                GuiDrawIcon(128 /* cross */, x + 1, y + 2, 1, BLACK);

                x = 50;
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
                if (part->SelectedField == NameField) {
                    if (GuiTextBox((Rectangle){ x, y, 200, 19 }, part->Name, PART_NAME_LENGTH, true)) {
                        part->SelectedField = 0;
                    }
                } else {
                    DrawText(part->Name, x, y, 20, BLACK);
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)
                            && CheckCollisionPointRec(GetMousePosition(), (Rectangle){ x, y, 200, 19 })
                    ) {
                        part->SelectedField = NameField;
                    }
                }
                y += spacing;
            }

            // add part
            if (GuiButton((Rectangle){ 20, y, 100, 19 }, "Add Part")) {
                if (numParts < MAX_PARTS) {
                    parts[numParts] = (Part){
                        .Position = { 400, 400 },
                        .Name = "New Part",
                        .Boxes = {
                            {
                                .Width = 24, .Height = 2, .Depth = 1,
                                .WallThickness = 0.25,
                                .Quantity = 2,
                                .Material = Aluminum,
                            },
                        },
                        .NumBoxes = 2,
                    };
                    Part *newPart = &parts[numParts];
                    numParts++;

                    editablePart = newPart;
                }
            }

            if (partToDelete > -1) {
                ClearSelected();
                for (int p2 = partToDelete + 1; p2 < numParts; p2++) {
                    parts[p2-1] = parts[p2];
                }
                numParts--;
            }
        }

        Vector2 overallCOM = ComputeCOM(parts, numParts);
        if (!editablePart) {
            DrawCircleV(overallCOM, 10, PURPLE);
        }

        // Draw the field
        DrawLine(0, floorY, screenWidth, floorY, GRAY);
        DrawCircleV((Vector2){
            midRungX,
            floorY - midRungHeightIn*IN2PX,
        }, 1.66/2*IN2PX, GRAY);
        DrawCircleV((Vector2){
            midRungX - rungSpacing*IN2PX,
            floorY - highRungHeightIn*IN2PX,
        }, 1.66/2*IN2PX, GRAY);
        DrawCircleV((Vector2){
            midRungX - 2*rungSpacing*IN2PX,
            floorY - traversalRungHeightIn*IN2PX,
        }, 1.66/2*IN2PX, GRAY);

        // Draw the attached stuff!!
        if (attachmentPart) {
            Vector2 attachPos = Part2World(*attachmentPart, attachmentPart->AttachmentPoint);
            for (int p = 0; p < numParts; p++) {
                Part *part = &parts[p];

                for (int b = 0; b < part->NumBoxes; b++) {
                    Box *box = &part->Boxes[b];

                    DrawBoxAttachment(*box, attachPos, overallCOM, editablePart ? GRAY : BLACK);
                }
            }

            Vector2 COMAfterAttachment = Vector2Add(midRungPos(), World2Attachment(attachPos, overallCOM, overallCOM));

            DrawCircleV(COMAfterAttachment, 10, PURPLE);
        }

        // Draw the total mass
        DrawText(
            TextFormat("Total weight: %.2f lbs", TotalMass(parts, numParts)),
            20, screenHeight - 20 - 20,
            20, BLACK
        );

        if (GuiButton((Rectangle){ screenWidth - 20 - 100, screenHeight - 20 - 20, 100, 20 }, "Share")) {
            SaveJSONQuery(Parts2JSON(parts, numParts));
        }
    }
    EndDrawing();
    //----------------------------------------------------------------------------------

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        ClearQuery();
    }
    SaveJSONWIP(Parts2JSON(parts, numParts));
}
