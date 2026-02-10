#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H
#include <optional>

#include "network/client.h"


class ClientManager {
    public:
    static Client& create(const Net::Address& addr);
    static bool has();
    static Client& get();
    static void leave();

    private:
    static std::optional<Client> mClient;
};
#endif //CLIENTMANAGER_H