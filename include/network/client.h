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

    // State
    bool isConnected() const;
    NetState state() const;

    // Getter / Setter
    Socket getServer() const {
        return mServer;
    }
    NetState getState() const {
        return mState;
    }

private:
    void processNetwork();

    Net::Address mServerAddr;
    Socket mServer;

    bool mReadable = false;
    bool mWritable = false;

    NetState mState = NetState::IDLE;
};

#endif //CLIENT_H
