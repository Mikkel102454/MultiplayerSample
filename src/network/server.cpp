#include "network/server.h"

#include <cstring>
#include <iostream>
#include <thread>

#include "manager/console_manager.h"
#include "network/packets.h"
#include "util/dev/console/console.h"

/**
 *
 * Initialize a server
 *
 * @param address
 * @param maxClients
 */
Server::Server(const Net::Address &address, int maxClients) {
    mSocket = Socket::create(Net::Protocol::NET_TCP, true);
    this->mMaxClients = maxClients;
    if (Socket::bind(mSocket, address) != Net::Result::NET_OK) {
        ConsoleManager::get().log(FATAL, "Server: Failed to bind to socket");
        return;
    }

    if (Socket::listen(mSocket, mMaxClients * 2) != Net::Result::NET_OK) {
        ConsoleManager::get().log(FATAL, "Server: Failed to listen on socket");
        return;
    }
}

/**
 *
 * Process packages a client has sent. this will run until all packages not proccess
 *
 * @param client
 */
void Server::processPackage(Client* client) {
    while (true) {
        PacketType packetType;

        Net::Result res = Packet::getNextType(client->sock, &packetType);

        if (res == Net::Result::NET_DISCONNECTED) {
            removeClient(client->id, DisconnectReason::DIS_LEFT);
            break;
        }
        if (res != Net::Result::NET_OK) {
            break;
        }
        switch (packetType) {
            case PacketType::PCK_NOTHING:
                return;
            case PacketType::PCK_DISCONNECT: {
                char buffer[sizeof(PlayerDisconnectPacket)]{};
                res = Packet::receive(client->sock, buffer, sizeof(buffer));

                PlayerDisconnectPacket packet;
                Packet::deserialize(buffer, &packet, sizeof(packet));

                ConsoleManager::get().log(INFO, "Server: Client left the game: %d", packet.reason);
                removeClient(client->id, DisconnectReason::DIS_LEFT);
                break;
            }
            case PacketType::PCK_CONNECT: {
                char recvBuffer[sizeof(ConnectPacket)]{};
                res = Packet::receive(client->sock, recvBuffer, sizeof(recvBuffer));

                ConnectPacket packet;
                Packet::deserialize(recvBuffer, &packet, sizeof(packet));
                if (res == Net::Result::NET_DISCONNECTED) {
                    removeClient(client->id, DisconnectReason::DIS_LEFT);
                    break;
                }

                if (packet.id != -1) {
                    ConsoleManager::get().log(INFO, "Server: Somebody tried to join the server with a set id");
                    return;
                }

                ConsoleManager::get().log(INFO, "Server: New client named: %s", packet.name);

                //Broadcast to everyone new client joined
                PlayerJoinPacket joinPacket{};
                joinPacket.id = client->id;
                joinPacket.announce = true;
                std::memcpy(joinPacket.name, packet.name, 25);
;
                char sendBufferPlayerJoin[sizeof(PlayerJoinPacket)]{};

                Packet::serialize(PacketType::PCK_JOIN, &joinPacket, sizeof(joinPacket), recvBuffer);
                broadcast(sendBufferPlayerJoin, sizeof(sendBufferPlayerJoin));

                std::memcpy(client->name, &packet.name[0], 25);
                client->connected = true;
                client->accepted = true;

                packet.id = client->id;

                char sendBuffer[sizeof(ConnectPacket) + 1]{};
                Packet::serialize(PacketType::PCK_CONNECT, nullptr, 0, sendBuffer);
                Packet::send(client->sock, sendBuffer, sizeof(sendBuffer));

                for (int i = 0; i < mClients.size(); i++) {
                    if (i == client->id) continue;
                    if(!mClients[i].accepted) continue;
                    PlayerJoinPacket playerPacket{};
                    playerPacket.id = i;
                    playerPacket.announce = false;
                    std::memcpy(playerPacket.name, mClients[i].name, 25);
                    char sendBufferPlayer[sizeof(PlayerJoinPacket) + 1]{};

                    Packet::serialize(PacketType::PCK_CONNECT, &playerPacket, sizeof(playerPacket), sendBufferPlayer);
                    Packet::send(client->sock, sendBufferPlayer, sizeof(sendBufferPlayer));
                }
                break;
            }
            default:
                ConsoleManager::get().log(WARNING, "Server: Unknown package type: %d", packetType);
                return;
        }
    }
}

/**
 *
 * Accepts all queued client connection request if there is space
 *
 */
