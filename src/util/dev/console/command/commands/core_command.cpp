#include "network/client.h"
#include "network/packets.h"
#include "network/server.h"
#include "util/dev/console/dev.h"
#include "util/dev/console/command/auto_completion.h"
#include "util/dev/console/command/registry.h"

void RegisterCoreCommands(CommandRegistry& registry) {

    registry.registerCommand({
        "start_server",
        "Start a new server if no server is currently active",

        {
            {"ip", ArgType::STRING, false},
            {"port", ArgType::UINT16_T, false}
        },

        [](const ParsedArgs& args) {
            if (Server_Has()) {
                Console_Log(WARNING, "A server is already active. Please shutdown that server to make a new one");
                return;
            }

            std::string ip = std::get<std::string>(args.values.at("ip"));
            uint16_t port = std::get<uint16_t>(args.values.at("port"));

            Server* server = new Server();
            const NetAddress addressServer = Net_ResolveAddress(ip.c_str(), port);

            Server_Init(server, addressServer, 4);
             Server_Set(server);

             std::thread(Server_Run, server).detach();
        }
    });

    registry.registerCommand({
        "stop_server",
        "Stop a active server",

        {},

        [](const ParsedArgs& args) {
            if (!Server_Has()) {
                Console_Log(WARNING, "There is no active server");
                return;
            }

            Server_Get()->stopped = true;

            Console_Log(INFO, "Successfully shutdown server");
        }
    });

    registry.registerCommand({
        "join_server",
        "Join a active server",

        {
        {"ip", ArgType::STRING, false},
        {"port", ArgType::UINT16_T, false},
        {"name", ArgType::STRING, false}
        },

        [](const ParsedArgs& args) {
            if (Client_Has()) {
                Console_Log(WARNING, "You are already in a server. Please leave your current server before joining a new one");
                return;
            }

            std::string ip = std::get<std::string>(args.values.at("ip"));
            uint16_t port = std::get<uint16_t>(args.values.at("port"));

            Net_Client* client = new Net_Client();
            const NetAddress addressClient = Net_ResolveAddress(ip.c_str(), port);

            Client_Init(client, addressClient);
            Client_Connect(client);
            Client_Set(client);

            std::string name = std::get<std::string>(args.values.at("name"));

            ConnectPacket connectPacket{};
            memcpy(&connectPacket.name, name.c_str(), 25);
            char send_buffer[sizeof(ConnectPacket) + 1];

            Packet_Serialize(PCK_CONNECT, &connectPacket, sizeof(connectPacket), send_buffer);
            Packet_Send(client->server, send_buffer, sizeof(send_buffer));
        }
    });

    registry.registerCommand({
        "quit_server",
        "Leave the current server you are connected to",

        {},

        [](const ParsedArgs& args) {
            if (!Client_Has()) {
                Console_Log(WARNING, "You are not in a server. Please join a server before you can leave one");
                return;
            }

            Client_Destroy(Client_Get());

            Console_Log(SUCCESS, "Successfully left server");
        }
    });

    registry.registerCommand({
        "list",
        "List player that are connected to the server",

        {},

        [](const ParsedArgs& args) {
            if (!Client_Has()) {
                Console_Log(WARNING, "You are not in a server. Please join a server before you can see active players");
                return;
            }

            char send_buffer[sizeof(ConnectPacket) + 1];

            Packet_Send(Client_Get()->server, send_buffer, sizeof(send_buffer));
        }
    });

    registry.registerCommand({
        "help",
        "Show available commands",

        {
            {
                "command",
                ArgType::STRING,
                true,
                CompleteCommandNames
            }
        },

        [&registry](const ParsedArgs& args) {

            if (!args.values.contains("command")) {

                Console_Log(INFO, "Available commands:");

                for (const auto& [name, cmd] : registry.all()) {
                    Console_Log(INFO, "  %-12s - %s",
                        name.c_str(),
                        cmd.description.c_str()
                    );
                }

                return;
            }

            const std::string& target =
                std::get<std::string>(args.values.at("command"));

            Command* cmd = registry.find(target);

            if (!cmd) {
                Console_Log(FATAL, "Unknown command: %s", target.c_str());
                return;
            }

            std::string usage = target;

            for (const auto& arg : cmd->args) {

                if (arg.optional)
                    usage += " [" + arg.name + "]";
                else
                    usage += " <" + arg.name + ">";
            }

            Console_Log(INFO, "Usage: %s", usage.c_str());
            Console_Log(INFO, "Description: %s", cmd->description.c_str());

            if (!cmd->args.empty()) {

                Console_Log(INFO, "Arguments:");

                for (const auto& arg : cmd->args) {

                    Console_Log(INFO,
                        "  %s (%s)%s",
                        arg.name.c_str(),
                        ArgTypeToString(arg.type),
                        arg.optional ? " optional" : ""
                    );
                }
            }
        }
    });

    registry.registerCommand({
        "clear",
        "Clears the console",

        {},

        [](const ParsedArgs& args) {
            for (int i = 0; i < CONSOLE_MAX_LOG; i++) {
                delete Console_Get()->log[i];
            }
            Console_Get()->log_count = 0;
            Console_Get()->scroll_offset = 0;
        }
    });

    registry.registerCommand({
        "test",
        "test command",

        {
            {"string", ArgType::STRING, false,}
        },

        [](const ParsedArgs& args) {
            Console_Log(INFO, "Test command: %s", std::get<std::string>(args.values.at("string")).c_str());
        }
    });
}
