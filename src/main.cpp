#include "main.h"

#include <cstring>
#include <thread>

#include "raylib.h"
#include "screen_manager.h"
#include "network/client.h"
#include "network/packets.h"
#include "util/dev/console/console.h"
#include "manager/client_manager.h"
#include "manager/console_manager.h"
#include "manager/server_manager.h"
#include "input/input.h"
#include "util/resource_loader.h"
#include "sound_manager.h"

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
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    InitAudioDevice();
    //--------------------------------------------------------------------------------------

    setup();
    ResourceLoader::load();
    SoundManager::init();

    ScreenManager screenManager{};

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        InputManager::get()->process();

        if (ClientManager::has()) {
            ClientManager::get().update();
            //TODO call player update func
        }

        if (InputManager::get()->isPressed("dev_console")) {
            if (ConsoleManager::has()) {
                ConsoleManager::get().setOpen(!ConsoleManager::get().isOpen());
            }
        }
        if (ConsoleManager::has() && ConsoleManager::get().isOpen()) ConsoleManager::get().handleInput();

        if(InputManager::get()->isPressed("ui_click")){
            ConsoleManager::get().log(INFO, "walk is held");
        }

        SoundManager::update();

        screenManager.update();
        draw(&screenManager);
        //----------------------------------------------------------------------------------
    }

    shutdown();

    return 0;
}

void setup() {
    Net::init();
    ConsoleManager::create();

    InputManager::get()->init(ASSETS_PATH "pixel_game/config/keybinds.json");
    InputManager::get()->setContext("menu");
}

void draw(ScreenManager* screenManager) {
    BeginDrawing();

    ClearBackground(WHITE);
    screenManager->draw();

    if (ClientManager::has()) {
        if(ClientManager::get().mState == NetState::IDLE) DrawText("Type ip of server to conenct", 10, 50, 20, GREEN);
        else if(ClientManager::get().mState == NetState::CONNECTING) DrawText("Connecting to server...", 10, 50, 20, GREEN);
        else if(ClientManager::get().mState == NetState::READY) DrawText("Ready to play", 10, 50, 20, GREEN);
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

    SoundManager::shutdown();
    ResourceLoader::unload();

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseAudioDevice();
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
}