#include "ui/elements/button.h"

#include "input/input.h"

void ThreeStateButton::draw() {
    sourceRec.y = frameHeight * static_cast<float>(mState);
    DrawTextureRec(mTexture, sourceRec, (Vector2){ mBounds.x, mBounds.y }, WHITE);
}

bool ThreeStateButton::update() {
    if (CheckCollisionPointRec(GetMousePosition(), mBounds))
    {
        if (InputManager::get()->isHeld("ui_click")) mState = State::PRESSED;
        else mState = State::HOVERED;

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            if (mOnClick) mOnClick();
            return true;
        }
    }
    else mState = State::NORMAL;

    return false;
}
