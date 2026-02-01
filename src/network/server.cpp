#include "network/server.h"

void Server_Init(Server* server, NetAddress addr, int max_clients) {
    server->socket = Socket_Create(NET_TCP, true);
    if (Socket_Bind(server->socket, addr) != NET_OK) {
        std::cout << "Server: Failed to bind to socket\n";
        return;
    }

    if (Socket_Listen(server->socket, max_clients * 2) != NET_OK) {
        std::cout << "Server: Failed to listen on socket\n";
        return;
    }

    server->clients = std::make_unique<Client[]>(max_clients);
}

void Server_AcceptClients(Server* server) {
    for(int i = 0; i < server->max_clients; i++){
        if(!server->clients[i].connected) continue;

        NetSocket sock;
        NetAddress addr;
        if (Socket_Accept(server->socket, &sock, &addr) != NET_OK) {
            return;
        }

        server->clients[i].connected = true;
        server->clients[i].sock = sock;
        server->clients[i].addr = addr;

        server->client_count++;
    }
}

void Server_Destroy(Server* server) {
    //TODO stop server
}