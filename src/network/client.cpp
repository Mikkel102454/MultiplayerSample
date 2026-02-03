#include "network/client.h"

#include <iostream>

#include "raylib.h"
#include "network/packets.h"
#include "util/dev.h"

Net_Client* net_clientRef;

Net_Client* Client_Get() {
    return net_clientRef;
}

void Client_Set(Net_Client* client) {
    net_clientRef = client;
}

bool Client_Has() {
    return net_clientRef != nullptr;
}
/**
 *
 * Connect client to a server with given address
 *
 * @param client
 * @param addr
 */
void Client_Init(Net_Client* client, NetAddress addr) {
    client->state = IDLE;
    client->addr = addr;
    client->server = Socket_Create(NET_TCP, true);
}
void Client_Connect(Net_Client* client) {
    client->state = CONNECTING;
    if(Socket_Connect(client->server, client->addr) == NET_ERROR) {
        Console_Log(WARNING, "Client: Failed to connect socket to server");

        client->state = IDLE;
        return;
    }
}

void Client_Update (Net_Client* client){
    if (client->state == IDLE) {
        return;
    }

    NetResult res = Socket_Poll(&client->server, 1, 0, &client->readable, &client->writeable);

    if (res != NET_OK) {
        Console_Log(WARNING, "Client: Failed to poll data from server");
        return;
    }

    if (client->readable) {
        while (true) {
            PacketType packetType;

            res = Packet_GetNextType(client->server, &packetType);

            if (res == NET_DISCONNECTED) {
                Client_Disconnect(client);
                break;
            }

            if (res != NET_OK) {
                break;
            }
            switch (packetType) {
                case PCK_NOTHING:
                    return;
                case PCK_ACCEPTED: {
                    AcceptedPacket packet;
                    res = Packet_RecvAccepted(client->server, &packet);
                    if (res == NET_DISCONNECTED) {
                        Client_Disconnect(client);
                        break;
                    }

                    client->state = READY;
                    Console_Log(SUCCESS, "Client: Connected to server");
                    break;
                }
                case PCK_DISCONNECT: {
                    DisconnectedPacket packet;
                    res = Packet_RecvDisconnect(client->server, &packet);
                    if (res == NET_DISCONNECTED) {
                        Client_Disconnect(client);
                        break;
                    }

                    Client_Disconnect(client);
                    break;
                }
                default:
                    Console_Log(WARNING, "Client: Unknown package type: %d", packetType);
                    return;
            }
        }
    }
}

void Client_Disconnect(Net_Client* client) {
    Console_Log(WARNING, "Client: Lost connection to the server");

    Packet_SendDisconnect(client->server, DIS_LEFT);
    Socket_Close(client->server);

    client->state = IDLE;
    net_clientRef = nullptr;
}

void Client_Destroy(Net_Client* client) {
    Client_Disconnect(client);
    delete client;
}
