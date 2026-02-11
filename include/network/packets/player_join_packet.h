#ifndef PLAYER_JOIN_PACKET_H
#define PLAYER_JOIN_PACKET_H
#include "manager/console_manager.h"
#include "network/client.h"
#include "network/packets.h"

class PlayerJoinPacket final : public IPacket {
public:
    char name[25]{};
    uint8_t announce{};
    DisconnectReason reason{};
    int32_t id{};

    PacketType type() const override { return PacketType::PCK_JOIN; }
    void serialize(std::vector<uint8_t>& outPayload) const override {
        outPayload.clear();
        outPayload.reserve(25 + 1 + 1 + 4);

        outPayload.insert(outPayload.end(),
                          reinterpret_cast<const uint8_t*>(name),
                          reinterpret_cast<const uint8_t*>(name) + 25);

        PacketCodec::write_u8(outPayload, announce);
        PacketCodec::write_u8(outPayload, static_cast<uint8_t>(reason));
        PacketCodec::write_i32_be(outPayload, id);
    }
    bool deserialize(const uint8_t* payload, size_t payloadSize) override {
        size_t off = 0;
        if (payloadSize != 25 + 1 + 1 + 4) return false;

        if (!PacketCodec::read_bytes(payload, payloadSize, off, name, 25)) return false;

        uint8_t r{};
        if (!PacketCodec::read_u8(payload, payloadSize, off, announce)) return false;
        if (!PacketCodec::read_u8(payload, payloadSize, off, r)) return false;
        reason = static_cast<DisconnectReason>(r);

        if (!PacketCodec::read_i32_be(payload, payloadSize, off, id)) return false;

        return true;
    }

    void handleClient(Client* client) const override {
        if (id == client->mId) return;

        ConsoleManager::get().log(SUCCESS, "Client: New client discovered with the name %s and id %d", name, id);

        if (announce) {

        }
    }
    void handleServer(Server* server, Server::Client* client) const override {};
};
AUTO_REGISTER_PACKET(PlayerJoinPacket, PacketType::PCK_JOIN);

#endif //PLAYER_JOIN_PACKET_H