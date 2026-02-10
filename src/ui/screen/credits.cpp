#include "ui/screen/credits.h"

#include <fstream>

static constexpr float CREDITS_LETTER_SPACING = 1.0f;

void CreditsScreen::loadCreditsFile() {
    mLines.clear();

    std::ifstream file(ASSETS_PATH "pixel_game/config/credit.txt");
    if (!file.is_open()) {
        mLines.emplace_back("Credits file not found");
        mLines.emplace_back(ASSETS_PATH "pixel_game/config/credit.txt");
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        mLines.emplace_back(line);
    }

    if (mLines.empty()) {
        mLines.emplace_back("No credits.");
    }
}

void CreditsScreen::resetRoll() {
    recomputeLayout();
}

void CreditsScreen::recomputeLayout() {
    mBlockHeightPx = 0.0f;
    if (!mLines.empty()) {
        mBlockHeightPx = static_cast<float>(mLines.size()) * CREDITS_LINE_SPACING;
    }

    const float screenH = static_cast<float>(GetScreenHeight());

    mStartY = screenH + mBottomPadPx;

    mScrollOffsetPx = 0.0f;
}

void CreditsScreen::onUpdate() {
    if (IsKeyPressed(KEY_ESCAPE)) {
        resetRoll();
        if (mNavigate) mNavigate("main_menu");
        return;
    }

    if (IsWindowResized()) {
        recomputeLayout();
    }

    const float dt = GetFrameTime();
    mScrollOffsetPx += CREDITS_SCROLL_SPEED * dt;

    const float lastLineBottomY =
        mStartY + (mBlockHeightPx - CREDITS_LINE_SPACING)
        - mScrollOffsetPx
        + CREDITS_LINE_SPACING;

    if (lastLineBottomY < -mTopPadPx) {
        resetRoll();
        if (mNavigate) mNavigate("main_menu");
        return;
    }
}

void CreditsScreen::onDraw() {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);

    Font font = (mFont != nullptr) ? *mFont : GetFontDefault();
    const float screenW = static_cast<float>(GetScreenWidth());
    const float screenH = static_cast<float>(GetScreenHeight());

    for (size_t i = 0; i < mLines.size(); ++i) {
        const std::string& line = mLines[i];

        const float y =
            mStartY + static_cast<float>(i) * CREDITS_LINE_SPACING - mScrollOffsetPx;

        if (y < -200.0f || y > screenH + 200.0f) continue;

        const Vector2 size = MeasureTextEx(
            font, line.c_str(),
            CREDITS_FONT_SIZE,
            CREDITS_LETTER_SPACING
        );

        const float x = screenW * 0.5f - size.x * 0.5f;

        DrawTextEx(
            font, line.c_str(),
            Vector2{ x, y },
            CREDITS_FONT_SIZE,
            CREDITS_LETTER_SPACING,
            RAYWHITE
        );
    }
}