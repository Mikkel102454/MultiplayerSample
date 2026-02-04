#ifndef SERVER_H
#define SERVER_H
#include "util/net.h"
#include <memory>
#include <cstdint>

enum class DisconnectReason : uint8_t;

class Server {
    struct Client {
        int id{};

        char name[25] {};

        Socket sock{};
        Net::Address addr{};

        bool connected{};
        bool accepted{};

        bool readable{};
        bool writable{};
    };

    static Server* server;
    Socket socket{};

    static void ProcessPackage(Client* client);
    static void AcceptClients();
    static void Sleep(double tickStartTimeMs);
    static void ProcessClients();

    public:
        std::unique_ptr<Client[]> clients = std::make_unique<Client[]>(max_clients);
        int client_count{};
        int max_clients{};

        uint64_t tick{};
        bool stopped{};
        static Server* Get();
        static bool Has();

        static void Init(Net::Address addr, int max_clients);
        static void Run();
        static void RemoveClient(int id, DisconnectReason reason);
        static void Destroy();
        static void Stop();
};



#endif //SERVER_H
