#include "network/client.h"

#include "manager/ClientManager.h"
#include "manager/ConsoleManager.h"
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

/**
 *
 * Update the client
 *
 */
void Client::update() {
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
                case PacketType::PCK_ACCEPTED: {
                    char recvBuffer[sizeof(AcceptedPacket)]{};
                    res = Packet::receive(mServer, recvBuffer, sizeof(recvBuffer));

                    AcceptedPacket packet;
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
                    char recvBuffer[sizeof(DisconnectedPacket)]{};
                    res = Packet::receive(mServer, recvBuffer, sizeof(recvBuffer));

                    DisconnectedPacket packet;
                    Packet::deserialize(recvBuffer, &packet, sizeof(packet));
                    ClientManager::leave();
                    return;
                }
                case PacketType::PCK_PLAYERLIST_HEADER: {
                    char recvBuffer[sizeof(PlayerListHeaderPacket)]{};
                    res = Packet::receive(mServer, recvBuffer, sizeof(recvBuffer));

                    PlayerListHeaderPacket packet;
                    Packet::deserialize(recvBuffer, &packet, sizeof(packet));
                    if (res == Net::Result::NET_DISCONNECTED) {
                        ClientManager::leave();
                        break;
                    }

                    ConsoleManager::get().log(SUCCESS, "Client: There is %d players in this game", packet.playerCount);
                    break;
                }
                case PacketType::PCK_PLAYERLIST: {
                    char recvBuffer[sizeof(PlayerListPacket)]{};
                    res = Packet::receive(mServer, recvBuffer, sizeof(recvBuffer));

                    PlayerListPacket packet;
                    Packet::deserialize(recvBuffer, &packet, sizeof(packet));
                    if (res == Net::Result::NET_DISCONNECTED) {
                        ClientManager::leave();
                        break;
                    }

                    ConsoleManager::get().log(SUCCESS, "Client: Client discovered with the name %s and id %d", packet.name, packet.id);
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
    ConsoleManager::get().log(WARNING, "Client: Lost connection to the server");

    DisconnectedPacket disconnectedPacket{};
    disconnectedPacket.reason = DisconnectReason::DIS_LEFT;
    char sendBuffer[sizeof(PlayerListHeaderPacket) + 1]{};

    Packet::serialize(PacketType::PCK_DISCONNECT, &disconnectedPacket, sizeof(disconnectedPacket), sendBuffer);
    Packet::send(mServer, sendBuffer, sizeof(sendBuffer));

    Socket::close(mServer);
}
