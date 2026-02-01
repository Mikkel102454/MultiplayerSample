#ifndef SERVER_H
#define SERVER_H
#include "util/net.h"

struct Client {
    NetSocket sock;
    NetAddress addr;

    bool connected;
    bool accepted;
};

struct Server {
    Client*[] clients;
    int client_count;
    int max_clients;

    NetSocket socket;
};

void Server_Init(Server* server);
void Server_AcceptClients(Server* server);
void Server_Destroy(Server* server);


#endif //SERVER_H
