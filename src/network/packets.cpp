#include "network/packets.h"

#include <cstring>

// -------------------- PacketRegistry implementation --------------------

static std::unordered_map<PacketType, PacketRegistry::Factory>& getRegistryMap() {
    static std::unordered_map<PacketType, PacketRegistry::Factory> map;
    return map;
}

void PacketRegistry::registerFactory(PacketType type, Factory factory) {
    getRegistryMap()[type] = std::move(factory);
}

std::unique_ptr<IPacket> PacketRegistry::create(PacketType type) {
    auto& map = getRegistryMap();
    auto it = map.find(type);
    if (it == map.end()) return nullptr;
    return (it->second)();
}

// -------------------- PacketIO raw --------------------

Net::Result PacketIO::send(Socket socket, const void* buffer, int bufferSize) {
    return Socket::send(socket, buffer, bufferSize);
}

Net::Result PacketIO::receive(Socket socket, void* outBuffer, int bufferCapacity) {
    return Socket::read(socket, outBuffer, bufferCapacity);
}

// -------------------- PacketIO framed --------------------

Net::Result PacketIO::sendPacket(Socket socket, const IPacket& packet) {
    std::vector<uint8_t> payload;
    payload.reserve(64);
    packet.serialize(payload);

    if (payload.size() > 0xFFFFu) {
        return Net::Result::NET_ERROR;
    }

    uint8_t header[3]{};
    header[0] = static_cast<uint8_t>(packet.type());

    // payloadLen u16 big-endian
    const uint16_t len = static_cast<uint16_t>(payload.size());
    header[1] = static_cast<uint8_t>((len >> 8) & 0xFF);
    header[2] = static_cast<uint8_t>(len & 0xFF);

    Net::Result res = PacketIO::send(socket, header, 3);
    if (res != Net::Result::NET_OK) return res;

    if (!payload.empty()) {
        res = PacketIO::send(socket, payload.data(), static_cast<int>(payload.size()));
        if (res != Net::Result::NET_OK) return res;
    }

    return Net::Result::NET_OK;
}

Net::Result PacketIO::receivePacket(Socket socket, std::unique_ptr<IPacket>& outPacket) {
    outPacket.reset();

    uint8_t header[3]{};
    Net::Result res = PacketIO::receive(socket, header, 3);
    if (res != Net::Result::NET_OK) return res;

    const PacketType type = static_cast<PacketType>(header[0]);
    const uint16_t payloadLen = (static_cast<uint16_t>(header[1]) << 8) |
                                (static_cast<uint16_t>(header[2]));

    std::vector<uint8_t> payload(payloadLen);
    if (payloadLen > 0) {
        res = PacketIO::receive(socket, payload.data(), payloadLen);
        if (res != Net::Result::NET_OK) return res;
    }

    std::unique_ptr<IPacket> pkt = PacketRegistry::create(type);

    if (!pkt) {
        // Unknown packet type: safely ignored because we had a length prefix.
        return Net::Result::NET_OK;
    }

    if (!pkt->deserialize(payload.data(), payload.size())) {
        return Net::Result::NET_ERROR;
    }

    outPacket = std::move(pkt);
    return Net::Result::NET_OK;
}


