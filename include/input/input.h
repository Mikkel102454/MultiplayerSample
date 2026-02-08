#ifndef INPUT_H
#define INPUT_H
#include <string>
#include <vector>
#include <map>
#include "raylib.h"
#include <memory>
#include "keybind.h"
#include "device.h"

class InputContext {
public:
    std::string mName;

    std::vector<Keybind> mBindings;
};

class ActionState {
public:
    bool mPressed;
    bool mPressedRepeat;
    bool mHeld;
    bool mUp;
    bool mReleased;
    float mValue;
};


class InputManager {

    std::map<std::string, InputContext> mContexts;

    InputContext* mActiveContext;

    std::map<std::string, ActionState> mActions;

    std::vector<std::unique_ptr<InputDevice>> mDevices;
public:

    bool isPressed(std::string_view action);
    bool isPressedRepeat(std::string_view action);
    bool isReleased(std::string_view action);
    bool isHeld(std::string_view action);
    bool isUp(std::string_view action);
    float getAxis(std::string_view action);
    void addContext(InputContext context);
    void addAction(std::string_view name);
    void addDevice(InputDevice* device);
    void setContext(std::string_view name);
    void process();
};

#endif //INPUT_H
