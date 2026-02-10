#include "screen_manager.h"

#include "manager/console_manager.h"
#include "ui/screen/main_menu.h"
#include "ui/screen/credits.h"
#include "util/dev/console/console.h"

ScreenManager::ScreenManager() {
    auto navigate = [this](std::string_view name) {
        this->setScreen(name);
    };

    addScreen("main_menu", std::make_unique<MainMenuScreen>(navigate));
    addScreen("credits", std::make_unique<CreditsScreen>(navigate));

    setScreen("main_menu");
}

void ScreenManager::draw() {
    if (mCurrentScreen == nullptr) {
        ConsoleManager::get().log(FATAL, "Tried to draw screen with no active screen");
        return;
    }
    mCurrentScreen->draw();
}

void ScreenManager::update() {
    if (mCurrentScreen == nullptr) {
        ConsoleManager::get().log(FATAL, "Tried to update screen with no active screen");
        return;
    }
    mCurrentScreen->update();
}

void ScreenManager::addScreen(std::string name, std::unique_ptr<Screen> screen) {
    mScreens.emplace(std::move(name), std::move(screen));
}

void ScreenManager::setScreen(std::string_view name) {
    const auto it = mScreens.find(std::string(name));
    if (it == mScreens.end()) {
        ConsoleManager::get().log(FATAL, "Tried to set unknown screen: %s", std::string(name).c_str());
        return;
    }
    mCurrentScreen = it->second.get();
}
