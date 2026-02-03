#include <cstring>
#include <thread>

#include "raylib.h"
#include "network/client.h"
#include "network/packets.h"
#include "network/server.h"
#include "util/dev.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1080;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "raylib example - multiplayer");

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    Net_Init();

    Console* console = new Console();
    Console_Init(console);

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {

        if (Client_Has()) {
            Client_Update(Client_Get());
        }

        if (IsKeyPressed(KEY_GRAVE)) {
            Console_SetOpen(Console_Get(), !Console_Get()->open);
        }

        if (Console_IsOpen(Console_Get())) {
            Console_HandleInput(console);

            if (GetMouseWheelMove() > 0) {
                Console_Get()->scroll_offset++;
            } else if (GetMouseWheelMove() < 0) {
                Console_Get()->scroll_offset--;
            }
        }
        BeginDrawing();

        ClearBackground(GRAY);

        if (Client_Has()) {
            if(Client_Get()->state == IDLE) DrawText("Type ip of server to conenct", 10, 50, 20, GREEN);
            else if(Client_Get()->state == CONNECTING) DrawText("Connecting to server...", 10, 50, 20, GREEN);
            else if(Client_Get()->state == READY) DrawText("Ready to play", 10, 50, 20, GREEN);
        }

        if (Console_IsOpen(Console_Get())) {
            Console_Draw(Console_Get());
        }
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    Console_Destroy(console);

    if (Server_Has()) {
        Server_Destroy(Server_Get());
    }
    if (Client_Has()) {
        Client_Destroy(Client_Get());
    }

    Net_Shutdown();
    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}