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
Net::Result Packet::GetNextType(Socket socket, PacketType* out_pckType) {
    char buffer[1];
    Net::Result readCode = Socket::Read(socket, buffer, 1);
    if (readCode != Net::Result::NET_OK) {
        return readCode;
    }

    *out_pckType = static_cast<PacketType>(buffer[0]);
    return Net::Result::NET_OK;
}


// -------- Send Packets ---------
Net::Result Packet::Send(Socket socket, const char* buffer, int buffer_size) {
    return Socket::Send(socket, buffer, buffer_size);
}

void Packet::Serialize(PacketType type, const PacketData* data, int size, char* out_buffer) {
    out_buffer[0] = static_cast<char>(type);

    std::memcpy(&out_buffer[1], data, size);
}
// -------- Recv Packets ---------

Net::Result Packet::Receive(Socket socket, char* out_buffer, int buffer_capacity) {
    return Socket::Read(socket, out_buffer, buffer_capacity);
}

void Packet::Deserialize(const char* buffer, PacketData* out_packet, int size) {
    memcpy(out_packet, &buffer[0], size);
}