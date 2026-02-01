#include "network/client.h"

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