#include "network/packets.h"

#include <cstring>
#include <iostream>
#include <_bsd_types.h>

/**
 *
 * Get the next package type from a given socket
 *
 * @param socket
 * @param out_pckType
 * @return
 */
NetResult Packet_GetNextType(NetSocket socket, PacketType* out_pckType) {
    char buffer[1];
    NetResult readCode = Socket_Read(socket, buffer, 1);
    if (readCode != NET_OK) {
        return readCode;
    }

    PacketType type = static_cast<PacketType>(buffer[0]);
    if (out_pckType != nullptr) *out_pckType = type;
    return NET_OK;
}


// -------- Send Packets ---------
NetResult Packet_SendConnect(NetSocket socket, const char name[]) {
    char buffer[26]{};

    buffer[0] = PCK_CONNECT;

    std::memcpy(&buffer[1], name,std::min<size_t>(25, std::strlen(name)));

    return Socket_Send(socket, buffer, sizeof(buffer));
}

NetResult Packet_SendMessage(NetSocket socket, const char message[]) {
    char buffer[51]{};

    buffer[0] = PCK_MESSAGE;

    std::memcpy(&buffer[1], message,std::min<size_t>(50, std::strlen(message)));

    return Socket_Send(socket, buffer, sizeof(buffer));
}

NetResult Packet_SendAccepted(NetSocket socket) {
    char buffer[1]{};
    buffer[0] = PCK_ACCEPTED;
    return Socket_Send(socket, buffer, sizeof(buffer));
}

NetResult Packet_SendDisconnect(NetSocket socket, DisconnectReason reason) {
    char buffer[2]{};

    buffer[0] = PCK_DISCONNECT;
    buffer[1] = reason;

    return Socket_Send(socket, buffer, sizeof(buffer));
}


// -------- Recv Packets ---------

NetResult Packet_RecvConnect(NetSocket socket, ConnectPacket* out_packet) {
    char buffer[25]{};
    NetResult readCode = Socket_Read(socket, buffer, sizeof(buffer));
    if (readCode != NET_OK) {
        return readCode;
    }

    std::memcpy(out_packet->name, &buffer[0], 25);
    return NET_OK;
}

NetResult Packet_RecvMessage(NetSocket socket, MessagePacket* out_packet) {
    char buffer[50]{};
    NetResult readCode = Socket_Read(socket, buffer, sizeof(buffer));
    if (readCode != NET_OK) {
        return readCode;
    }

    std::memcpy(out_packet->message, &buffer[0], 25);
    return NET_OK;
}

NetResult Packet_RecvAccepted(NetSocket socket, AcceptedPacket* out_packet) {
    return NET_OK;
}

NetResult Packet_RecvDisconnect(NetSocket socket, DisconnectedPacket* out_packet) {
    char buffer[1]{};
    NetResult readCode = Socket_Read(socket, buffer, sizeof(buffer));
    if (readCode != NET_OK) {
        return readCode;
    }

    out_packet->reason = static_cast<DisconnectReason>(buffer[0]);
    return NET_OK;
}