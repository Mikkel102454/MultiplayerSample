#include "network/client.h"

void Client_Connect(Net_Client* client, NetAddress addr) {
    client->server = Socket_Create(NET_TCP, true);
    client->addr = addr;

    if(Socket_Connect(client->server, addr) == NET_ERROR) {
        //TraceLog(LOG_FATAL, "Server: Failed connect socket to server");
        return;
    }
}