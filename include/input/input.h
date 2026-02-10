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
    static InputManager mInstance;

    std::map<std::string, InputContext> mContexts;

    InputContext* mActiveContext = nullptr;

    std::map<std::string, ActionState> mActions;

    std::vector<std::unique_ptr<InputDevice>> mDevices;
public:
    static InputManager* get() {
        static InputManager instance;
        return &instance;
    }

    bool isPressed(std::string_view action);
    bool isPressedRepeat(std::string_view action);
    bool isReleased(std::string_view action);
    bool isHeld(std::string_view action);
    bool isUp(std::string_view action);
    float getAxis(std::string_view action);
    void addContext(InputContext context);
    void addAction(std::string_view name);
    void addDevice(std::unique_ptr<InputDevice> devicePtr);
    void setContext(std::string_view name);
    void process();

    void init(std::string_view path);
    void load(std::string_view path);
};

#endif //INPUT_H
