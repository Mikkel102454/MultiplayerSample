#ifndef NET_H
#define NET_H

#include <cstdint>

enum NetResult {
    NET_OK = 0,
    NET_ERROR = -1,
    NET_WOULDBLOCK = -2,
    NET_DISCONNECTED = -3,
};

enum NetProtocol {
    NET_TCP,
    NET_UDP,
};
struct NetAddress {
    uint32_t ip;
    uint16_t port;
};

struct NetSocket {
    uintptr_t handle;
};

void Net_Init();
void Net_Shutdown();

NetAddress Net_ResolveAddress(const char* hostname, uint16_t port);

NetSocket Socket_Create(NetProtocol protocol, bool nonblocking);
NetResult Socket_Bind(NetSocket sock, NetAddress addr);
NetResult Socket_Connect(NetSocket sock, NetAddress addr);
NetResult Socket_Listen(NetSocket sock, int backlog);
NetResult Socket_Close(NetSocket sock);
NetResult Socket_Accept(NetSocket sock, NetSocket* out_socket, NetAddress* out_addr);
NetResult Socket_Read(NetSocket sock, void* buffer, int length);
NetResult Socket_Send(NetSocket sock, const void* data, int length);
NetResult Socket_Poll(NetSocket* sockets, int count, int timeout_ms, bool* readable, bool* writable);
#endif //NET_H
