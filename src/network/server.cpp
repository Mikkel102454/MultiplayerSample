#include "network/server.h"

#include <cstring>
#include <iostream>
#include <thread>

#include "raylib.h"
#include "network/packets.h"
#include "../../include/util/dev/console/dev.h"

Server* serverRef;

Server* Server_Get() {
    return serverRef;
}
void Server_Set(Server* server) {
    serverRef = server;
}
bool Server_Has() {
    return serverRef != nullptr;
}


/**
 *
 * Initialize a server
 *
 * @param server
 * @param addr
 * @param max_clients
 */
void Server_Init(Server* server, NetAddress addr, int max_clients) {
    server->socket = Socket_Create(NET_TCP, true);
    server->max_clients = max_clients;
    if (Socket_Bind(server->socket, addr) != NET_OK) {
        Console_Log(FATAL, "Server: Failed to bind to socket");
        return;
    }

    if (Socket_Listen(server->socket, max_clients * 2) != NET_OK) {
        Console_Log(FATAL, "Server: Failed to listen on socket");
        return;
    }

    server->clients = std::make_unique<Client[]>(max_clients);
}

/**
 *
 * Process packages a client has sent. this will run until all packages not proccess
 *
 * @param server
 * @param client
 */
void Client_ProcessPackage(Server* server, Client* client) {
    while (true) {
        PacketType packetType;

        NetResult res = Packet_GetNextType(client->sock, &packetType);

        if (res == NET_DISCONNECTED) {
            Server_RemoveClient(server, client->id, DIS_LEFT);
            break;
        }
        if (res != NET_OK) {
            break;
        }
        switch (packetType) {
            case PCK_NOTHING:
                return;
            case PCK_DISCONNECT: {
                char buffer[sizeof(DisconnectedPacket)];
                res = Packet_Recv(client->sock, buffer, sizeof(buffer));

                DisconnectedPacket packet;
                Packet_Deserialize(buffer, &packet, sizeof(packet));

                Console_Log(INFO, "Server: Client left the game: %d", packet.reason);
                Server_RemoveClient(server, client->id, DIS_LEFT);
                break;
            }
            case PCK_CONNECT: {
                char recv_buffer[sizeof(ConnectPacket)];
                res = Packet_Recv(client->sock, recv_buffer, sizeof(recv_buffer));

                ConnectPacket packet;
                Packet_Deserialize(recv_buffer, &packet, sizeof(packet));
                if (res == NET_DISCONNECTED) {
                    Server_RemoveClient(server, client->id, DIS_LEFT);
                    break;
                }

                Console_Log(INFO, "Server: New client named: %s", packet.name);

                std::memcpy(client->name, &packet.name[0], 25);
                client->connected = true;
                client->accepted = true;

                char send_buffer[sizeof(AcceptedPacket) + 1];
                Packet_Serialize(PCK_ACCEPTED, nullptr, 0, send_buffer);
                Packet_Send(client->sock, send_buffer, sizeof(send_buffer));

                break;
            }
            case PCK_PLAYERLIST_REQUEST: {
                char recv_buffer[sizeof(PlayerListRequestPacket)];
                res = Packet_Recv(client->sock, recv_buffer, sizeof(recv_buffer));

                PlayerListRequestPacket packet;
                Packet_Deserialize(recv_buffer, &packet, sizeof(packet));

                if (res == NET_DISCONNECTED) {
                    Server_RemoveClient(server, client->id, DIS_LEFT);
                    break;
                }

                PlayerListHeaderPacket headerPacket{};
                headerPacket.playerCount = server->client_count;
                char send_buffer[sizeof(PlayerListHeaderPacket) + 1];

                Packet_Serialize(PCK_PLAYERLIST_REQUEST, &headerPacket, sizeof(headerPacket), send_buffer);
                Packet_Send(client->sock, send_buffer, sizeof(send_buffer));

                for (int i = 0; i < server->max_clients; i++) {
                    if(!server->clients[i].accepted) continue;
                    PlayerListPacket playerPacket{};
                    playerPacket.id = i;
                    std::memcpy(playerPacket.name, server->clients[i].name, 25);
                    char send_buffer_player[sizeof(PlayerListPacket) + 1];

                    Packet_Serialize(PCK_PLAYERLIST_REQUEST, &headerPacket, sizeof(headerPacket), send_buffer);
                    Packet_Send(client->sock, send_buffer_player, sizeof(send_buffer_player));
                }
                break;
            }
            default:
                Console_Log(WARNING, "Server: Unknown package type: %d", packetType);
                return;
        }
    }
}

