//
// Created by mikkel on 2/4/2026.
//

#include "util/dev/command/auto_completion.h"

#include "util/dev.h"

std::vector<std::string> CompleteCommandNames(std::string_view prefix) {
    std::vector<std::string> out;

    auto& registry = Console_Get()->command_registry;

    for (const auto& [name, cmd] : registry.all()) {
        if (name.starts_with(prefix)) out.push_back(name);
    }

    return out;
}
