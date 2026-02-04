#ifndef MULTIPLAYERSAMPLE_DEV_H
#define MULTIPLAYERSAMPLE_DEV_H
#include <span>
#include <string_view>
#include <vector>

#include "raylib.h"
#include "util/dev/console/command/registry.h"

#define CONSOLE_MAX_LOG 1000
#define CONSOLE_MAX_INPUT 256
#define CONSOLE_MAX_HISTORY 18

class ConsoleCommand;

enum LogLevel {
    FATAL,
    WARNING,
    INFO,
    SUCCESS
};

class Console {
    struct CommandLine {
        LogLevel level;
        std::string text;
    };

    struct ConsoleInstance {
        CommandLine* log[CONSOLE_MAX_LOG] {};
        char input[CONSOLE_MAX_INPUT] {};
        std::string history[CONSOLE_MAX_HISTORY] {};
        int history_count = 0;
        int history_offset = 0;
        int log_count = 0;
        int scroll_offset = 0;
        int cursor_position = 0;
        bool open = false;
        bool cursor_can_blink = true;



        Font consoleFont;
    };
    static CommandRegistry* command_registry;

    static ConsoleInstance* console;

    static void ExecuteCommand();

    static void AutoComplete();
public:
    static void Init();

    static void SetOpen(bool open);
    static bool IsOpen();

    static void Draw();
    static void Log(LogLevel level, const char* format, ...);
    static void HandleInput();

    static void Destroy();

    static void RegisterCommands();
    static void ClearLogs();

    static CommandRegistry* GetCommandRegistry() {
        return command_registry;
    }
};


class ConsoleCommand {
public:
    virtual ~ConsoleCommand() = default;
    virtual std::span<const std::string_view> getArgs() = 0;
    virtual std::string_view getDescription() = 0;
    virtual void execute(std::string_view command) = 0;

protected:
    static std::vector<std::string_view> extractArgs(std::string_view command) {
        std::vector<std::string_view> args;

        size_t pos = 0;
        while (true) {
            size_t next = command.find(' ', pos);

            if (next == std::string_view::npos) {
                args.emplace_back(command.substr(pos));
                break;
            }

            args.emplace_back(command.substr(pos, next - pos));
            pos = next + 1;
        }

        return args;
    }
};

#endif //MULTIPLAYERSAMPLE_DEV_H