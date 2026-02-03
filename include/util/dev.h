#ifndef MULTIPLAYERSAMPLE_DEV_H
#define MULTIPLAYERSAMPLE_DEV_H
#include <memory>
#include <span>
#include <string_view>
#include <thread>
#include <vector>

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

struct CommandLine {
    LogLevel level;
    std::string text;
};

struct Console {
    CommandLine* log[CONSOLE_MAX_LOG] {};
    char input[CONSOLE_MAX_INPUT] {};
    std::string history[CONSOLE_MAX_HISTORY] {};
    int history_count = 0;
    int history_offset = 0;
    int log_count = 0;
    int scroll_offset = 0;
    bool open = false;

    int command_count = 0;

    std::vector<std::unique_ptr<ConsoleCommand>> commands;
};

class ConsoleCommand {
public:
    virtual ~ConsoleCommand() = default;
    virtual std::span<const std::string_view> getArgs() = 0;
    virtual std::string_view getDescription() = 0;
    virtual void execute(std::string_view command) = 0;

protected:
    std::vector<std::string_view> extractArgs(std::string_view command) {
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


Console* Console_Get();
void Console_Init(Console* console);

void Console_SetOpen(Console* console, bool open);
bool Console_IsOpen(Console* console);

void Console_Draw(Console* console);
void Console_Log(LogLevel level, const char* format, ...);
void Console_ExecuteCommand(Console* console);
void Console_HandleInput(Console* console);
void Console_Destroy(Console* console);

#endif //MULTIPLAYERSAMPLE_DEV_H