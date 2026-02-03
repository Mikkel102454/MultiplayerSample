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

    void execute(std::string_view command) override;
};


#endif //MULTIPLAYERSAMPLE_COMMANDS_H