#include "util/dev/console/console.h"

#include <cstring>

#include "raylib.h"
#include "util/dev/console/command/registry.h"
#include "util/dev/console/command/commands/core_command.h"

Console::ConsoleInstance *Console::console = nullptr;
CommandRegistry *Console::command_registry = nullptr;

void Console::RegisterCommands() {
    RegisterCoreCommands(*command_registry);
}

void Console::Init() {
    command_registry = new CommandRegistry();
    console = new ConsoleInstance();
    RegisterCommands();

    console->consoleFont = LoadFontEx(ASSETS_PATH "pixel_game/fonts/VictorMono-Medium.ttf", 14, nullptr, 0);
}

void Console::SetOpen(const bool open) {
    console->open = open;
}

bool Console::IsOpen() {
    return console->open;
}

void Console::Draw()
{
    const int padding = 8;
    const int fontSize = 14;
    const int lineHeight = fontSize + 2;
    const int height = 300;

    DrawRectangle(0, 0, GetScreenWidth(), height, (Color){ 0, 0, 0, 220 });
    const int inputBarHeight = lineHeight + padding * 2;

    DrawRectangle(0, height - inputBarHeight, GetScreenWidth(), inputBarHeight, (Color){ 35, 35, 35, 240 });

    DrawLine(0, height - inputBarHeight, GetScreenWidth(), height - inputBarHeight, (Color){ 60, 60, 60, 255 });

    const int logAreaHeight = height - inputBarHeight;
    int maxVisibleLines = (logAreaHeight - padding * 2) / lineHeight;

    // clamp scroll offset
    if (console->scroll_offset < 0)
        console->scroll_offset = 0;

    int maxScroll =
        console->log_count > maxVisibleLines
        ? console->log_count - maxVisibleLines
        : 0;

    if (console->scroll_offset > maxScroll)
        console->scroll_offset = maxScroll;

    if (maxVisibleLines > console->log_count)
        maxVisibleLines = console->log_count;

    int start = console->log_count - 1 - console->scroll_offset;

    // draw logs
    for (int i = 0; i < maxVisibleLines; i++)
    {
        int logIndex = start - i;
        if (logIndex < 0)
            break;

        Color color = RAYWHITE;
        switch (console->log[logIndex]->level)
        {
            case FATAL:   color = RED;    break;
            case WARNING: color = YELLOW; break;
            case INFO:    color = RAYWHITE; break;
            case SUCCESS: color = GREEN;  break;
        }

        DrawTextEx(
            console->consoleFont,
            console->log[logIndex]->text.c_str(),
            {
                static_cast<float>(padding),
                static_cast<float>(logAreaHeight - padding - lineHeight * (i + 1))
            },
            static_cast<float>(fontSize),
            1.0f,
            color
        );
    }

    // input draw thing
    const float inputY = static_cast<float>(height - padding - lineHeight);

    DrawTextEx(
        console->consoleFont,
        ">",
        { static_cast<float>(padding), inputY },
        static_cast<float>(fontSize),
        1.0f,
        RAYWHITE
    );

    DrawTextEx(
        console->consoleFont,
        console->input,
        { static_cast<float>(padding + 10), inputY },
        static_cast<float>(fontSize),
        1.0f,
        RAYWHITE
    );

    if (!console->cursor_can_blink || static_cast<int>(GetTime() * 2) % 2 == 0)
    {
        if (console->cursor_position < 0) console->cursor_position = 0;

        char temp[1024];
        strncpy(temp, console->input, console->cursor_position);
        temp[console->cursor_position] = '\0';

        Vector2 textSize = MeasureTextEx(
            console->consoleFont,
            temp,
            static_cast<float>(fontSize),
            1.0f
        );

        const float textStartX = static_cast<float>(padding + 11);
        const float cursorX = textStartX + textSize.x;

        const float cursorTopY = inputY;
        const float cursorBottomY = inputY + fontSize;

        DrawLine(
            static_cast<int>(cursorX),
            static_cast<int>(cursorTopY),
            static_cast<int>(cursorX),
            static_cast<int>(cursorBottomY),
            RAYWHITE
        );
    }
}

void Console::Log(const LogLevel level, const char* format, ...)
{
    char buffer[512];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (console->log_count >= CONSOLE_MAX_LOG)
    {
        delete console->log[0];
        for (int i = 1; i < console->log_count; i++)
        {
            console->log[i - 1] = console->log[i];
        }
        console->log_count--;
    }

    CommandLine* commandLine = new CommandLine();
    commandLine->level = level;
    commandLine->text = buffer;

    console->log[console->log_count++] = commandLine;
}

