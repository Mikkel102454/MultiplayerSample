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
 * @param outPckType
 * @return
 */
Net::Result Packet::getNextType(Socket socket, PacketType* outPckType) {
    char buffer[1];
    Net::Result readCode = Socket::read(socket, buffer, 1);
    if (readCode != Net::Result::NET_OK) {
        return readCode;
    }

    *outPckType = static_cast<PacketType>(buffer[0]);
    return Net::Result::NET_OK;
}


// Send Packets
Net::Result Packet::send(Socket socket, const char* buffer, int bufferSize) {
    return Socket::send(socket, buffer, bufferSize);
}

void Packet::serialize(PacketType type, const PacketData* data, int size, char* outBuffer) {
    outBuffer[0] = static_cast<char>(type);

    std::memcpy(&outBuffer[1], data, size);
}
// Recv Packets

Net::Result Packet::receive(Socket socket, char* outBuffer, int bufferCapacity) {
    return Socket::read(socket, outBuffer, bufferCapacity);
}

void Packet::deserialize(const char* buffer, PacketData* outPacket, int size) {
    memcpy(outPacket, &buffer[0], size);
}