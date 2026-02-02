#include "network/client.h"

#include <iostream>

#include "network/packets.h"

/**
 *
 * Connect client to a server with given address
 *
 * @param client
 * @param addr
 */
void Client_Init(Net_Client* client) {
    client->state = IDLE;
}
void Client_Connect(Net_Client* client, NetAddress addr) {
    client->server = Socket_Create(NET_TCP, true);
    client->addr = addr;

    client->state = CONNECTING;
    if(Socket_Connect(client->server, addr) == NET_ERROR) {
        std::cout << "Client: Failed connect socket to server\n";
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
        std::cout << "Client: Failed to poll data from server.\n";
        return;
    }

    if (client->readable) {
        while (true) {
            PacketType packetType;

            int readCode = Packet_GetNextType(client->server, &packetType);

            if (readCode != NET_OK) {
                if (readCode != NET_ERROR) return;
                Client_Disconnect(client);
                std::cout << "Client: Failed to send data to server. Leaving...\n";
                break;
            }
            switch (packetType) {
                case PCK_NOTHING:
                    return;
                case PCK_ACCEPTED: {
                    AcceptedPacket packet;
                    Packet_RecvAccepted(client->server, &packet);

                    client->state = READY;
                    std::cout << "Connected to server" << std::endl;
                    break;
                }
                default:
                    std::cout << "Unknown package" << std::endl;
                    return;
            }
        }
    }
}

void Client_Disconnect(Net_Client* client) {
    Packet_SendDisconnect(client->server, DIS_LEFT);
    client->state = IDLE;
}
