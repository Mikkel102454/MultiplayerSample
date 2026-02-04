#ifndef SERVER_H
#define SERVER_H
#include "util/net.h"
#include <memory>
#include <cstdint>

enum DisconnectReason : uint8_t;

struct Client {
    int id{};

    char name[25] {};

    NetSocket sock{};
    NetAddress addr{};

    bool connected{};
    bool accepted{};

    bool readable{};
    bool writable{};
};

struct Server {
    std::unique_ptr<Client[]> clients = std::make_unique<Client[]>(max_clients);
    int client_count{};
    int max_clients{};

    uint64_t tick{};

    NetSocket socket{};

    bool stopped{};
};
Server* Server_Get();
void Server_Set(Server* server);
bool Server_Has();

void Server_Init(Server* server, NetAddress addr, int max_clients);
void Server_ProcessPackages(Server* server);
void Server_Run(Server* server);

void Server_AcceptClients(Server* server);
void Server_RemoveClient(Server* server, int id, DisconnectReason reason);
void Server_Sleep(Server* server, double tickStartTimeMs);
void Server_ProcessClients(Server* server);

void Server_Destroy(Server* server);


#endif //SERVER_H
