#ifndef REGISTRY_H
#define REGISTRY_H
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

        if (mCommands.contains(cmd.name)) return false; // already exists

        mCommands.emplace(cmd.name, std::move(cmd));
        return true;
    }

    Command* find(const std::string& name) {

        auto it = mCommands.find(name);

        if (it == mCommands.end()) return nullptr;

        return &it->second;
    }

    void remove(const std::string& name) {
        mCommands.erase(name);
    }

    const std::unordered_map<std::string, Command>& all() const {
        return mCommands;
    }

private:
    std::unordered_map<std::string, Command> mCommands;
};

bool ParseArgs(const Command& cmd, const std::vector<std::string>& input, ParsedArgs& out);
bool ParseOneArg(std::string_view text, ArgType type, ArgValue& out);
std::vector<std::string> SplitArgs(std::string_view input);
const char* ArgTypeToString(ArgType t);

#endif //MULTIPLAYERSAMPLE_REGISTRY_H