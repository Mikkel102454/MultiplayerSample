#ifndef CLIENT_H
#define CLIENT_H
#include "util/net.h"

enum class NetState {
    IDLE = 0,
    CONNECTING = 1,
    READY = 2
};

class Client {
public:
    // Construct a fully usable client
    explicit Client(const Net::Address& serverAddr);
    ~Client();

    // Control
    void connect();
    void disconnect();
    void update();

    // Getter / Setter
    Socket getServer() const {
        return mServer;
    }

    int mId{};

    NetState mState = NetState::IDLE;
private:
    void processNetwork();

    Net::Address mServerAddr;
    Socket mServer;

    bool mReadable = false;
    bool mWritable = false;


};

#endif //CLIENT_H
