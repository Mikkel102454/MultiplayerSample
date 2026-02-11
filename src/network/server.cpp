#include "network/server.h"

#include <cstring>
#include <iostream>
#include <thread>

#include "manager/console_manager.h"
#include "network/packets.h"
#include "network/packets/player_disconnect_packet.h"
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
        std::unique_ptr<IPacket> pkt;
        Net::Result res = PacketIO::receivePacket(client->sock, pkt);

        if (res == Net::Result::NET_DISCONNECTED) {
            removeClient(client->id, DisconnectReason::DIS_LEFT);
            break;
        }
        if (res != Net::Result::NET_OK) {
            break;
        }
        if (!pkt) {
            continue;
        }

        pkt->handleServer(this, client);
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
 * @param announce
 */
void Server::removeClient(const int id, const DisconnectReason reason, bool announce) {
    ConsoleManager::get().log(INFO, "Server: Removing client %d", id);

    PlayerDisconnectPacket disconnectedPacket{};
    disconnectedPacket.reason = reason;
    disconnectedPacket.id = -1;
    disconnectedPacket.announce = false;

    PacketIO::sendPacket(mClients[id].sock, disconnectedPacket);

    mClients[id].accepted = false;
    mClients[id].connected = false;
    Socket::close(mClients[id].sock);

    disconnectedPacket.id = id;
    disconnectedPacket.announce = announce;

    broadcastPacket(disconnectedPacket, true);
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
        removeClient(i, DisconnectReason::DIS_CLOSE, false);
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

void Server::broadcastPacket(const IPacket& packet, bool acceptedOnly) {
    for (auto& c : mClients) {
        if (acceptedOnly && !c.accepted) continue;
        PacketIO::sendPacket(c.sock, packet);
    }
}