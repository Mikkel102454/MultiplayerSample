#include "network/client.h"

#include "network/packets.h"
#include "util/dev/console/console.h"

Client::Net_Client* Client::client = nullptr;

Client::Net_Client* Client::Get() {
    return client;
}
bool Client::Has() {
    return client != nullptr;
}
/**
 *
 * Initializes a new client
 *
 * @param newClient
 * @param addr
 */
auto Client::Init(Net::Address addr) -> void {
    client = new Net_Client();
    client->state = Net_State::IDLE;
    client->addr = addr;
    client->server = Socket::Create(Net::Protocol::NET_TCP, true);
}
void Client::Connect() {
    client->state = Net_State::CONNECTING;
    if(Socket::Connect(client->server, client->addr) == Net::Result::NET_ERROR) {
        Console::Log(WARNING, "Client: Failed to connect socket to server");

        client->state = Net_State::IDLE;
    }
}

void Client::Update() {
    if (client->state == Net_State::IDLE) {
        return;
    }

    Net::Result res = Socket::Poll(&client->server, 1, 0, &client->readable, &client->writeable);

    if (res != Net::Result::NET_OK) {
        Console::Log(WARNING, "Client: Failed to poll data from server");
        return;
    }

    if (client->readable) {
        while (true) {
            PacketType packetType;

            res = Packet::GetNextType(client->server, &packetType);

            if (res == Net::Result::NET_DISCONNECTED) {
                Destroy();
                break;
            }

            if (res != Net::Result::NET_OK) {
                break;
            }
            switch (packetType) {
                case PacketType::PCK_NOTHING:
                    return;
                case PacketType::PCK_ACCEPTED: {
                    char recv_buffer[sizeof(AcceptedPacket)]{};
                    res = Packet::Receive(client->server, recv_buffer, sizeof(recv_buffer));

                    AcceptedPacket packet;
                    Packet::Deserialize(recv_buffer, &packet, sizeof(packet));
                    if (res == Net::Result::NET_DISCONNECTED) {
                        Destroy();
                        break;
                    }

                    client->state = Net_State::READY;
                    Console::Log(SUCCESS, "Client: Connected to server");
                    break;
                }
                case PacketType::PCK_DISCONNECT: {
                    char recv_buffer[sizeof(DisconnectedPacket)]{};
                    res = Packet::Receive(client->server, recv_buffer, sizeof(recv_buffer));

                    DisconnectedPacket packet;
                    Packet::Deserialize(recv_buffer, &packet, sizeof(packet));
                    Destroy();
                    return;;
                }
                case PacketType::PCK_PLAYERLIST_HEADER: {
                    char recv_buffer[sizeof(PlayerListHeaderPacket)]{};
                    res = Packet::Receive(client->server, recv_buffer, sizeof(recv_buffer));

                    PlayerListHeaderPacket packet;
                    Packet::Deserialize(recv_buffer, &packet, sizeof(packet));
                    if (res == Net::Result::NET_DISCONNECTED) {
                        Destroy();
                        break;
                    }

                    Console::Log(SUCCESS, "Client: There is %d players in this game", packet.playerCount);
                    break;
                }
                case PacketType::PCK_PLAYERLIST: {
                    char recv_buffer[sizeof(PlayerListPacket)]{};
                    res = Packet::Receive(client->server, recv_buffer, sizeof(recv_buffer));

                    PlayerListPacket packet;
                    Packet::Deserialize(recv_buffer, &packet, sizeof(packet));
                    if (res == Net::Result::NET_DISCONNECTED) {
                        Destroy();
                        break;
                    }

                    Console::Log(SUCCESS, "Client: Client discovered with the name %s and id %d", packet.name, packet.id);
                    break;
                }
                default:
                    Console::Log(WARNING, "Client: Unknown package type: %d", packetType);
                    return;
            }
        }
    }
}

void Client::Destroy() {
    Console::Log(WARNING, "Client: Lost connection to the server");

    DisconnectedPacket disconnectedPacket{};
    disconnectedPacket.reason = DisconnectReason::DIS_LEFT;
    char send_buffer[sizeof(PlayerListHeaderPacket) + 1]{};

    Packet::Serialize(PacketType::PCK_DISCONNECT, &disconnectedPacket, sizeof(disconnectedPacket), send_buffer);
    Packet::Send(client->server, send_buffer, sizeof(send_buffer));

    Socket::Close(client->server);

    delete client;
    client = nullptr;
}
