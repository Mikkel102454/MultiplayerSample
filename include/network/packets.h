#ifndef PACKETS_H
#define PACKETS_H
#include "util/net.h"
#include <cstdint>

enum PacketType : uint8_t {
    PCK_NOTHING = 0,
    PCK_CONNECT = 1,
    PCK_ACCEPTED = 2,
    PCK_DISCONNECT = 3,
    PCK_MESSAGE = 4,
    PCK_PLAYERLIST_REQUEST = 5,
    PCK_PLAYERLIST_HEADER = 6,
    PCK_PLAYERLIST = 7
};

enum DisconnectReason : uint8_t {
    DIS_LEFT = 0,
    DIS_KICK = 1,
    DIS_TIMEOUT = 2,
    DIS_CLOSE = 3
};

struct ConnectPacket {
    char name[25]{};
};

struct MessagePacket {
    char message[50]{};
};
struct AcceptedPacket {
};
struct DisconnectedPacket {
    DisconnectReason reason{};
};
struct PlayerListRequestPacket {
};
struct PlayerListHeaderPacket {
    uint8_t playerCount{};
};
struct PlayerListPacket {
    char name[25]{};
    int id{};
};

NetResult Packet_GetNextType(NetSocket socket, PacketType* out_pckType);

NetResult Packet_Send(NetSocket socket, const char* buffer);
NetResult Packet_Recv(NetSocket socket, char* out_buffer);

void Packet_Serialize(uint8_t type, const void* data, uint16_t size, char* out_buffer);
void Packet_Deserialize(char* buffer, void* out_packet, uint16_t size);
#endif //PACKETS_H
