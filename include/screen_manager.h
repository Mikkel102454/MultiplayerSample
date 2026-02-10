#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H
#include <map>
#include <memory>
#include <string_view>

#include "ui/screen.h"

class ScreenManager {
public:
    ScreenManager();
    ~ScreenManager() = default;

    void draw();
    void update();
    void setScreen(std::string_view);
    
private:
    Screen* mCurrentScreen = nullptr;
    std::map<std::string, std::unique_ptr<Screen>> mScreens;

    void addScreen(std::string name, std::unique_ptr<Screen> screen);
};

#endif //SCREEN_MANAGER_H