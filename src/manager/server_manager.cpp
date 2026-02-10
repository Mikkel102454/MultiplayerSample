//
// Created by mikkel on 2/5/2026.
//

#include "manager/server_manager.h"

std::optional<Server> ServerManager::mServer = std::nullopt;

Server& ServerManager::create(const Net::Address& addr, int maxClients)
{
    if (!mServer.has_value()) {
        mServer.emplace(addr, maxClients);
    }

    return *mServer;
}


bool ServerManager::has()
{
    return mServer.has_value();
}


Server& ServerManager::get()
{
    return *mServer;
}


void ServerManager::stop()
{
    mServer.reset();
}