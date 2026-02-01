#ifndef CLIENT_H
#define CLIENT_H
#include "util/net.h"
#include <iostream>

enum Net_State {
    IDLE = 0,
    CONNECTING = 1,
    ACCEPTING =2,
    READY = 3
};

struct Net_Client {
    NetAddress addr;
    NetSocket server;

    Net_State state;
};

void Client_Connect(Net_Client* client, NetAddress addr);
void Client_Destroy(Net_Client* client, NetAddress addr);

#endif //CLIENT_H
