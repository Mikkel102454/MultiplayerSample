#include "network/server.h"

#include <cstring>
#include <iostream>
#include <thread>

#include "network/packets.h"
#include "util/dev/console/console.h"

Server *Server::server = nullptr;

Server* Server::Get() {
    return server;
}
bool Server::Has() {
    return server != nullptr;
}


/**
 *
 * Initialize a server
 *
 * @param addr
 * @param max_clients
 */
void Server::Init(const Net::Address addr, const int max_clients) {
    server = new Server();
    server->socket = Socket::Create(Net::Protocol::NET_TCP, true);
    server->max_clients = max_clients;
    if (Socket::Bind(server->socket, addr) != Net::Result::NET_OK) {
        Console::Log(FATAL, "Server: Failed to bind to socket");
        return;
    }

    if (Socket::Listen(server->socket, max_clients * 2) != Net::Result::NET_OK) {
        Console::Log(FATAL, "Server: Failed to listen on socket");
        return;
    }

    server->clients = std::make_unique<Client[]>(max_clients);
}

/**
 *
 * Process packages a client has sent. this will run until all packages not proccess
 *
 * @param client
 */
void Server::ProcessPackage(Client* client) {
    while (true) {
        PacketType packetType;

        Net::Result res = Packet::GetNextType(client->sock, &packetType);

        if (res == Net::Result::NET_DISCONNECTED) {
            RemoveClient(client->id, DisconnectReason::DIS_LEFT);
            break;
        }
        if (res != Net::Result::NET_OK) {
            break;
        }
        switch (packetType) {
            case PacketType::PCK_NOTHING:
                return;
            case PacketType::PCK_DISCONNECT: {
                char buffer[sizeof(DisconnectedPacket)]{};
                res = Packet::Receive(client->sock, buffer, sizeof(buffer));

                DisconnectedPacket packet;
                Packet::Deserialize(buffer, &packet, sizeof(packet));

                Console::Log(INFO, "Server: Client left the game: %d", packet.reason);
                RemoveClient(client->id, DisconnectReason::DIS_LEFT);
                break;
            }
            case PacketType::PCK_CONNECT: {
                char recv_buffer[sizeof(ConnectPacket)]{};
                res = Packet::Receive(client->sock, recv_buffer, sizeof(recv_buffer));

                ConnectPacket packet;
                Packet::Deserialize(recv_buffer, &packet, sizeof(packet));
                if (res == Net::Result::NET_DISCONNECTED) {
                    RemoveClient(client->id, DisconnectReason::DIS_LEFT);
                    break;
                }

                Console::Log(INFO, "Server: New client named: %s", packet.name);

                std::memcpy(client->name, &packet.name[0], 25);
                client->connected = true;
                client->accepted = true;

                char send_buffer[sizeof(AcceptedPacket) + 1]{};
                Packet::Serialize(PacketType::PCK_ACCEPTED, nullptr, 0, send_buffer);
                Packet::Send(client->sock, send_buffer, sizeof(send_buffer));

                break;
            }
            case PacketType::PCK_PLAYERLIST_REQUEST: {
                char recv_buffer[sizeof(PlayerListRequestPacket)]{};
                res = Packet::Receive(client->sock, recv_buffer, sizeof(recv_buffer));

                PlayerListRequestPacket packet;
                Packet::Deserialize(recv_buffer, &packet, sizeof(packet));

                if (res == Net::Result::NET_DISCONNECTED) {
                    RemoveClient(client->id, DisconnectReason::DIS_LEFT);
                    break;
                }

                PlayerListHeaderPacket headerPacket{};
                headerPacket.playerCount = server->client_count;
                char send_buffer[sizeof(PlayerListHeaderPacket) + 1]{};

                Packet::Serialize(PacketType::PCK_PLAYERLIST_HEADER, &headerPacket, sizeof(headerPacket), send_buffer);
                Packet::Send(client->sock, send_buffer, sizeof(send_buffer));

                for (int i = 0; i < server->max_clients; i++) {
                    if(!server->clients[i].accepted) continue;
                    PlayerListPacket playerPacket{};
                    playerPacket.id = i;
                    std::memcpy(playerPacket.name, server->clients[i].name, 25);
                    char send_buffer_player[sizeof(PlayerListPacket) + 1]{};

                    Packet::Serialize(PacketType::PCK_PLAYERLIST, &playerPacket, sizeof(playerPacket), send_buffer_player);
                    Packet::Send(client->sock, send_buffer_player, sizeof(send_buffer_player));
                }
                break;
            }
            default:
                Console::Log(WARNING, "Server: Unknown package type: %d", packetType);
                return;
        }
    }
}