void Console::AutoComplete()
{
    size_t len = std::strlen(console->input);

    if (len == 0) return;

    size_t cursor = console->cursor_position;

    if (cursor > len) cursor = len;

    size_t wordStart = cursor;
    bool inQuotes = false;

    // Scan left
    for (size_t i = cursor; i > 0; i--)
    {
        if (console->input[i - 1] == '"')
            inQuotes = !inQuotes;

        if (!inQuotes && console->input[i - 1] == ' ')
        {
            wordStart = i;
            break;
        }

        wordStart = i - 1;
    }

    size_t wordEnd = cursor;
    inQuotes = false;

    // Scan right
    for (size_t i = cursor; i < len; i++)
    {
        if (console->input[i] == '"') inQuotes = !inQuotes;

        if (!inQuotes && console->input[i] == ' ')
        {
            wordEnd = i;
            break;
        }

        wordEnd = i + 1;
    }

    if (cursor != wordEnd) return;

    std::string_view prefix(console->input + wordStart, cursor - wordStart);

    std::string_view input(console->input, len);
    auto tokens = splitArgs(input);

    if (tokens.empty()) return;

    if (wordStart == 0)
    {
        for (const auto& [name, cmd] : command_registry->all())
        {
            if (name.starts_with(prefix))
            {
                std::string newInput;

                newInput += name;

                newInput.append(console->input + wordEnd, len - wordEnd);

                size_t max = sizeof(console->input) - 1;
                size_t copyLen = std::min(newInput.size(), max);

                std::memcpy(console->input, newInput.data(), copyLen);

                console->input[copyLen] = '\0';

                console->cursor_position = name.size();

                return;
            }
        }

        return;
    }

    std::string cmdName(tokens[0]);

    Command* cmd = command_registry->find(cmdName);
    if (!cmd)return;

    size_t argIndex = 0;

    {
        size_t count = 0;

        inQuotes = false;

        for (size_t i = 0; i < wordStart; i++)
        {
            if (console->input[i] == '"')
                inQuotes = !inQuotes;

            if (!inQuotes && console->input[i] == ' ')
                count++;
        }
        if (count == 0) return;

        argIndex = count - 1;
    }

    if (argIndex >= cmd->args.size()) return;

    const CommandArg& arg = cmd->args[argIndex];

    if (!arg.completer) return;

    auto suggestions = arg.completer(prefix);

    if (suggestions.empty()) return;

    const std::string& completion = suggestions[0];

    std::string newInput;

    newInput.append(console->input, wordStart);
    newInput += completion;
    newInput.append(console->input + wordEnd, len - wordEnd);

    size_t max = sizeof(console->input) - 1;
    size_t copyLen = std::min(newInput.size(), max);

    std::memcpy(console->input, newInput.data(), copyLen);

    console->input[copyLen] = '\0';

    console->cursor_position = wordStart + completion.size();
}