void Server::acceptClients()
{

    for (int i = 0; i < mMaxClients - mClients.size(); i++)
    {
        Socket sock{};
        Net::Address addr{};
        Net::Result result = Socket::accept(mSocket, &sock, &addr);
        if (result != Net::Result::NET_OK) {
            break;
        }

        int id = 0;

        while (true)
        {
            bool found = false;
            for (const auto& client : mClients)
            {
                if (client.id == id)
                {
                    found = true;
                    break;
                }
            }

            if (!found) break;
            id++;
        }

        Client client;
        client.id = id;
        client.connected = true;
        client.accepted = true;
        client.sock = sock;
        client.addr = addr;

        mClients.push_back(client);
    }
}

/**
 *
 * Removes a client from the server and send a disconnect package if socket is open
 *
 * @param id
 * @param reason
 */
void Server::removeClient(const int id, const DisconnectReason reason) {
    ConsoleManager::get().log(INFO, "Server: Removing client %d", id);

    PlayerDisconnectPacket disconnectedPacket{};
    disconnectedPacket.reason = reason;
    disconnectedPacket.id = -1;
    disconnectedPacket.announce = false;
    char sendBuffer[sizeof(PlayerDisconnectPacket) + 1]{};

    Packet::serialize(PacketType::PCK_DISCONNECT, &disconnectedPacket, sizeof(disconnectedPacket), sendBuffer);
    Packet::send(mClients[id].sock, sendBuffer, sizeof(sendBuffer));

    Socket::close(mClients[id].sock);

    for (auto i = mClients.begin(); i != mClients.end(); ++i) {
        if (i->id == id) {
            mClients.erase(i);
            break;
        }
    }

    disconnectedPacket.id = id;
    disconnectedPacket.announce = true;
    Packet::serialize(PacketType::PCK_DISCONNECT, &disconnectedPacket, sizeof(disconnectedPacket), sendBuffer);
    broadcast(sendBuffer, sizeof(sendBuffer));

}

bool Server::isRunning() const {
    return mRunning;
}

/**
 *
 * Loop through clients and remove dead clients.
 * Starts Client_ProcessPackage for each client
 *
 */
void Server::processClients() {
    for (int i = 0; i < mClients.size(); i++) {
        if (!mClients[i].connected) continue;
        if (Socket::poll(&mClients[i].sock, 1, 0, &mClients[i].readable, &mClients[i].writable) != Net::Result::NET_OK) {
            ConsoleManager::get().log(FATAL, "Server: Failed to poll client %d", i);
        }
    }

    for (int i = 0; i < mClients.size(); i++) {
        if (!mClients[i].connected) continue;


        processPackage(&mClients[i]);
    }
}

/**
 *
 * Sleep for the reaming time of this tick and warns if tick have been skipped
 *
 * @return the number of tick that was skipped because of stress
 */
void Server::sleep(const double tickStartTimeMs) {
    constexpr double tickMs = 33.333;

    const double nowMs =
        std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();

    const double elapsed = nowMs - tickStartTimeMs;

    if (elapsed < tickMs) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(
                static_cast<long long>(tickMs - elapsed)
            )
        );
        return;
    }

    int skippedTicks = static_cast<int>(elapsed / tickMs) - 1;
    if (skippedTicks < 0) skippedTicks = 0;
    ConsoleManager::get().log(SUCCESS, "Server is running behind! Skipped %d ticks", skippedTicks);

    mTick++;
}

/**
 *
 * Start the server. This will start the ticking process and begin accepting clients
 *
 */
void Server::run() {
    std::thread([&] {
        ConsoleManager::get().log(SUCCESS, "Successfully started server");
        mRunning = true;
        while (mRunning) {
            auto tickStart = std::chrono::steady_clock::now();

            // for client shit (important)
            acceptClients();
            processClients();

            // Tick logic goes here

            double tickStartMs = std::chrono::duration<double, std::milli>(
                    tickStart.time_since_epoch()
                ).count();

            sleep(tickStartMs);
        }
    }).detach();
}

/**
 *
 * Closes the server
 *
 */
Server::~Server() {
    mRunning = false;
    for (int i = 0; i < mClients.size(); i++) {
        if (!mClients[i].accepted) continue;
        removeClient(i, DisconnectReason::DIS_CLOSE);
    }
    Socket::close(mSocket);
}

/**
 *
 * Send stop
 *
 */
void Server::stop() {
    mRunning = false;
}

void Server::broadcast(const char* buffer, int bufferSize) {
    for (auto & mClient : mClients) {
        Packet::send(mClient.sock, buffer, bufferSize);
    }
}
