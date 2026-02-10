#include <thread>

#include "manager/client_manager.h"
#include "manager/console_manager.h"
#include "manager/server_manager.h"
#include "network/client.h"
#include "network/packets.h"
#include "network/server.h"
#include "util/dev/console/console.h"
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
            if (ServerManager::has()) {
                ConsoleManager::get().log(WARNING, "A server is already active. Please shutdown that server to make a new one");
                return;
            }

            std::string ip = std::get<std::string>(args.values.at("ip"));
            uint16_t port = std::get<uint16_t>(args.values.at("port"));

            const Net::Address addressServer = Net::resolveAddress(ip.c_str(), port);

            ServerManager::create(addressServer, 4);
            ServerManager::get().run();
        }
    });

    registry.registerCommand({
        "stop_server",
        "Stop a active server",

        {},

        [](const ParsedArgs& args) {
            if (!ServerManager::has()) {
                ConsoleManager::get().log(WARNING, "There is no active server");
                return;
            }

            ServerManager::stop();

            ConsoleManager::get().log(INFO, "Successfully shutdown server");
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
            if (ClientManager::has()) {
                ConsoleManager::get().log(WARNING, "You are already in a server. Please leave your current server before joining a new one");
                return;
            }

            std::string ip = std::get<std::string>(args.values.at("ip"));
            uint16_t port = std::get<uint16_t>(args.values.at("port"));

            const Net::Address addressClient = Net::resolveAddress(ip.c_str(), port);

            ClientManager::create(addressClient);
            ClientManager::get().connect();

            std::string name = std::get<std::string>(args.values.at("name"));

            ConnectPacket connectPacket{};
            memcpy(&connectPacket.name, name.c_str(), 25);
            char sendBuffer[sizeof(ConnectPacket) + 1]{};

            Packet::serialize(PacketType::PCK_CONNECT, &connectPacket, sizeof(connectPacket), sendBuffer);
            Packet::send(ClientManager::get().getServer(), sendBuffer, sizeof(sendBuffer));
        }
    });

    registry.registerCommand({
        "quit_server",
        "Leave the current server you are connected to",

        {},

        [](const ParsedArgs& args) {
            if (!ClientManager::has()) {
                ConsoleManager::get().log(WARNING, "You are not in a server. Please join a server before you can leave one");
                return;
            }

            ClientManager::leave();
        }
    });

    registry.registerCommand({
        "list",
        "List player that are connected to the server",

        {},

        [](const ParsedArgs& args) {
            if (!ClientManager::has()) {
                ConsoleManager::get().log(WARNING, "You are not in a server. Please join a server before you can see active players");
                return;
            }

            char sendBuffer[sizeof(PlayerListRequestPacket) + 1];

            Packet::serialize(PacketType::PCK_PLAYERLIST_REQUEST, nullptr, 0, sendBuffer);
            Packet::send(ClientManager::get().getServer(), sendBuffer, sizeof(sendBuffer));
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

                ConsoleManager::get().log(INFO, "Available commands:");

                for (const auto& [name, cmd] : registry.all()) {
                    ConsoleManager::get().log(INFO, "  %-12s - %s",
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
                ConsoleManager::get().log(FATAL, "Unknown command: %s", target.c_str());
                return;
            }

            std::string usage = target;

            for (const auto& arg : cmd->args) {

                if (arg.optional)
                    usage += " [" + arg.name + "]";
                else
                    usage += " <" + arg.name + ">";
            }

            ConsoleManager::get().log(INFO, "Usage: %s", usage.c_str());
            ConsoleManager::get().log(INFO, "Description: %s", cmd->description.c_str());

            if (!cmd->args.empty()) {

                ConsoleManager::get().log(INFO, "Arguments:");

                for (const auto& arg : cmd->args) {

                    ConsoleManager::get().log(INFO,
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
            ConsoleManager::get().clearLogs();
        }
    });

    registry.registerCommand({
        "test",
        "test command",

        {
            {"string", ArgType::STRING, false,}
        },

        [](const ParsedArgs& args) {
            ConsoleManager::get().log(INFO, "Test command: %s", std::get<std::string>(args.values.at("string")).c_str());
        }
    });

    registry.registerCommand({
    "reload",
    "Reload a specific feature",

    {
        {
            "name",
            ArgType::STRING,
            false,
            CompleteCommandNames
        }
    },

    [](const ParsedArgs& args) {
        std::string name = std::get<std::string>(args.values.at("name"));
    }
});
}
