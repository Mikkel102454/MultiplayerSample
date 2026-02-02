#include "network/client.h"

#include <iostream>

#include "raylib.h"
#include "network/packets.h"
#include "network/server.h"

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
        TraceLog(LOG_WARNING, "Client: Failed to connect socket to server");
        client->state = IDLE;
    }
}

void Client_Update (Net_Client* client){
    if (client->state == IDLE) {
        return;
    }

    NetResult res = Socket_Poll(&client->server, 1, 0, &client->readable, &client->writeable);

    if (res != NET_OK) {
        TraceLog(LOG_WARNING, "Client: Failed to poll data from server");

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
                    TraceLog(LOG_INFO, "Client: Connected to server");
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
                    TraceLog(LOG_WARNING, "Client: Unknown package type: %d", packetType);
                    return;
            }
        }
    }
}

void Client_Disconnect(Net_Client* client) {
    TraceLog(LOG_INFO, "Client: Lost connection to the server");
    Packet_SendDisconnect(client->server, DIS_LEFT);
    client->state = IDLE;
}
