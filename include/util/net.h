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

    static void Init();
    static void Shutdown();
    static bool ParsePort(std::string_view str, uint16_t& out);

    static Address ResolveAddress(const char* hostname, uint16_t port);
};

class Socket {
public:
    uintptr_t handle{};

    static Socket Create(Net::Protocol protocol, bool nonblocking);
    static Net::Result Bind(Socket sock, Net::Address addr);
    static Net::Result Connect(Socket sock, Net::Address addr);
    static Net::Result Listen(Socket sock, int backlog);
    static Net::Result Close(Socket sock);
    static Net::Result Accept(Socket sock, Socket* out_socket, Net::Address* out_addr);
    static Net::Result Read(Socket sock, void* buffer, int length);
    static Net::Result Send(Socket sock, const void* data, int length);
    static Net::Result Poll(const Socket* sockets, int count, int timeout_ms, bool* readable, bool* writable);
};
#endif //NET_H
