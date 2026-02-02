#ifndef PACKETS_H
#define PACKETS_H
#include "util/net.h"
#include <cstdint>

enum PacketType : uint8_t {
    PCK_NOTHING = 0,
    PCK_CONNECT = 1,
    PCK_ACCEPTED = 2,
    PCK_DISCONNECT = 3,
    PCK_MESSAGE = 4
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

NetResult Packet_GetNextType(NetSocket socket, PacketType* out_pckType);

NetResult Packet_SendConnect(NetSocket socket, const char name[]);
NetResult Packet_SendMessage(NetSocket socket, const char message[]);
NetResult Packet_SendAccepted(NetSocket socket);
NetResult Packet_SendDisconnect(NetSocket socket, DisconnectReason reason);

NetResult Packet_RecvConnect(NetSocket socket, ConnectPacket* out_packet);
NetResult Packet_RecvMessage(NetSocket socket, MessagePacket* out_packet);
NetResult Packet_RecvAccepted(NetSocket socket, AcceptedPacket* out_packet);
NetResult Packet_RecvDisconnect(NetSocket socket, DisconnectedPacket* out_packet);
#endif //PACKETS_H
