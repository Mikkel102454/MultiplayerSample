#ifndef PLAYER_UPDATE_PACKET_H
#define PLAYER_UPDATE_PACKET_H
#include "network/packets.h"

class PlayerUpdatePacket final : public IPacket {
public:
    int32_t id{};
    int32_t posX{};
    int32_t posY{};

    PacketType type() const override { return PacketType::PCK_NOTHING; }
    void serialize(std::vector<uint8_t>& outPayload) const override {
        outPayload.clear();
        outPayload.reserve(12);
        PacketCodec::write_i32_be(outPayload, id);
        PacketCodec::write_i32_be(outPayload, posX);
        PacketCodec::write_i32_be(outPayload, posY);
    }
    bool deserialize(const uint8_t* payload, size_t payloadSize) override {
        size_t off = 0;
        if (payloadSize != 12) return false;
        if (!PacketCodec::read_i32_be(payload, payloadSize, off, id)) return false;
        if (!PacketCodec::read_i32_be(payload, payloadSize, off, posX)) return false;
        if (!PacketCodec::read_i32_be(payload, payloadSize, off, posY)) return false;
        return true;
    }

    void handleClient(Client* client) const override {

    }
    void handleServer(Server* server, Server::Client* client) const override {};
};

#endif //PLAYER_UPDATE_PACKET_H