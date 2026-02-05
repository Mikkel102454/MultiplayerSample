#ifndef SERVER_H
#define SERVER_H
#include <atomic>

#include "util/net.h"
#include <memory>
#include <cstdint>
#include <vector>

enum class DisconnectReason : uint8_t;

class Server {
public:
    explicit Server(const Net::Address& address, int maxClients);
    ~Server();

    void run();
    void stop();

    void removeClient(int id, DisconnectReason reason);


    // Status
    bool isRunning() const;

private:
    struct Client {
        int id = -1;

        char name[25] {};

        Socket sock{};
        Net::Address addr{};

        bool connected = false;
        bool accepted = false;

        bool readable = false;
        bool writable = false;
    };

    void processPackage(Client* client);
    void acceptClients();
    void sleep(double tickStartTimeMs);
    void processClients();

    Socket mSocket{};
    int mMaxClients{};

    std::vector<Client> mClients;

    uint64_t mTick{};
    std::atomic<bool> mRunning{false};
};



#endif //SERVER_H
