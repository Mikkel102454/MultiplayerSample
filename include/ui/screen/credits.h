#ifndef CREDITS_H
#define CREDITS_H

#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include "raylib.h"
#include "sound_manager.h"
#include "ui/screen.h"
#include "util/resource_loader.h"

#define CREDITS_LINE_SPACING 30
#define CREDITS_FONT_SIZE 20
#define CREDITS_SCROLL_SPEED 30

class CreditsScreen : public Screen {
public:
    using NavigateFn = std::function<void(std::string_view)>;

    explicit CreditsScreen(NavigateFn navigate) : mNavigate(std::move(navigate)) {
        mFont = ResourceLoader::getFont("main");

        loadCreditsFile();
        recomputeLayout();
    };
    ~CreditsScreen() override = default;

protected:
    void onDraw() override;
    void onUpdate() override;

private:
    void loadCreditsFile();
    void recomputeLayout();
    void resetRoll();

    NavigateFn mNavigate;

    std::vector<std::string> mLines{};

    Font* mFont = nullptr;
    float mScrollOffsetPx = 0.0f;
    float mBlockHeightPx = 0.0f;

    float mStartY = 0.0f;
    float mBottomPadPx = 50.0f;
    float mTopPadPx = 50.0f;
};

#endif //CREDITS_H