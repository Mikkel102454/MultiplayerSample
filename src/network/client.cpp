#include "network/client.h"

#include "manager/client_manager.h"
#include "manager/console_manager.h"
#include "network/packets.h"
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
    char sendBuffer[sizeof(disconnectedPacket) + 1]{};

    Packet::serialize(PacketType::PCK_DISCONNECT, &disconnectedPacket, sizeof(disconnectedPacket), sendBuffer);
    Packet::send(mServer, sendBuffer, sizeof(sendBuffer));

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

    if (mReadable) {
        while (true) {
            res = Socket::poll(&mServer, 1, 0, &mReadable, &mWritable);
            if (!mReadable || res != Net::Result::NET_OK) {
                return;
            }

            PacketType packetType;

            res = Packet::getNextType(mServer, &packetType);

            if (res == Net::Result::NET_DISCONNECTED) {
                ClientManager::leave();
                break;
            }

            if (res != Net::Result::NET_OK) {
                break;
            }
            switch (packetType) {
                case PacketType::PCK_NOTHING:
                    return;
                case PacketType::PCK_CONNECT: {
                    char recvBuffer[sizeof(ConnectPacket)]{};
                    res = Packet::receive(mServer, recvBuffer, sizeof(recvBuffer));

                    ConnectPacket packet;
                    Packet::deserialize(recvBuffer, &packet, sizeof(packet));
                    if (res == Net::Result::NET_DISCONNECTED) {
                        ClientManager::leave();
                        break;
                    }

                    mState = NetState::READY;
                    ConsoleManager::get().log(SUCCESS, "Client: Connected to server");
                    break;
                }
                case PacketType::PCK_DISCONNECT: {
                    char recvBuffer[sizeof(PlayerDisconnectPacket)]{};
                    res = Packet::receive(mServer, recvBuffer, sizeof(recvBuffer));

                    PlayerDisconnectPacket packet;
                    Packet::deserialize(recvBuffer, &packet, sizeof(packet));

                    if (packet.id == -1) {
                        // get outa here
                        ClientManager::leave();
                        return;
                    }

                    // somebody else left
                    ConsoleManager::get().log(SUCCESS, "Client: Player %d left the game", packet.id);

                    if (packet.announce) {

                    }

                    break;
                }
                case PacketType::PCK_JOIN: {
                    char recvBuffer[sizeof(PlayerJoinPacket)]{};
                    res = Packet::receive(mServer, recvBuffer, sizeof(recvBuffer));

                    PlayerJoinPacket packet;
                    Packet::deserialize(recvBuffer, &packet, sizeof(packet));
                    if (res == Net::Result::NET_DISCONNECTED) {
                        ClientManager::leave();
                        break;
                    }
                    ConsoleManager::get().log(SUCCESS, "Client: New client discovered with the name %s and id %d", packet.name, packet.id);

                    if (packet.announce) {

                    }

                    break;
                }
                default:
                    ConsoleManager::get().log(WARNING, "Client: Unknown package type: %d", packetType);
                    return;
            }
        }
    }
}

Client::~Client() {
    disconnect();
}
