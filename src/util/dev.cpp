#include "util/dev.h"

#include <cstring>

#include "raylib.h"
#include "util/dev/commands.h"

Console* consoleRef;
Font consoleFont;



void Console_RegisterCommands(Console* console) {
    console->commands.push_back(std::make_unique<ConsoleCommandHelp>());
    console->command_count++;

    console->commands.push_back(std::make_unique<ConsoleCommandStartServer>());
    console->command_count++;

    console->commands.push_back(std::make_unique<ConsoleCommandStopServer>());
    console->command_count++;

    console->commands.push_back(std::make_unique<ConsoleCommandJoinServer>());
    console->command_count++;

    console->commands.push_back(std::make_unique<ConsoleCommandQuitServer>());
    console->command_count++;

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


void Console_AutoComplete(Console* console) {
    int len = strlen(console->input);
    if (len == 0) return;

    for (int i = 0; i < console->command_count; i++) {
        auto args = console->commands[i]->getArgs();
        std::string_view cmd = args[0];

        if (cmd.starts_with(console->input))
        {
            strncpy(console->input, cmd.data(), sizeof(console->input) - 1);
            console->input[sizeof(console->input) - 1] = '\0';
            break;
        }
    }
}

void Console_HandleInput(Console* console)
{
    static double backspaceStartTime = 0.0;
    static double lastRepeatTime = 0.0;

    const double BACKSPACE_DELAY  = 0.30; // seconds before repeat
    const double BACKSPACE_REPEAT = 0.05; // seconds per delete

    // -------- Character input --------
    int key = GetCharPressed();
    while (key > 0)
    {
        int len = strlen(console->input);

        if (key >= 32 && key <= 126 && len < CONSOLE_MAX_INPUT - 1)
        {
            console->input[len] = (char)key;
            console->input[len + 1] = '\0';
        }

        key = GetCharPressed();
    }

    // -------- Backspace handling --------
    if (IsKeyPressed(KEY_BACKSPACE))
    {
        backspaceStartTime = GetTime();
        lastRepeatTime = backspaceStartTime;

        int len = strlen(console->input);
        if (len > 0) {
            // Ctrl + Backspace
            if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
            {
                while (len > 0 && console->input[len - 1] == ' ')
                    console->input[--len] = '\0';

                while (len > 0 && console->input[len - 1] != ' ')
                    console->input[--len] = '\0';
            }
            else
            {
                console->input[len - 1] = '\0';
            }
        }
    }
    else if (IsKeyDown(KEY_BACKSPACE))
    {
        double now = GetTime();

        if (now - backspaceStartTime >= BACKSPACE_DELAY &&
            now - lastRepeatTime >= BACKSPACE_REPEAT)
        {
            lastRepeatTime = now;

            int len = strlen(console->input);
            if (len > 0)
            {
                // Ctrl + Backspace
                if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
                {
                    while (len > 0 && console->input[len - 1] == ' ')
                        console->input[--len] = '\0';

                    while (len > 0 && console->input[len - 1] != ' ')
                        console->input[--len] = '\0';
                }
                else
                {
                    console->input[len - 1] = '\0';
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
        if (console->history_count > 0 &&
            console->history_offset < console->history_count)
        {
            console->history_offset++;

            int index = console->history_count - console->history_offset;
            strncpy(
                console->input,
                console->history[index].c_str(),
                sizeof(console->input) - 1
            );
            console->input[sizeof(console->input) - 1] = '\0';
        }
    }

    if (IsKeyPressed(KEY_DOWN))
    {
        if (console->history_offset > 1)
        {
            console->history_offset--;

            int index = console->history_count - console->history_offset;
            strncpy(
                console->input,
                console->history[index].c_str(),
                sizeof(console->input) - 1
            );
            console->input[sizeof(console->input) - 1] = '\0';
        }
        else
        {
            console->history_offset = 0;
            console->input[0] = '\0';
        }
    }

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

void Console_ExecuteCommand(Console* console)
{
    int len = strlen(console->input);
    if (len == 0)
        return;

    const char* input = console->input;
    const char* space = strchr(input, ' ');

    std::string_view commandName;
    if (space)
        commandName = std::string_view(input, space - input);
    else
        commandName = std::string_view(input);

    for (int i = 0; i < console->command_count; i++)
    {
        auto args = console->commands[i]->getArgs();
        std::string_view cmd = args[0];

        if (cmd == commandName)
        {
            console->commands[i]->execute(console->input);
            return;
        }
    }
    Console_Log(FATAL, "Unknown command. type 'help' to see command list");
}

