#ifndef CONNECT_PACKET_H
#define CONNECT_PACKET_H
#include "player_join_packet.h"
#include "manager/console_manager.h"
#include "network/client.h"
#include "network/packets.h"
#include "network/server.h"

class PlayerJoinPacket;
class ConnectPacket final : public IPacket {
public:
    char name[25]{};
    int32_t id{};

    PacketType type() const override { return PacketType::PCK_CONNECT; }
    void serialize(std::vector<uint8_t>& outPayload) const override {
        outPayload.clear();
        outPayload.reserve(25 + 4);

        outPayload.insert(outPayload.end(),
                          reinterpret_cast<const uint8_t*>(name),
                          reinterpret_cast<const uint8_t*>(name) + 25);

        PacketCodec::write_i32_be(outPayload, id);
    }
    bool deserialize(const uint8_t* payload, size_t payloadSize) override {
        size_t off = 0;
        if (payloadSize != 25 + 4) return false;

        if (!PacketCodec::read_bytes(payload, payloadSize, off, name, 25)) return false;
        if (!PacketCodec::read_i32_be(payload, payloadSize, off, id)) return false;

        return true;
    }

    void handleClient(Client* client) const override {
        client->mId = id;
        client->mState = NetState::READY;
        ConsoleManager::get().log(SUCCESS, "Client: Connected to server");
    }
    void handleServer(Server* server, Server::Client* client) const override {
        if (id != -1) {
            ConsoleManager::get().log(INFO, "Server: Somebody tried to join the server with a set id");
            return;
        }

        ConsoleManager::get().log(INFO, "Server: New client named: %s", name);

        std::memcpy(client->name, &name[0], 25);
        client->connected = true;
        client->accepted = true;

        ConnectPacket response{};
        std::memcpy(response.name, name, 25);
        response.id = client->id;

        PacketIO::sendPacket(client->sock, response);

        // Tell new client about already-accepted clients
        for (int i = 0; i < static_cast<int>(server->mClients.size()); i++) {
            if (i == client->id) continue;
            if (!server->mClients[i].accepted) continue;

            PlayerJoinPacket playerPacket{};
            playerPacket.id = i;
            playerPacket.announce = 0;
            playerPacket.reason = DisconnectReason::DIS_LEFT;
            std::memcpy(playerPacket.name, server->mClients[i].name, 25);

            PacketIO::sendPacket(client->sock, playerPacket);
        }

        // Broadcast: new client joined
        PlayerJoinPacket joinPacket{};
        joinPacket.id = client->id;
        joinPacket.announce = 1;
        joinPacket.reason = DisconnectReason::DIS_LEFT;
        std::memcpy(joinPacket.name, name, 25);

        server->broadcastPacket(joinPacket, true);
    }
};
AUTO_REGISTER_PACKET(ConnectPacket, PacketType::PCK_CONNECT);

#endif //CONNECT_PACKET_H