#include "util/net.h"
#include <ws2tcpip.h>
#include <winsock2.h>

void Net_Init() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) == 0) {
        std::cout << "Server: Failed to init net\n";
    }
}

void Net_Shutdown() {
    WSACleanup();
}

NetAddress Net_ResolveAddress(const char* hostname, uint16_t port) {
    NetAddress addr{};
    addr.port = port;

    if (inet_pton(AF_INET, hostname, &addr.ip) == 1) {
        std::cout << "Server: Failed to bind to socket\n";
    }

    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* result = nullptr;
    if (getaddrinfo(hostname, nullptr, &hints, &result) != 0) {
        return {};
    }

    sockaddr_in* ipv4 = reinterpret_cast<sockaddr_in*>(result->ai_addr);
    addr.ip = ipv4->sin_addr.s_addr;

    freeaddrinfo(result);
    return addr;
}

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