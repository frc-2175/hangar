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

#include "cad.h"

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
            .X = 300, .Y = 400,
            .Width = 36, .Height = 2, .Depth = 1,
        },
        {
            .X = 300, .Y = 200,
            .Width = 18, .Height = 2, .Depth = 1,
            .Angle = 45,
        },
    },
    .NumBoxes = 2,
};

#define IN2PX 16

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
// Update and draw game frame
static void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    UpdatePartCOM(&testPart);
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
    {
        ClearBackground(RAYWHITE);

        for (int i = 0; i < testPart.NumBoxes; i++) {
            Box box = testPart.Boxes[i];
            DrawRectanglePro(
                (Rectangle){
                    box.X, box.Y,
                    box.Width * IN2PX, box.Height * IN2PX,
                },
                (Vector2){ (box.Width*IN2PX)/2, (box.Height*IN2PX)/2 },
                box.Angle,
                BLACK
            );
            DrawText(TextFormat("%.1f", BoxMass(box)), box.X, box.Y, 20, WHITE);
        }

        DrawCircle(testPart.CenterOfMass.x, testPart.CenterOfMass.y, 10, BLUE);

        DrawFPS(10, 10);
    }
    EndDrawing();
    //----------------------------------------------------------------------------------
}
