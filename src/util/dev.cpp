#include "util/dev.h"

#include <cstring>

#include "raylib.h"
#include "util/dev/command/commands/core_command.h"

Console* consoleRef;
Font consoleFont;



void Console_RegisterCommands(Console* console) {
    RegisterCoreCommands(console->command_registry);
}

void Console_Init(Console* console) {
    Console_RegisterCommands(console);
    consoleRef = console;
    consoleFont = LoadFontEx(ASSETS_PATH "pixel_game/fonts/VictorMono-Medium.ttf", 14, nullptr, 0);
}

Console* Console_Get() {
    return consoleRef;
}

void Console_SetOpen(Console* console, bool open) {
    console->open = open;
}

bool Console_IsOpen(Console* console) {
    return console->open;
}

void Console_Draw(Console* console)
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
            consoleFont,
            console->log[logIndex]->text.c_str(),
            {
                (float)padding,
                (float)(logAreaHeight - padding - lineHeight * (i + 1))
            },
            (float)fontSize,
            1.0f,
            color
        );
    }

    // input draw thing
    const float inputY = (float)(height - padding - lineHeight);

    DrawTextEx(
        consoleFont,
        ">",
        { (float)padding, inputY },
        (float)fontSize,
        1.0f,
        RAYWHITE
    );

    DrawTextEx(
        consoleFont,
        console->input,
        { (float)(padding + 10), inputY },
        (float)fontSize,
        1.0f,
        RAYWHITE
    );

    if (!console->cursor_can_blink || static_cast<int>(GetTime() * 2) % 2 == 0)
    {
        int len = strlen(console->input);
        int cursorPos = len - (len - console->cursor_position);
        if (cursorPos < 0) cursorPos = 0;

        char temp[1024];
        strncpy(temp, console->input, cursorPos);
        temp[cursorPos] = '\0';

        Vector2 textSize = MeasureTextEx(
            consoleFont,
            temp,
            static_cast<float>(fontSize),
            1.0f
        );

        float textStartX = static_cast<float>(padding + 11);
        float cursorX = textStartX + textSize.x;

        float cursorTopY = inputY;
        float cursorBottomY = inputY + fontSize;

        DrawLine(
            static_cast<int>(cursorX),
            static_cast<int>(cursorTopY),
            static_cast<int>(cursorX),
            static_cast<int>(cursorBottomY),
            RAYWHITE
        );
    }
}

void Console_Log(LogLevel level, const char* format, ...)
{
    char buffer[512];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (consoleRef->log_count >= CONSOLE_MAX_LOG)
    {
        delete consoleRef->log[0];
        for (int i = 1; i < consoleRef->log_count; i++)
        {
            consoleRef->log[i - 1] = consoleRef->log[i];
        }
        consoleRef->log_count--;
    }

    CommandLine* commandLine = new CommandLine();
    commandLine->level = level;
    commandLine->text = buffer;

    consoleRef->log[consoleRef->log_count++] = commandLine;
}

void Console_AutoComplete(Console* console)
{
    size_t len = std::strlen(console->input);

    if (len == 0) return;

    size_t cursor = console->cursor_position;

    if (cursor > len) cursor = len;

    size_t wordStart = cursor;
    while (wordStart > 0 && console->input[wordStart - 1] != ' ') wordStart--;

    size_t wordEnd = cursor;
    while (wordEnd < len && console->input[wordEnd] != ' ') wordEnd++;

    if (cursor != wordEnd) return;

    std::string_view prefix(console->input + wordStart, cursor - wordStart);

    std::string_view input(console->input, len);
    auto tokens = splitArgs(input);

    if (tokens.empty()) return;

    if (wordStart == 0)
    {
        for (const auto& [name, cmd] : console->command_registry.all())
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

    Command* cmd = console->command_registry.find(cmdName);
    if (!cmd)return;

    size_t argIndex = 0;

    {
        size_t count = 0;

        for (size_t i = 0; i < wordStart; i++)
        {
            if (console->input[i] == ' ') count++;
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

void Console_HandleInput(Console* console)
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

                    while (start > 0 && console->input[start - 1] == ' ') start--;
                    while (start > 0 && console->input[start - 1] != ' ') start--;

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
            Console_Log(INFO, TextFormat("> %s", console->input));
            Console_ExecuteCommand(console);

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
        Console_AutoComplete(console);
    }
}

void Console_Destroy(Console* console) {
    for (int i = 0; i < console->log_count; i++) {
        delete console->log[i];
    }
    delete console;
    consoleRef = nullptr;
}

void Console_ExecuteCommand(Console* console) {

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

    Command* command =console->command_registry.find(std::string(commandName));

    if (!command) {
        Console_Log(FATAL, "Unknown command. Type 'help' to see command list");
        return;
    }

    // tokenize args
    auto tokens = splitArgs(argString);

    ParsedArgs parsed;

    if (!parseArgs(*command, tokens, parsed)) {
        Console_Log(FATAL, "Invalid arguments. Type 'help %s' for usage", std::string(commandName).c_str());
        return;
    }

    command->execute(parsed);
}

