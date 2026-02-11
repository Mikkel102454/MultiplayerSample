#ifndef PACKETS_H
#define PACKETS_H

#include "util/net.h"
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "server.h"

class Client;
class Server;

enum class PacketType : uint8_t {
    PCK_NOTHING    = 0,
    PCK_CONNECT    = 1,
    PCK_JOIN       = 2,
    PCK_DISCONNECT = 3,
};

enum class DisconnectReason : uint8_t {
    DIS_LEFT    = 0,
    DIS_KICK    = 1,
    DIS_TIMEOUT = 2,
    DIS_CLOSE   = 3
};


namespace PacketCodec {
    inline void write_u8(std::vector<uint8_t>& out, uint8_t v) {
        out.push_back(v);
    }

    inline void write_i32_be(std::vector<uint8_t>& out, int32_t v) {
        // Manual BE (no winsock include needed in headers)
        uint32_t u = static_cast<uint32_t>(v);
        out.push_back(static_cast<uint8_t>((u >> 24) & 0xFF));
        out.push_back(static_cast<uint8_t>((u >> 16) & 0xFF));
        out.push_back(static_cast<uint8_t>((u >> 8) & 0xFF));
        out.push_back(static_cast<uint8_t>(u & 0xFF));
    }

    inline bool read_u8(const uint8_t* data, size_t size, size_t& off, uint8_t& out) {
        if (off + 1 > size) return false;
        out = data[off++];
        return true;
    }

    inline bool read_i32_be(const uint8_t* data, size_t size, size_t& off, int32_t& out) {
        if (off + 4 > size) return false;
        uint32_t u = (static_cast<uint32_t>(data[off + 0]) << 24) |
                     (static_cast<uint32_t>(data[off + 1]) << 16) |
                     (static_cast<uint32_t>(data[off + 2]) << 8)  |
                     (static_cast<uint32_t>(data[off + 3]));
        off += 4;
        out = static_cast<int32_t>(u);
        return true;
    }

    inline bool read_bytes(const uint8_t* data, size_t size, size_t& off, void* out, size_t n) {
        if (off + n > size) return false;
        std::memcpy(out, data + off, n);
        off += n;
        return true;
    }
} // namespace PacketCodec

// packet interface
class IPacket {
public:
    virtual ~IPacket() = default;

    virtual PacketType type() const = 0;

    // payload only (no framing)
    virtual void serialize(std::vector<uint8_t>& outPayload) const = 0;
    virtual bool deserialize(const uint8_t* payload, size_t payloadSize) = 0;

    virtual void handleClient(Client* client) const {}
    virtual void handleServer(Server* server, Server::Client* client) const {}
};

// packet registry
class PacketRegistry {
public:
    using Factory = std::function<std::unique_ptr<IPacket>()>;

    static void registerFactory(PacketType type, Factory factory);
    static std::unique_ptr<IPacket> create(PacketType type);
};

#define AUTO_REGISTER_PACKET(PacketClass, PacketTypeValue)                          \
    namespace PacketAutoReg {                                                       \
        inline const bool PacketClass##_registered = []() {                         \
            PacketRegistry::registerFactory(                                        \
                PacketTypeValue,                                                    \
                []() -> std::unique_ptr<IPacket> { return std::make_unique<PacketClass>(); } \
            );                                                                      \
            return true;                                                            \
        }();                                                                        \
    }

// -------------------- IO (framed packets) --------------------
class PacketIO {
public:
    static Net::Result send(Socket socket, const void* buffer, int bufferSize);
    static Net::Result receive(Socket socket, void* outBuffer, int bufferCapacity);

    // Framing: | type:u8 | payloadLen:u16 BE | payload... |
    static Net::Result sendPacket(Socket socket, const IPacket& packet);
    static Net::Result receivePacket(Socket socket, std::unique_ptr<IPacket>& outPacket);
};

#endif //PACKETS_H
