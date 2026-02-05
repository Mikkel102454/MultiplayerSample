#ifndef NET_H
#define NET_H

#include <cstdint>
#include <string_view>

class Net {
public:
    enum class Result {
        NET_OK = 0,
        NET_ERROR = -1,
        NET_WOULDBLOCK = -2,
        NET_DISCONNECTED = -3,
    };

    enum class Protocol {
        NET_TCP,
        NET_UDP,
    };

    struct Address {
        uint32_t ip;
        uint16_t port;
    };

    static void init();
    static void shutdown();
    static bool parsePort(std::string_view str, uint16_t& out);

    static Address resolveAddress(const char* hostname, uint16_t port);
};

class Socket {
public:
    uintptr_t handle{};

    static Socket create(Net::Protocol protocol, bool nonblocking);
    static Net::Result bind(Socket sock, Net::Address addr);
    static Net::Result connect(Socket sock, Net::Address addr);
    static Net::Result listen(Socket sock, int backlog);
    static Net::Result close(Socket sock);
    static Net::Result accept(Socket sock, Socket* outSocket, Net::Address* outAddr);
    static Net::Result read(Socket sock, void* buffer, int length);
    static Net::Result send(Socket sock, const void* data, int length);
    static Net::Result poll(const Socket* sockets, int count, int timeoutMs, bool* readable, bool* writable);
};
#endif //NET_H
