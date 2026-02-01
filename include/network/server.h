#ifndef SERVER_H
#define SERVER_H
#include "util/net.h"
#include <iostream>

struct Client {
    NetSocket sock;
    NetAddress addr;

    bool connected;
    bool accepted;
};

struct Server {
    std::unique_ptr<Client[]> clients;
    int client_count;
    int max_clients;

    NetSocket socket;
};

void Server_Init(Server* server, NetAddress addr, int max_clients);
void Server_AcceptClients(Server* server);
void Server_Destroy(Server* server);


#endif //SERVER_H
