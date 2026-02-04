#ifndef PACKETS_H
#define PACKETS_H
#include "util/net.h"
#include <cstdint>

struct PacketData;

enum class PacketType : uint8_t {
    PCK_NOTHING = 0,
    PCK_CONNECT = 1,
    PCK_ACCEPTED = 2,
    PCK_DISCONNECT = 3,
    PCK_MESSAGE = 4,
    PCK_PLAYERLIST_REQUEST = 5,
    PCK_PLAYERLIST_HEADER = 6,
    PCK_PLAYERLIST = 7
};

enum class DisconnectReason : uint8_t {
    DIS_LEFT = 0,
    DIS_KICK = 1,
    DIS_TIMEOUT = 2,
    DIS_CLOSE = 3
};

class Packet {
public:
    static Net::Result GetNextType(Socket socket, PacketType* out_pckType);

    static Net::Result Send(Socket socket, const char* buffer, int buffer_size);
    static Net::Result Receive(Socket socket, char* out_buffer, int buffer_capacity);

    static void Serialize(PacketType type, const PacketData* data, int size, char* out_buffer) ;
    static void Deserialize(const char* buffer, PacketData* out_packet, int size) ;
};


struct PacketData {
};

struct ConnectPacket : PacketData {
    char name[25]{};
};

struct MessagePacket : PacketData {
    char message[50]{};
};
struct AcceptedPacket : PacketData {
};
struct DisconnectedPacket : PacketData {
    DisconnectReason reason{};
};
struct PlayerListRequestPacket : PacketData {
};
struct PlayerListHeaderPacket : PacketData {
    uint8_t playerCount{};
};
struct PlayerListPacket : PacketData {
    char name[25]{};
    int id{};
};


#endif //PACKETS_H
