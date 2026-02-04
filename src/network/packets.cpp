#include "network/packets.h"

#include <cstring>
#include <iostream>
#include <winsock2.h>
#include <_bsd_types.h>
#include <bits/fs_fwd.h>

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
NetResult Packet_Send(NetSocket socket, const char* buffer) {
    return Socket_Send(socket, buffer, sizeof(buffer));
}

void Packet_Serialize(uint8_t type, const void* data, uint16_t size, char* out_buffer) {
    out_buffer[0] = static_cast<char>(type);

    std::memcpy(&out_buffer[1], data, size);
}
// -------- Recv Packets ---------

NetResult Packet_Recv(NetSocket socket, char* out_buffer) {
    return Socket_Read(socket, out_buffer, sizeof(out_buffer));
}

void Packet_Deserialize(char* buffer, void* out_packet, uint16_t size) {
    memcpy(out_packet, &buffer[0], size);
}