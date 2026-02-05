#include "manager/ConsoleManager.h"

std::optional<Console> ConsoleManager::mConsole = std::nullopt;

Console& ConsoleManager::create()
{
    if (!mConsole.has_value()) {
        mConsole.emplace();
    }

    return *mConsole;
}


bool ConsoleManager::has()
{
    return mConsole.has_value();
}


Console& ConsoleManager::get()
{
    return *mConsole;
}


void ConsoleManager::destroy()
{
    mConsole.reset();
}