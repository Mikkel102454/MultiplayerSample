#include <nlohmann/json.hpp>

#include "input/input.h"

#include <fstream>

#include "input/device.h"
#include "manager/ConsoleManager.h"
bool InputManager::isPressed(std::string_view action) {
    const ActionState* act = nullptr;

    auto it = mActions.find(action.data());

    if (it != mActions.end())
    {
        act = &it->second;
    }
    if(act == nullptr){
        ConsoleManager::get().log(WARNING, "Tried to check if action is pressed on a non existing action: %s", action);
        return false;
    }
    return act->mPressed;
}

bool InputManager::isPressedRepeat(std::string_view action){
    const ActionState* act = nullptr;

    auto it = mActions.find(action.data ());

    if (it != mActions.end())
    {
        act = &it->second;
    }
    if(act == nullptr){
        ConsoleManager::get().log(WARNING, "Tried to check if action is pressed on a non existing action: %s", action);
        return false;
    }
    return act->mPressedRepeat;
}

bool InputManager::isReleased(std::string_view action) {
    const ActionState* act = nullptr;

    auto it = mActions.find(action.data());

    if (it != mActions.end())
    {
        act = &it->second;
    }
    if(act == nullptr){
        ConsoleManager::get().log(WARNING, "Tried to check if action is released on a non existing action: %s", action);
        return false;
    }
    return act->mReleased;
}

bool InputManager::isHeld(std::string_view action) {
    const ActionState* act = nullptr;

    auto it = mActions.find(action.data());

    if (it != mActions.end())
    {
        act = &it->second;
    }
    if(act == nullptr){
        ConsoleManager::get().log(WARNING, "Tried to check if action is held on a non existing action: %s", action);
        return false;
    }
    return act->mHeld;
}

bool InputManager::isUp(std::string_view action) {
    const ActionState* act = nullptr;

    auto it = mActions.find(action.data());

    if (it != mActions.end())
    {
        act = &it->second;
    }
    if(act == nullptr){
        ConsoleManager::get().log(WARNING, "Tried to check if action is up value on a non existing action: %s", action);
        return false;
    }
    return act->mUp;
}

float InputManager::getAxis(std::string_view action) {
    const ActionState* act = nullptr;

    auto it = mActions.find(action.data());

    if (it != mActions.end())
    {
        act = &it->second;
    }
    if(act == nullptr){
        ConsoleManager::get().log(WARNING, "Tried to check if action value on a non existing action: %s", action);
        return 0.0f;
    }
    return act->mValue;
}

void InputManager::addContext(InputContext context) {
    mContexts.emplace(context.mName, context);
}

void InputManager::setContext(std::string_view name) {
    InputContext* context = nullptr;

    auto it = mContexts.find(name.data());

    if (it != mContexts.end())
    {
        context = &it->second;
    }
    if(context == nullptr){
        ConsoleManager::get().log(WARNING, "Tried to set a context that doesnt exist: %s", name);
        return;
    }

    //dont know if this should be a pointer to a pointer
    mActiveContext = context;
}

void InputManager::process() {
    if (mActiveContext == nullptr) {
        ConsoleManager::get().log(WARNING, "Tried to process keybindings with no active context");
        return;
    }
    for(auto bind : mActiveContext->mBindings){
        if(!mActions.contains(bind.mAction)){
            ConsoleManager::get().log(WARNING, "Tried to process a keybinding that doesnt have the corresponding action registered: %s", bind.mAction.c_str());
            continue;
        }
        ActionState* action = nullptr;

        auto it = mActions.find(bind.mAction);

        if (it != mActions.end())
        {
            action = &it->second;
        }
        if(action == nullptr) continue;

        action->mPressed = false;
        action->mPressedRepeat = false;
        action->mHeld = false;
        action->mUp = false;
        action->mReleased = false;
        action->mValue = 0.0f;
        for(auto &device : mDevices){
            if(!bind.mKeyCodes.contains(device->getType())) continue;

            int code = -1;

            auto it = bind.mKeyCodes.find(device->getType());

            if (it != bind.mKeyCodes.end())
            {
                code = it->second;
            }
            if(device->getButtonPressed(code)) action->mPressed = true;
            if(device->getButtonPressedRepeat(code)) action->mPressedRepeat = true;
            if(device->getButtonDown(code)) action->mHeld = true;
            if(device->getButtonUp(code)) action->mUp = true;
            if(device->getButtonReleased(code)) action->mReleased = true;
            if(device->getAxis(code) != 0.0f) action->mValue = device->getAxis(code);
        }
    }
}

void InputManager::init(std::string_view path) {
    addDevice(std::make_unique<KeyboardDevice>());
    addDevice(std::make_unique<ControllerDevice>());

    load(path);
}

void InputManager::load(std::string_view path) {
    std::ifstream file(path.data());

    if (!file)
    {
        ConsoleManager::get().log(FATAL, "Failed to load keybinds.json");
        return;
    }

    mContexts.clear();

    nlohmann::json data;
    file >> data;

    for (const auto& context : data["contexts"])
    {
        InputContext inputContext{};
        inputContext.mName = context["name"];


        for (const auto& binding : context["bindings"])
        {
            Keybind keybind{};
            keybind.mAction = binding["action"];

            int keyboard = binding["keyCodes"].value("keyboard", -1);
            int controller = binding["keyCodes"].value("controller", -1);

            if (keyboard != -1)
            {
                keybind.addBind(InputDevice::Type::KEYBOARD, keyboard);
            }

            if (controller != -1)
            {
                keybind.addBind(InputDevice::Type::CONTROLLER, controller);
            }


            inputContext.mBindings.push_back(keybind);
            addAction(keybind.mAction);
        }
        addContext(inputContext);
    }
}

void InputManager::addAction(std::string_view name) {
    ActionState actionState{};
    mActions.emplace(name, actionState);
}

void InputManager::addDevice(std::unique_ptr<InputDevice> devicePtr) {
    mDevices.emplace_back(std::move(devicePtr));
}
