#ifndef PACKETS_H
#define PACKETS_H

enum PacketType {
    PCK_NOTHING = -1,
    PCK_CONNECT = 0
};

struct ConnectPacket {
    int playerId;
    char name[25];
};

#endif //PACKETS_H
