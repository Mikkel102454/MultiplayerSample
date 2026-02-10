//
// Created by mikkel on 2/5/2026.
//

#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H
#include <optional>

#include "network/server.h"


class ServerManager {
public:
    static Server& create(const Net::Address& addr, int maxClients);
    static bool has();
    static Server& get();
    static void stop();

private:
    static std::optional<Server> mServer;
};

#endif //SERVERMANAGER_H