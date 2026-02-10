//
// Created by mikkel on 2/5/2026.
//

#ifndef CONSOLEMANAGER_H
#define CONSOLEMANAGER_H
#include <optional>

#include "util/dev/console/console.h"


class ConsoleManager {
public:
    static Console& create();
    static bool has();
    static Console& get();
    static void destroy();

private:
    static std::optional<Console> mConsole;
};

#endif //CONSOLEMANAGER_H