/**
 *
 * Accepts all queued client connection request if there is space
 *
 * @param server
 */
void Server_AcceptClients(Server* server) {
    for(int i = 0; i < server->max_clients; i++){
        if(server->clients[i].connected) continue;

        NetSocket sock{};
        NetAddress addr{};
        NetResult result = Socket_Accept(server->socket, &sock, &addr);
        if (result != NET_OK) {
            break;
        }

        server->clients[i].connected = true;
        server->clients[i].id = i;
        server->clients[i].sock = sock;
        server->clients[i].addr = addr;

        server->client_count++;
    }
}

/**
 *
 * Removes a client from the server and send a disconnect package if socket is open
 *
 * @param server
 * @param id
 */
void Server_RemoveClient(Server* server, int id, DisconnectReason reason) {
    Console_Log(INFO, "Server: Removing client %d", id);

    DisconnectedPacket disconnectedPacket{};
    disconnectedPacket.reason = reason;
    char send_buffer[sizeof(PlayerListHeaderPacket) + 1];

    Packet_Serialize(PCK_DISCONNECT, &disconnectedPacket, sizeof(disconnectedPacket), send_buffer);
    Packet_Send(server->clients[id].sock, send_buffer, sizeof(send_buffer));

    Socket_Close(server->clients[id].sock);

    server->clients[id].connected = false;
    server->clients[id].accepted = false;
    server->client_count--;
}

/**
 *
 * Loop through clients and remove dead clients.
 * Starts Client_ProcessPackage for each client
 *
 * @param server
 */
void Server_ProcessClients(Server* server) {
    for (int i = 0; i < server->max_clients; i++) {
        if (!server->clients[i].connected) continue;
        if (Socket_Poll(&server->clients[i].sock, 1, 0, &server->clients[i].readable, &server->clients[i].writable) != NET_OK) {
            Console_Log(FATAL, "Server: Failed to poll client %d", i);
        }
    }

    for (int i = 0; i < server->max_clients; i++) {
        if (!server->clients[i].connected) continue;


        Client_ProcessPackage(server, &server->clients[i]);
    }
}

/**
 *
 * Sleep for the reaming time of this tick and warns if tick have been skipped
 *
 * @param server
 * @return the number of tick that was skipped because of stress
 */
void Server_Sleep(Server* server, double tickStartTimeMs) {
    constexpr double TICK_MS = 33.333;

    double nowMs =
        std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();

    double elapsed = nowMs - tickStartTimeMs;

    if (elapsed < TICK_MS) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(
                static_cast<long long>(TICK_MS - elapsed)
            )
        );
        return;
    }

    // Tick overran → calculate skipped ticks
    int skippedTicks = static_cast<int>(elapsed / TICK_MS) - 1;
    if (skippedTicks < 0) skippedTicks = 0;
    Console_Log(SUCCESS, "Server is running behind! Skipped %d ticks", skippedTicks);

    server->tick++;
}

/**
 *
 * Start the server. This will start the ticking process and begin accepting clients
 *
 * @param server
 */
void Server_Run(Server* server) {
    Console_Log(SUCCESS, "Successfully started server");
    while (true) {
        if (server->stopped) {
            // do shutdown logic
            Server_Destroy(server);
            break;
        }
        auto tickStart = std::chrono::steady_clock::now();

        // for client shit (important)
        Server_AcceptClients(server);
        Server_ProcessClients(server);

        // Tick logic goes here

        double tickStartMs = std::chrono::duration<double, std::milli>(
                tickStart.time_since_epoch()
            ).count();

        Server_Sleep(server, tickStartMs);
    }
}

/**
 *
 * Closes the server
 *
 * @param server
 */
void Server_Destroy(Server* server) {
    for (int i = 0; i < server->max_clients; i++) {
        if (!server->clients[i].accepted) continue;
        Server_RemoveClient(server, i, DIS_CLOSE);
    }
    Socket_Close(server->socket);
    delete server;
    serverRef = nullptr;
}