void Console::HandleInput()
{
    static double backspaceStartTime = 0.0;
    static double lastRepeatTime = 0.0;

    const double BACKSPACE_DELAY  = 0.30;
    const double BACKSPACE_REPEAT = 0.05;

    static double arrowStartTime = 0;
    static double lastArrowRepeat = 0;

    double now = GetTime();
    bool userEditing = false;

    // -------- Character input --------
    int key = GetCharPressed();
    while (key > 0)
    {
        int len = strlen(console->input);

        if (key >= 32 && key <= 126 && len < CONSOLE_MAX_INPUT - 1)
        {
            userEditing = true;
            int pos = console->cursor_position;

            memmove(&console->input[pos + 1], &console->input[pos], len - pos + 1);

            console->input[pos] = (char)key;
            console->cursor_position++;
        }

        key = GetCharPressed();
    }

    // -------- Backspace handling --------
    bool ctrlDown = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

    if (IsKeyPressed(KEY_BACKSPACE))
    {
        backspaceStartTime = GetTime();
        lastRepeatTime = backspaceStartTime;

        int len = strlen(console->input);
        int pos = console->cursor_position;

        if (pos > 0 && len > 0)
        {
            if (ctrlDown)
            {
                int start = pos;

                while (start > 0 && console->input[start - 1] == ' ')start--;
                while (start > 0 && console->input[start - 1] != ' ')start--;

                memmove(&console->input[start], &console->input[pos], len - pos + 1);

                console->cursor_position = start;
            }
            else
            {
                memmove(&console->input[pos - 1], &console->input[pos], len - pos + 1);

                console->cursor_position--;
            }
        }
    }
    else if (IsKeyDown(KEY_BACKSPACE))
    {
        if (now - backspaceStartTime >= BACKSPACE_DELAY &&
            now - lastRepeatTime >= BACKSPACE_REPEAT)
        {
            lastRepeatTime = now;

            int len = strlen(console->input);
            int pos = console->cursor_position;

            if (pos > 0 && len > 0)
            {
                if (ctrlDown)
                {
                    int start = pos;

                    bool inQuotes = false;

                    for (int i = start - 1; i >= 0; i--)
                    {
                        if (console->input[i] == '"') inQuotes = !inQuotes;

                        if (!inQuotes && console->input[i] == ' ')
                        {
                            start = i + 1;
                            break;
                        }

                        start = i;
                    }

                    memmove(&console->input[start], &console->input[pos], len - pos + 1);

                    console->cursor_position = start;
                }
                else
                {
                    memmove(&console->input[pos - 1], &console->input[pos], len - pos + 1);

                    console->cursor_position--;
                }
            }
        }
    }

    // -------- Enter --------
    if (IsKeyPressed(KEY_ENTER))
    {
        if (console->input[0] != '\0')
        {
            Log(INFO, TextFormat("> %s", console->input));
            ExecuteCommand();

            if (console->history_count >= CONSOLE_MAX_HISTORY) {
                for (int i = 1; i < console->history_count; i++) {
                    console->history[i - 1] = console->history[i];
                }
                console->history_count--;
            }

            console->history[console->history_count] = console->input;
            console->history_count++;

            console->input[0] = '\0';
            console->history_offset = 0;
        }
    }

    if (IsKeyPressed(KEY_UP))
    {
        if (console->history_count > 0 && console->history_offset < console->history_count)
        {
            console->history_offset++;

            int index = console->history_count - console->history_offset;
            strncpy(console->input, console->history[index].c_str(), sizeof(console->input) - 1);
            console->input[sizeof(console->input) - 1] = '\0';
            console->cursor_position = strlen(console->input);
        }
    }

    if (IsKeyPressed(KEY_DOWN))
    {
        if (console->history_offset > 1)
        {
            console->history_offset--;

            int index = console->history_count - console->history_offset;
            strncpy(console->input, console->history[index].c_str(), sizeof(console->input) - 1);
            console->input[sizeof(console->input) - 1] = '\0';
            console->cursor_position = strlen(console->input);
        }
        else
        {
            console->cursor_position = 0;
            console->history_offset = 0;
            console->input[0] = '\0';
        }
    }

    if (IsKeyPressed(KEY_LEFT))
    {
        arrowStartTime = GetTime();
        lastArrowRepeat = arrowStartTime;

        if (console->cursor_position > 0)
            console->cursor_position--;
    }
    else if (IsKeyDown(KEY_LEFT))
    {
        if (now - arrowStartTime >= BACKSPACE_DELAY &&
            now - lastArrowRepeat >= BACKSPACE_REPEAT)
        {
            lastArrowRepeat = now;

            if (console->cursor_position > 0)
                console->cursor_position--;
        }
    }

    // -------- Right Arrow --------
    if (IsKeyPressed(KEY_RIGHT))
    {
        arrowStartTime = GetTime();
        lastArrowRepeat = GetTime();

        int len = strlen(console->input);

        if (console->cursor_position < len)
            console->cursor_position++;
    }
    else if (IsKeyDown(KEY_RIGHT))
    {
        if (now - arrowStartTime >= BACKSPACE_DELAY &&
            now - lastArrowRepeat >= BACKSPACE_REPEAT)
        {
            lastArrowRepeat = now;

            int len = strlen(console->input);

            if (console->cursor_position < len)
                console->cursor_position++;
        }
    }

    bool arrowActive = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT);
    bool backspaceActive = IsKeyDown(KEY_BACKSPACE);

    if (!arrowActive && !backspaceActive && !userEditing) console->cursor_can_blink = true;
    else console->cursor_can_blink = false;

    // -------- Autocomplete --------
    if (IsKeyPressed(KEY_TAB))
    {
        AutoComplete();
    }

    if (GetMouseWheelMove() > 0) {
        console->scroll_offset++;
    } else if (GetMouseWheelMove() < 0) {
        console->scroll_offset--;
    }
}

void Console::Destroy() {
    for (int i = 0; i < console->log_count; i++) {
        delete console->log[i];
    }
    delete command_registry;
    delete console;
    console = nullptr;
}

void Console::ExecuteCommand() {

    size_t len = std::strlen(console->input);

    if (len == 0) return;

    console->cursor_position = 0;
    std::string_view input(console->input, len);

    size_t spacePos = input.find(' ');

    std::string_view commandName;
    std::string_view argString;

    if (spacePos != std::string_view::npos) {
        commandName = input.substr(0, spacePos);
        argString   = input.substr(spacePos + 1);
    } else {
        commandName = input;
        argString   = {};
    }

    Command* command = command_registry->find(std::string(commandName));

    if (!command) {
        Log(FATAL, "Unknown command. Type 'help' to see command list");
        return;
    }

    // tokenize args
    auto tokens = splitArgs(argString);

    ParsedArgs parsed;

    if (!parseArgs(*command, tokens, parsed)) {
        Log(FATAL, "Invalid arguments. Type 'help %s' for usage", std::string(commandName).c_str());
        return;
    }

    command->execute(parsed);
}

void Console::ClearLogs() {
    for (int i = 0; i < CONSOLE_MAX_LOG; i++) {
        delete console->log[i];
    }
    console->log_count = 0;
    console->scroll_offset = 0;
}

