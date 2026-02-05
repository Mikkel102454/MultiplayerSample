#include "manager/ClientManager.h"

std::optional<Client> ClientManager::mClient = std::nullopt;

Client& ClientManager::create(const Net::Address& addr){
    if (!mClient.has_value()) {
        mClient.emplace(addr);
    }

    return *mClient;
}


bool ClientManager::has(){
    return mClient.has_value();
}


Client& ClientManager::get(){
    return *mClient;
}


void ClientManager::leave(){
    mClient.reset();
}
