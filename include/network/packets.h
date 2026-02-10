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
    static Net::Result getNextType(Socket socket, PacketType* outPckType);

    static Net::Result send(Socket socket, const char* buffer, int bufferSize);
    static Net::Result receive(Socket socket, char* outBuffer, int bufferCapacity);

    static void serialize(PacketType type, const PacketData* data, int size, char* outBuffer) ;
    static void deserialize(const char* buffer, PacketData* outPacket, int size) ;
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

struct PlayerUpdatePacket : PacketData {
    int id{};
    int posX{};
    int posY{};
};


#endif //PACKETS_H
