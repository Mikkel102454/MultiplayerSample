//
// Created by mikkel on 2/4/2026.
//

#include "util/dev/console/command/registry.h"

#include "../../../../../include/util/dev/console/dev.h"

const char* ArgTypeToString(ArgType t) {

    switch (t) {
        case ArgType::STRING: return "string";
        case ArgType::INT:    return "int";
        case ArgType::FLOAT:  return "float";
        case ArgType::BOOL:   return "bool";
        case ArgType::UINT16_T:   return "uint16_t";
    }

    return "unknown";
}

std::vector<std::string_view> splitArgs(std::string_view input) {
    std::vector<std::string_view> result;

    size_t i = 0;

    while (i < input.size()) {

        while (i < input.size() && input[i] == ' ')
            ++i;

        if (i >= input.size())
            break;

        if (input[i] == '"') {

            size_t start = ++i;

            while (i < input.size() && input[i] != '"')
                ++i;

            result.emplace_back(
                input.substr(start, i - start)
            );

            if (i < input.size())
                ++i;
        }

        else {

            size_t start = i;

            while (i < input.size() && input[i] != ' ')
                ++i;

            result.emplace_back(
                input.substr(start, i - start)
            );
        }
    }

    return result;
}

bool parseOneArg(std::string_view text, ArgType type, ArgValue& out) {
    try {

        switch (type) {

            case ArgType::STRING:
                if (text.empty()) return false;
                out = std::string(text);
                return true;

            case ArgType::INT:
                out = std::stoi(std::string(text));
                return true;

            case ArgType::FLOAT:
                out = std::stof(std::string(text));
                return true;

            case ArgType::BOOL:

                if (text == "true" || text == "1") {
                    out = true;
                    return true;
                }

                if (text == "false" || text == "0") {
                    out = false;
                    return true;
                }

                return false;
            case ArgType::UINT16_T: {
                int value = std::stoi(std::string(text));

                if (value < 0 || value > 65535)
                    return false;

                out = static_cast<uint16_t>(value);
                return true;
            }

        }

    } catch (...) {
        return false;
    }

    return false;
}

bool parseArgs(const Command& cmd, const std::vector<std::string_view>& input, ParsedArgs& out) {
    size_t required = 0;

    for (const auto& arg : cmd.args)
        if (!arg.optional)
            ++required;

    // too few args
    if (input.size() < required) {
        return false;
    }

    // too many args
    if (input.size() > cmd.args.size()) {
        return false;
    }

    for (size_t i = 0; i < input.size(); ++i) {
        const CommandArg& spec = cmd.args[i];

        ArgValue value;

        if (!parseOneArg(input[i], spec.type, value)) {
            Console_Log(FATAL, "Argument '%s' has invalid type", spec.name.c_str());
            return false;
        }

        out.values.emplace(spec.name, std::move(value));
    }

    return true;
}
