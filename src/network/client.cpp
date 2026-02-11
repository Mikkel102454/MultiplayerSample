#include "network/client.h"

#include "manager/client_manager.h"
#include "manager/console_manager.h"
#include "network/packets.h"
#include "network/packets/player_disconnect_packet.h"
#include "util/dev/console/console.h"

/**
 *
 * Initializes a new client
 *
 * @param serverAddr
 *
 */
Client::Client(const Net::Address& serverAddr) {
    mState = NetState::IDLE;
    mServerAddr = serverAddr;
    mServer = Socket::create(Net::Protocol::NET_TCP, false);
}

void Client::connect() {
    mState = NetState::CONNECTING;
    if(Socket::connect(mServer, mServerAddr) == Net::Result::NET_ERROR) {
        ConsoleManager::get().log(WARNING, "Client: Failed to connect socket to server");

        mState = NetState::IDLE;
    }
}

void Client::disconnect() {
    ConsoleManager::get().log(WARNING, "Client: Lost connection to the server");

    PlayerDisconnectPacket disconnectedPacket{};
    disconnectedPacket.reason = DisconnectReason::DIS_LEFT;
    disconnectedPacket.id = -1;
    disconnectedPacket.announce = false;

    PacketIO::sendPacket(mServer, disconnectedPacket);

    Socket::close(mServer);
}

void Client::update() {
    processNetwork();
}

/**
 *
 * Update the client
 *
 */
void Client::processNetwork() {
    if (mState == NetState::IDLE) {
        return;
    }

    Net::Result res = Socket::poll(&mServer, 1, 0, &mReadable, &mWritable);

    if (res != Net::Result::NET_OK) {
        ConsoleManager::get().log(WARNING, "Client: Failed to poll data from server");
        return;
    }

    while (mReadable) {
        Net::Result res = Socket::poll(&mServer, 1, 0, &mReadable, &mWritable);

        if (res != Net::Result::NET_OK) {
                return;
        }

        if (!mReadable) return;

        std::unique_ptr<IPacket> pkt;
        res = PacketIO::receivePacket(mServer, pkt);

        if (res == Net::Result::NET_DISCONNECTED) {
            ClientManager::leave();
            break;
        }
        if (res != Net::Result::NET_OK) {
            break;
        }
        if (!pkt) {
            continue;
        }

        pkt->handleClient(this);
    }
}

Client::~Client() {
    disconnect();
}
