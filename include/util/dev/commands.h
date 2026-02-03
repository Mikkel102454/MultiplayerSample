//
// Created by mikkel on 2/3/2026.
//

#ifndef MULTIPLAYERSAMPLE_COMMANDS_H
#define MULTIPLAYERSAMPLE_COMMANDS_H

#include "util/dev.h"

//Commands
class ConsoleCommandStartServer : public ConsoleCommand {
public:
    std::span<const std::string_view> getArgs() override {
        static constexpr std::string_view args[] = {
            "start_server",
            "ip",
            "port"
        };
        return args;
    }
    std::string_view getDescription() override {
        return std::string_view {"Start a new server if no server is currently active"};
    }

    void execute(std::string_view command) override;
};
class ConsoleCommandStopServer : public ConsoleCommand {
public:
    std::span<const std::string_view> getArgs() override {
        static constexpr std::string_view args[] = {
            "stop_server"
        };
        return args;
    }

    std::string_view getDescription() override {
        return std::string_view {"Stop a active server"};
    }

    void execute(std::string_view command) override;
};
class ConsoleCommandJoinServer : public ConsoleCommand {
public:
    std::span<const std::string_view> getArgs() override {
        static constexpr std::string_view args[] = {
            "join_server",
            "ip",
            "port",
            "name"
        };
        return args;
    }

    std::string_view getDescription() override {
        return std::string_view {"Join a active server"};
    }

    void execute(std::string_view command) override;
};
class ConsoleCommandQuitServer : public ConsoleCommand {
public:
    std::span<const std::string_view> getArgs() override {
        static constexpr std::string_view args[] = {
            "quit_server"
        };
        return args;
    }

    std::string_view getDescription() override {
        return std::string_view {"Leave the current server you are connected to"};
    }

    void execute(std::string_view command) override;
};

class ConsoleCommandHelp : public ConsoleCommand {
public:
    std::span<const std::string_view> getArgs() override {
        static constexpr std::string_view args[] = {
            "help"
        };
        return args;
    }

    std::string_view getDescription() override {
        return std::string_view {"Get information about available commands"};
    }

    void execute(std::string_view command) override;
};


#endif //MULTIPLAYERSAMPLE_COMMANDS_H