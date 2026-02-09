#include "main.h"

#include <cstring>
#include <thread>

#include "raylib.h"
#include "network/client.h"
#include "network/packets.h"
#include "util/dev/console/console.h"
#include "manager/ClientManager.h"
#include "manager/ConsoleManager.h"
#include "manager/ServerManager.h"
#include "input/input.h"

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

    //INIT INPUT
    InputManager inputManager{};
    setup(&inputManager);

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {

        if (ClientManager::has()) {
            ClientManager::get().update();
        }
        inputManager.process();

        if (inputManager.isPressed("dev_console")) {
            if (ConsoleManager::has()) {
                ConsoleManager::get().setOpen(!ConsoleManager::get().isOpen());
            }
        }
        if (ConsoleManager::has() && ConsoleManager::get().isOpen()) ConsoleManager::get().handleInput();

        if(inputManager.isHeld("walk")){
            ConsoleManager::get().log(INFO, "walk is held");
        }
        draw();
        //----------------------------------------------------------------------------------
    }

    shutdown();

    return 0;
}

void setup(InputManager* inputManager) {
    Net::init();
    ConsoleManager::create();

    inputManager->init(ASSETS_PATH "pixel_game/config/keybinds.json");
    inputManager->setContext("gameplay");
}

void draw() {
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
}

void shutdown() {
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
}