#include "util/net.h"

#include <charconv>
#include <iostream>
#include <ws2tcpip.h>
#include <winsock2.h>

#include "network/packets.h"

void Net::Init() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cout << "Failed to init net\n";
    }
}

void Net::Shutdown() {
    WSACleanup();
}

bool Net::ParsePort(std::string_view str, uint16_t& out) {
    unsigned int temp;

    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), temp);

    if (ec != std::errc{} || ptr != str.data() + str.size())
        return false;

    if (temp > 65535)
        return false;

    out = static_cast<uint16_t>(temp);
    return true;
}

/**
 *
 * Resolves an address
 *
 * @param hostname
 * @param port
 * @return the NetAddress
 */
Net::Address Net::ResolveAddress(const char* hostname, uint16_t port) {
    Net::Address addr{};
    addr.port = port;

    int code = inet_pton(AF_INET, hostname, &addr.ip);
    if (code != 1 && code != 0) {
        std::cout << "inet_pton failed\n";
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

/**
 *
 * Creates a socket
 *
 * @param protocol
 * @param nonblocking
 * @return the NetSocket
 */
Socket Socket::Create(Net::Protocol protocol, bool nonblocking){
    Socket sock;
    SOCKET handle = socket(AF_INET,
                           protocol == Net::Protocol::NET_TCP ? SOCK_STREAM : SOCK_DGRAM,
                           protocol == Net::Protocol::NET_TCP ? IPPROTO_TCP : IPPROTO_UDP);

    if (handle == INVALID_SOCKET) {
        sock.handle = 0;
        return sock;
    }

    if (nonblocking) {
        u_long mode = 1;
        ioctlsocket(handle, FIONBIO, &mode);
    }

    sock.handle = handle;

    return sock;
}

/**
 *
 * Bind a given NetSocket to a specific address and port
 *
 * @param sock socket to bind
 * @param addr address to bind to
 * @return the NetResult
 */
Net::Result Socket::Bind(Socket sock, Net::Address addr){
    SOCKADDR_IN sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = addr.ip;
    sa.sin_port = addr.port;

    if (bind(sock.handle, reinterpret_cast<SOCKADDR*>(&sa), sizeof(sa)) == SOCKET_ERROR) {
        return Net::Result::NET_ERROR;
    }

    return Net::Result::NET_OK;
}

/**
 *
 * Connects socket to remote address
 *
 * @param sock socket to bind
 * @param addr address to bind to
 * @return the NetResult
 */
Net::Result Socket::Connect(Socket sock, Net::Address addr){
    SOCKADDR_IN sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = addr.ip;
    sa.sin_port = addr.port;

    if (connect(sock.handle, reinterpret_cast<SOCKADDR*>(&sa), sizeof(sa)) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) return Net::Result::NET_WOULDBLOCK;
        return Net::Result::NET_ERROR;
    }

    return Net::Result::NET_OK;
}

Net::Result Socket::Close(Socket sock) {
    return closesocket(sock.handle) == 0 ? Net::Result::NET_OK : Net::Result::NET_ERROR;
}

/**
 *
 * Tell socket to listen for incomming connections
 *
 * @param sock socket that should listen
 * @param backlog how many connections can be queued up
 * @return the NetResult
 */
Net::Result Socket::Listen(Socket sock, int backlog){
    return listen(sock.handle, backlog) == 0 ? Net::Result::NET_OK : Net::Result::NET_ERROR;
}

/**
 *
 * Accept a connection on a socket
 *
 * @param sock
 * @param out_socket client's socket
 * @param out_addr client's address
 * @return the NetResult
 */
Net::Result Socket::Accept(Socket sock, Socket* out_socket, Net::Address* out_addr){
    SOCKADDR_IN sa;
    int len = sizeof(sa);
    SOCKET client = accept(sock.handle, reinterpret_cast<SOCKADDR*>(&sa), &len);

    if(client == INVALID_SOCKET) {
        return Net::Result::NET_ERROR;
    }

    if(out_socket != nullptr) out_socket->handle = client;
    if(out_addr != nullptr) {
        out_addr->ip = sa.sin_addr.s_addr;
        out_addr->port = sa.sin_port;
    }

    return Net::Result::NET_OK;
}

/**
 *
 * Read data from a socket
 *
 * @param sock
 * @param buffer buffer to write to
 * @param length
 * @return the NetResult
 */
Net::Result Socket::Read(Socket sock, void* buffer, int length){
    int res = recv(sock.handle, static_cast<char*>(buffer), length, 0);

    if (res == 0) return Net::Result::NET_DISCONNECTED;
    if(res == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) return Net::Result::NET_WOULDBLOCK;
        return Net::Result::NET_ERROR;
    }

    return Net::Result::NET_OK;
}

/**
 *
 * Send data on a socket
 *
 * @param sock
 * @param data
 * @param length length of data
 * @return the NetResult
 */
Net::Result Socket::Send(Socket sock, const void* data, int length){
    if(send(sock.handle, static_cast<const char*>(data), length, 0) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) return Net::Result::NET_WOULDBLOCK;
        return Net::Result::NET_ERROR;
    }

    return Net::Result::NET_OK;
}

/**
 *
 * @param sockets
 * @param count
 * @param timeout_ms
 * @param readable
 * @param writable
 * @return
 */
Net::Result Socket::Poll(const Socket* sockets, int count, int timeout_ms, bool* readable, bool* writable) {
    fd_set readfds;
    fd_set writefds;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);

    SOCKET maxfd = 0;
    for (int i = 0; i < count; i++) {
        SOCKET s = sockets[i].handle;
        FD_SET(s, &readfds);
        FD_SET(s, &writefds);
        if (s > maxfd) maxfd = s;
    }

    TIMEVAL tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int res = select((int) maxfd + 1, &readfds, &writefds, NULL, &tv);
    if (res == SOCKET_ERROR) return Net::Result::NET_ERROR;

    for (int i = 0; i < count; i++) {
        SOCKET s = sockets[i].handle;
        readable[i] = FD_ISSET(s, &readfds);
        writable[i] = FD_ISSET(s, &writefds);
    }

    return Net::Result::NET_OK;
}