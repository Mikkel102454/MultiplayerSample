//
// Created by mikkel on 2/4/2026.
//

#ifndef MULTIPLAYERSAMPLE_REGISTRY_H
#define MULTIPLAYERSAMPLE_REGISTRY_H
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <variant>

enum class ArgType {
    STRING,
    INT,
    FLOAT,
    BOOL,
    UINT16_T,
};

using ArgCompleter =
    std::function<std::vector<std::string>(std::string_view)>;

struct CommandArg {
    std::string name;
    ArgType type;
    bool optional;

    ArgCompleter completer;
};

using ArgValue = std::variant<
    std::string,
    int,
    float,
    bool,
    uint16_t
>;

struct ParsedArgs {
    std::unordered_map<std::string, ArgValue> values;
};

struct Command {
    std::string name;
    std::string description;

    std::vector<CommandArg> args;

    std::function<void(const ParsedArgs&)> execute;
};

class CommandRegistry {
public:
    bool registerCommand(Command cmd) {

        if (commands.contains(cmd.name)) return false; // already exists

        commands.emplace(cmd.name, std::move(cmd));
        return true;
    }

    Command* find(const std::string& name) {

        auto it = commands.find(name);

        if (it == commands.end()) return nullptr;

        return &it->second;
    }

    void remove(const std::string& name) {
        commands.erase(name);
    }

    const std::unordered_map<std::string, Command>& all() const {
        return commands;
    }

private:
    std::unordered_map<std::string, Command> commands;
};

bool parseArgs(const Command& cmd, const std::vector<std::string_view>& input, ParsedArgs& out);
bool parseOneArg(std::string_view text, ArgType type, ArgValue& out);
std::vector<std::string_view> splitArgs(std::string_view input);
const char* ArgTypeToString(ArgType t);

#endif //MULTIPLAYERSAMPLE_REGISTRY_H