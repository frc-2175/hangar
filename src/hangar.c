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

#include "cad.h"
#include "rayext.h"
#include "raydrag.h"

//----------------------------------------------------------------------------------
// Shared Variables Definition (global)
// NOTE: Those variables are shared between modules through screens.h
//----------------------------------------------------------------------------------
Font font = { 0 };

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

    InitAudioDevice();      // Initialize audio device

    // Load global data (assets that must be available in all screens, i.e. font)
    font = LoadFont("resources/mecha.png");

    emscripten_set_main_loop(UpdateDrawFrame, 10, 1);

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

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
// Update and draw game frame
static void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    UpdateDrag();

    for (int i = 0; i < testPart.NumBoxes; i++) {
        Box *box = &testPart.Boxes[i];

        TryToStartDrag(
            box,
            CheckCollisionBox(DragMouseStartPosition(), *box),
            (Vector2){ box->Translation.x, box->Translation.y }
        );

        int dragState = DragState(box);
        if (dragState) {
            Vector2 newPos = DragObjectNewPosition();
            box->Translation = (Vector3) { newPos.x, newPos.y, box->Translation.z };
        }
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

            DrawBox(*box, CheckCollisionBox(GetMousePosition(), *box) ? RED : BLACK);
            DrawText(TextFormat("%.1f", BoxMass(*box)), box->Translation.x, box->Translation.y, 20, WHITE);
        }

        DrawCircle(testPart.CenterOfMass.x, testPart.CenterOfMass.y, 10, BLUE);

        DrawFPS(10, 10);
    }
    EndDrawing();
    //----------------------------------------------------------------------------------
}