/**
 *
 * Accepts all queued client connection request if there is space
 *
 */
void Server::AcceptClients() {
    for(int i = 0; i < server->max_clients; i++){
        if(server->clients[i].connected) continue;

        Socket sock{};
        Net::Address addr{};
        Net::Result result = Socket::Accept(server->socket, &sock, &addr);
        if (result != Net::Result::NET_OK) {
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
 * @param id
 * @param reason
 */
void Server::RemoveClient(const int id, const DisconnectReason reason) {
    Console::Log(INFO, "Server: Removing client %d", id);

    DisconnectedPacket disconnectedPacket{};
    disconnectedPacket.reason = reason;
    char send_buffer[sizeof(PlayerListHeaderPacket) + 1]{};

    Packet::Serialize(PacketType::PCK_DISCONNECT, &disconnectedPacket, sizeof(disconnectedPacket), send_buffer);
    Packet::Send(server->clients[id].sock, send_buffer, sizeof(send_buffer));

    Socket::Close(server->clients[id].sock);

    server->clients[id].connected = false;
    server->clients[id].accepted = false;
    server->client_count--;
}

/**
 *
 * Loop through clients and remove dead clients.
 * Starts Client_ProcessPackage for each client
 *
 */
void Server::ProcessClients() {
    for (int i = 0; i < server->max_clients; i++) {
        if (!server->clients[i].connected) continue;
        if (Socket::Poll(&server->clients[i].sock, 1, 0, &server->clients[i].readable, &server->clients[i].writable) != Net::Result::NET_OK) {
            Console::Log(FATAL, "Server: Failed to poll client %d", i);
        }
    }

    for (int i = 0; i < server->max_clients; i++) {
        if (!server->clients[i].connected) continue;


        ProcessPackage(&server->clients[i]);
    }
}

/**
 *
 * Sleep for the reaming time of this tick and warns if tick have been skipped
 *
 * @return the number of tick that was skipped because of stress
 */
void Server::Sleep(const double tickStartTimeMs) {
    constexpr double TICK_MS = 33.333;

    const double nowMs =
        std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();

    const double elapsed = nowMs - tickStartTimeMs;

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
    Console::Log(SUCCESS, "Server is running behind! Skipped %d ticks", skippedTicks);

    server->tick++;
}

/**
 *
 * Start the server. This will start the ticking process and begin accepting clients
 *
 */
void Server::Run() {
    Console::Log(SUCCESS, "Successfully started server");
    while (true) {
        if (server->stopped) {
            // do shut down logic
            Destroy();
            return;
        }
        auto tickStart = std::chrono::steady_clock::now();

        // for client shit (important)
        AcceptClients();
        ProcessClients();

        // Tick logic goes here

        double tickStartMs = std::chrono::duration<double, std::milli>(
                tickStart.time_since_epoch()
            ).count();

        Sleep(tickStartMs);
    }
}

/**
 *
 * Closes the server
 *
 */
void Server::Destroy() {
    for (int i = 0; i < server->max_clients; i++) {
        if (!server->clients[i].accepted) continue;
        RemoveClient(i, DisconnectReason::DIS_CLOSE);
    }
    Socket::Close(server->socket);
    delete server;
    server = nullptr;
}

void Server::Stop() {
    server->stopped = true;
}