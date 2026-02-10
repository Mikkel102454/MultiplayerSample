#ifndef BUTTON_H
#define BUTTON_H

#include <functional>

#include "raylib.h"
#include "ui/element.h"
class Button : public Element {
public:
    enum class State {
        NORMAL,
        HOVERED,
        PRESSED
    };
};

class ThreeStateButton : public Button {
public:
    ThreeStateButton(int posX, int posY, const Texture2D& texture)
        : mTexture(texture) {

        const float width  = static_cast<float>(texture.width);
        const float height = static_cast<float>(texture.height);
        frameHeight = height / 3.0f;

        mBounds = {
            static_cast<float>(posX),
            static_cast<float>(posY),
            width,
            frameHeight
        };

        sourceRec = { 0.0f, 0.0f, width, frameHeight };
    }

    ~ThreeStateButton() override = default;

    void draw() override;
    bool update() override;

    void setOnClick(std::function<void()> onClick) { mOnClick = std::move(onClick); }

private:
    State mState = State::NORMAL;
    Rectangle mBounds;
    float frameHeight;
    Rectangle sourceRec;

    Texture2D mTexture;

    std::function<void()> mOnClick{};
};

#endif //BUTTON_H