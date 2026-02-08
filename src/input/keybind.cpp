#include "input/keybind.h"

void Keybind::addBind(InputDevice::Type type, int code) {
    mKeyCodes.emplace(type, code);
}
