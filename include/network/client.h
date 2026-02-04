#ifndef CLIENT_H
#define CLIENT_H
#include "util/net.h"

enum class Net_State {
    IDLE = 0,
    CONNECTING = 1,
    READY = 2
};





class Client {
    struct Net_Client {
        Net::Address addr{};
        Socket server{};

        bool readable{};
        bool writeable{};

        Net_State state{};
    };

    static Net_Client* client;

    public:
        static Net_Client* Get();
        static bool Has();
        static void Init(Net::Address addr);
        static void Connect();
        static void Destroy();
        static void Update();
};


#endif //CLIENT_H
