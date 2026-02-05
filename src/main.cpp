#include <cstring>
#include <thread>

#include "raylib.h"
#include "network/client.h"
#include "network/packets.h"
#include "network/server.h"
#include "../include/util/dev/console/console.h"
#include "manager/ClientManager.h"
#include "manager/ConsoleManager.h"
#include "manager/ServerManager.h"

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

    Net::init();

    ConsoleManager::create();

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {

        if (ClientManager::has()) {
            ClientManager::get().update();
        }

        if (IsKeyPressed(KEY_GRAVE)) {
            if (ConsoleManager::has()) {
                ConsoleManager::get().setOpen(!ConsoleManager::get().isOpen());
            }
        }

        if (ConsoleManager::has() && ConsoleManager::get().isOpen()) ConsoleManager::get().handleInput();

        BeginDrawing();

        ClearBackground(GRAY);

        if (ClientManager::has()) {
            if(ClientManager::get().getState() == NetState::IDLE) DrawText("Type ip of server to conenct", 10, 50, 20, GREEN);
            else if(ClientManager::get().getState() == NetState::CONNECTING) DrawText("Connecting to server...", 10, 50, 20, GREEN);
            else if(ClientManager::get().getState() == NetState::READY) DrawText("Ready to play", 10, 50, 20, GREEN);
        }

        if (ConsoleManager::has() && ConsoleManager::get().isOpen()) {
            ConsoleManager::get().draw();
        }
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    if (ServerManager::has()) {
        ServerManager::stop();
    }
    if (ClientManager::has()) {
        ClientManager::leave();
    }

    Net::shutdown();
    ConsoleManager::destroy();

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}