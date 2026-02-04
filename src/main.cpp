#include <cstring>
#include <thread>

#include "raylib.h"
#include "network/client.h"
#include "network/packets.h"
#include "network/server.h"
#include "../include/util/dev/console/console.h"

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

    Net::Init();

    Console::Init();

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {

        if (Client::Has()) {
            Client::Update();
        }

        if (IsKeyPressed(KEY_GRAVE)) {
            Console::SetOpen(!Console::IsOpen());
        }

        if (Console::IsOpen()) Console::HandleInput();

        BeginDrawing();

        ClearBackground(GRAY);

        if (Client::Has()) {
            if(Client::Get()->state == Net_State::IDLE) DrawText("Type ip of server to conenct", 10, 50, 20, GREEN);
            else if(Client::Get()->state == Net_State::CONNECTING) DrawText("Connecting to server...", 10, 50, 20, GREEN);
            else if(Client::Get()->state == Net_State::READY) DrawText("Ready to play", 10, 50, 20, GREEN);
        }

        if (Console::IsOpen()) {
            Console::Draw();
        }
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    if (Server::Has()) {
        Server::Stop();
        while (Server::Has()) {}
    }
    if (Client::Has()) {
        Client::Destroy();
    }

    Net::Shutdown();
    Console::Destroy();

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}