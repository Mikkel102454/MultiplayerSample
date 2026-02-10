#ifndef DEVICE_H
#define DEVICE_H
#include <string>
#include <vector>
#include <map>
#include "raylib.h"
#include <memory>
#include "keybind.h"

class InputDevice {
public:
    virtual ~InputDevice() = default;

    enum class Type : uint8_t {
        KEYBOARD,
        CONTROLLER,
        MOUSE,
    };

    virtual Type getType() = 0;
    virtual bool getButtonPressed(int code) = 0;
    virtual bool getButtonReleased(int code) = 0;
    virtual bool getButtonPressedRepeat(int code) = 0;
    virtual bool getButtonDown(int code) = 0;
    virtual bool getButtonUp(int code) = 0;

    virtual float getAxis(int code) = 0;


};

class KeyboardDevice : public InputDevice {
public:
    Type getType() override {
        return InputDevice::Type::KEYBOARD;
    }
    bool getButtonPressed(int code) override {
        return IsKeyPressed(code);
    }

    bool getButtonReleased(int code) override {
        return IsKeyReleased(code);
    }

    bool getButtonDown(int code) override {
        return IsKeyDown(code);
    }

    bool getButtonPressedRepeat(int code) override {
        return IsKeyPressedRepeat(code);
    }

    bool getButtonUp(int code) override {
        return IsKeyUp(code);
    }

    float getAxis(int code) override {
        return 0.0f;
    }
};

class MouseDevice : public InputDevice {
public:
    Type getType() override {
        return InputDevice::Type::MOUSE;
    }
    bool getButtonPressed(int code) override {
        return IsMouseButtonPressed(code);
    }

    bool getButtonReleased(int code) override {
        return IsMouseButtonReleased(code);
    }

    bool getButtonDown(int code) override {
        return IsMouseButtonDown(code);
    }

    bool getButtonPressedRepeat(int code) override {
        return false;
    }

    bool getButtonUp(int code) override {
        return IsMouseButtonUp(code);
    }

    float getAxis(int code) override {
        return 0.0f;
    }
};

class ControllerDevice : public InputDevice {
public:
    int mGamepad = 0;
    Type getType() override {
        return InputDevice::Type::CONTROLLER;
    }
    bool getButtonPressed(int code) override {
        return IsGamepadButtonPressed(mGamepad, code);
    }

    bool getButtonReleased(int code) override {
        return IsGamepadButtonReleased(mGamepad, code);
    }

    bool getButtonDown(int code) override {
        return IsGamepadButtonDown(mGamepad, code);
    }

    bool getButtonPressedRepeat(int code) override {
        return false;
    }

    bool getButtonUp(int code) override {
        return IsGamepadButtonUp(mGamepad, code);
    }

    float getAxis(int code) override {
        return GetGamepadAxisMovement(mGamepad, code);
    }
};

#endif //DEVICE_H
