#ifndef KEYBIND_H
#define KEYBIND_H
#include <string>
#include "input/device.h"

class Keybind {
public:
    std::string mAction;

    std::map<InputDevice::Type, int> mKeyCodes;

    float mScale = 1.0f;

    void addBind(InputDevice::Type type, int code);
};


#endif //KEYBIND_H
