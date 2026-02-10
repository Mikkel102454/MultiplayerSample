#ifndef PACKETS_H
#define PACKETS_H
#include "util/net.h"
#include <cstdint>

struct PacketData;

enum class PacketType : uint8_t {
    PCK_NOTHING = 0,
    PCK_CONNECT = 1,
    PCK_JOIN = 2,
    PCK_DISCONNECT = 3,
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
    int id{};
};
struct PlayerJoinPacket : PacketData {
    char name[25]{};
    bool announce{};
    DisconnectReason reason{};
    int id{};
};
struct PlayerDisconnectPacket : PacketData {
    bool announce{};
    DisconnectReason reason{};
    int id{};
};

struct PlayerUpdatePacket : PacketData {
    int id{};
    int posX{};
    int posY{};
};


#endif //PACKETS_H
