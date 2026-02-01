#include "util/net.h"
#include <ws2tcpip.h>
#include <winsock2.h>

NetSocket Socket_Create(NetProtocol protocol, bool nonblocking){
    NetSocket sock;
    SOCKET handle = socket(AF_INET,
                           protocol == NET_TCP ? SOCK_STREAM : SOCK_DGRAM,
                           protocol == NET_TCP ? IPPROTO_TCP : IPPROTO_UDP);

    if (handle == INVALID_SOCKET) {
        sock.handle = 0;
        return sock;
    }

    if (nonblocking) {
        u_long mode = 1;
        ioctlsocket(handle, FIONBIO, &mode);
    }

    sock.handle = (uintptr_t) handle;

    return sock;
}

NetResult Socket_Bind(NetSocket sock, NetAddress addr){
    SOCKADDR_IN sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = addr.ip;
    sa.sin_port = addr.port;

    if (bind((SOCKET) sock.handle, (SOCKADDR*) &sa, sizeof(sa)) == SOCKET_ERROR) {
        return NET_ERROR;
    }

    return NET_OK;
}

NetResult Socket_Connect(NetSocket sock, NetAddress addr){
    SOCKADDR_IN sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = addr.ip;
    sa.sin_port = addr.port;

    if (connect((SOCKET) sock.handle, (SOCKADDR*) &sa, sizeof(sa)) == SOCKET_ERROR) {
        return NET_ERROR;
    }

    return NET_OK;
}

NetResult Socket_Listen(NetSocket sock, int backlog){
    return listen((SOCKET) sock.handle, backlog) == 0 ? NET_OK : NET_ERROR;
}

NetResult Socket_Accept(NetSocket sock, NetSocket* out_socket, NetAddress* out_addr){
    SOCKADDR_IN sa;
    int len = sizeof(sa);
    SOCKET client = accept((SOCKET) sock.handle, (SOCKADDR*) &sa, &len);

    if(client == INVALID_SOCKET) {
        return NET_ERROR;
    }

    if(out_socket != nullptr) out_socket->handle = (uintptr_t) client;
    if(out_addr != nullptr) {
        out_addr->ip = sa.sin_addr.s_addr;
        out_addr->port = sa.sin_port;
    }

    return NET_OK;
}