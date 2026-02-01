#ifndef NET_H
#define NET_H

#include <stdbool.h>
#include <stdint.h>

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

NetSocket Socket_Create(NetProtocol protocol, bool nonblocking);
NetResult Socket_Bind(NetSocket sock, NetAddress addr);
NetResult Socket_Connect(NetSocket sock, NetAddress addr);
NetResult Socket_Listen(NetSocket sock, int backlog);
NetResult Socket_Accept(NetSocket sock, NetSocket* out_socket, NetAddress* out_addr);

#endif //NET_H
