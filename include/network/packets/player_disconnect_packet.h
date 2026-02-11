#ifndef PLAYER_DISCONNECT_PACKET_H
#define PLAYER_DISCONNECT_PACKET_H
#include "manager/client_manager.h"
#include "manager/console_manager.h"
#include "network/client.h"
#include "network/packets.h"

class PlayerDisconnectPacket final : public IPacket {
public:
    uint8_t announce{};
    DisconnectReason reason{};
    int32_t id{};

    PacketType type() const override { return PacketType::PCK_DISCONNECT; }
    void serialize(std::vector<uint8_t>& outPayload) const override {
        outPayload.clear();
        outPayload.reserve(1 + 1 + 4);

        PacketCodec::write_u8(outPayload, announce);
        PacketCodec::write_u8(outPayload, static_cast<uint8_t>(reason));
        PacketCodec::write_i32_be(outPayload, id);
    }
    bool deserialize(const uint8_t* payload, size_t payloadSize) override {
        size_t off = 0;
        if (payloadSize != 1 + 1 + 4) return false;

        uint8_t r{};
        if (!PacketCodec::read_u8(payload, payloadSize, off, announce)) return false;
        if (!PacketCodec::read_u8(payload, payloadSize, off, r)) return false;
        reason = static_cast<DisconnectReason>(r);

        if (!PacketCodec::read_i32_be(payload, payloadSize, off, id)) return false;
        return true;
    }

    void handleClient(Client* client) const override {
        if (id == -1) {
            // get outa here
            ClientManager::leave();
            return;
        }

        // somebody else left
        ConsoleManager::get().log(SUCCESS, "Client: Player %d left the game", id);

        if (announce) {

        }
    }
    void handleServer(Server* server, Server::Client* client) const override {
        ConsoleManager::get().log(INFO, "Server: Client left the game: %d", reason);
        server->removeClient(client->id, DisconnectReason::DIS_LEFT);
    }
};
AUTO_REGISTER_PACKET(PlayerDisconnectPacket, PacketType::PCK_DISCONNECT);

#endif //PLAYER_DISCONNECT_PACKET_H