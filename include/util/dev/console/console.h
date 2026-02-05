#ifndef MULTIPLAYERSAMPLE_DEV_H
#define MULTIPLAYERSAMPLE_DEV_H
#include <deque>
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
public:
    // Construct ready-to-use console
    explicit Console();
    ~Console();

    // Core
    void draw();
    void handleInput();
    void log(LogLevel level, const char* format, ...);

    void clearLogs();

    //Getter / Setter
    void setOpen(bool open);
    bool isOpen() const;

    CommandRegistry* getRegistry() {
        return &mRegistry;
    }

private:
    struct CommandLine {
        LogLevel level{};
        std::string text;
    };

    void executeCommand();
    void autoComplete();

    // Dependencies
    CommandRegistry mRegistry{};

    // Logs
    std::deque<CommandLine> mLogs;

    // Input
    std::string mInput;
    std::vector<std::string> mHistory;

    int mHistoryOffset = 0;
    int mScrollOffset = 0;
    int mCursorPos = 0;

    bool mOpen = false;
    bool mCursorBlink = true;

    Font mFont;
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