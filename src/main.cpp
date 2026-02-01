#include "raylib.h"
#include "network/client.h"
#include "network/server.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib example - multiplayer");

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    Net_Init();

    Server server{};
    NetAddress addressServer = Net_ResolveAddress("127.0.0.1", 59332);

    Server_Init(&server, addressServer, 4);

    Net_Client client;

    NetAddress addressClient = Net_ResolveAddress("127.0.0.1", 59332);

    Client_Connect(&client, addressClient);


    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        BeginDrawing();

        ClearBackground(GRAY);

        if(client.state == IDLE) DrawText("Type ip of server to conenct", 10, 50, 20, GREEN);
        else if(client.state == CONNECTING) DrawText("Connecting to server...", 10, 50, 20, GREEN);
        else if(client.state == ACCEPTING) DrawText("Awating server to accept...", 10, 50, 20, GREEN);
        else if(client.state == READY) DrawText("Ready to play", 10, 50, 20, GREEN);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    Net_Shutdown();
    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}