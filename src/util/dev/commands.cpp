//
// Created by mikkel on 2/3/2026.
//

#include "util/dev/commands.h"

#include "raylib.h"
#include "network/client.h"
#include "network/packets.h"
#include "network/server.h"

void ConsoleCommandStartServer::execute(std::string_view command) {
    std::vector<std::string_view> args = extractArgs(command);

    if (args.size() != 3) {
        Console_Log(FATAL, "Unsupported command arguments. Please use 'start_server {ip} {port}'");
        return;
    }

    if (Server_Has()) {
        Console_Log(WARNING, "A server is already active. Please shutdown that server to make a new one");
        return;
    }

    uint16_t port;
    if (Net_ParsePort(args[2], port)) {
        Console_Log(FATAL, "The given port was invalid");
        return;
    }

    Server* server = new Server();
    const NetAddress addressServer = Net_ResolveAddress(args[1].data(), port);

    Server_Init(server, addressServer, 4);

    std::thread(Server_Run, server).detach();
    Server_Set(server);
}
void ConsoleCommandStopServer::execute(std::string_view command) {
    std::vector<std::string_view> args = extractArgs(command);

    if (args.size() != 1) {
        Console_Log(FATAL, "Unsupported command arguments. Please use 'stop_server'");
        return;
    }

    if (!Server_Has()) {
        Console_Log(WARNING, "There is no active server.'");
        return;
    }

    Server_Destroy(Server_Get());

    Console_Log(INFO, "Successfully shutdown server");

}
void ConsoleCommandJoinServer::execute(std::string_view command) {
    std::vector<std::string_view> args = extractArgs(command);

    if (args.size() != 4) {
        Console_Log(FATAL, "Unsupported command arguments. Please use 'join_server {ip} {port} {username}'");
        return;
    }

    if (Client_Has()) {
        Console_Log(WARNING, "You are already in a server. Please leave your current server before joining a new one");
        return;
    }

    uint16_t port;
    if (Net_ParsePort(args[2], port)) {
        Console_Log(FATAL, "The given port was invalid");
        return;
    }

    Net_Client* client = new Net_Client();

    const NetAddress addressClient = Net_ResolveAddress(args[1].data(), port);

    Client_Init(client, addressClient);
    Client_Connect(client);

    Packet_SendConnect(client->server, (args[3].data()));
}
void ConsoleCommandQuitServer::execute(std::string_view command) {
    std::vector<std::string_view> args = extractArgs(command);

    if (args.size() != 1) {
        Console_Log(FATAL, "Unsupported command arguments. Please use 'quit_server'");
        return;
    }

    if (!Client_Has()) {
        Console_Log(WARNING, "You are not in a server. Please join a server before you can leave one");
        return;
    }

    Client_Destroy(Client_Get());

    Console_Log(SUCCESS, "Successfully left server");
}