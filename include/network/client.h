#ifndef CLIENT_H
#define CLIENT_H
#include "util/net.h"

enum Net_State {
    IDLE = 0,
    CONNECTING = 1,
    READY = 2
};

struct Net_Client {
    NetAddress addr{};
    NetSocket server{};

    bool readable{};
    bool writeable{};

    Net_State state{};
};

Net_Client* Client_Get();
void Client_Set(Net_Client* client);
bool Client_Has();

void Client_Init(Net_Client* client, NetAddress addr);
void Client_Connect(Net_Client* client);
void Client_Disconnect(Net_Client* client);
void Client_Destroy(Net_Client* client);

void Client_Update(Net_Client* client);

#endif //CLIENT_H
