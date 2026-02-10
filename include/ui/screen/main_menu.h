#ifndef MAINMENU_H
#define MAINMENU_H

#include <functional>
#include <string_view>

#include "raylib.h"
#include "ui/screen.h"
#include "ui/elements/button.h"
#include "util/resource_loader.h"

class MainMenuScreen : public Screen {
public:
    using NavigateFn = std::function<void(std::string_view)>;

    explicit MainMenuScreen(NavigateFn navigate)
        : mNavigate(std::move(navigate)) {

        mElements.emplace_back(&mPlayBtn);

        mCreditBtn.setOnClick([this]() {
            if (mNavigate) mNavigate("credits");
        });
        mElements.emplace_back(&mCreditBtn);

        mQuitBtn.setOnClick([]() {
        });
        mElements.emplace_back(&mQuitBtn);
    }

    ~MainMenuScreen() override = default;

private:
    NavigateFn mNavigate;

    ThreeStateButton mPlayBtn{0, 0, *ResourceLoader::getTexture2D("button")};
    ThreeStateButton mCreditBtn{0, 150, *ResourceLoader::getTexture2D("button")};
    ThreeStateButton mQuitBtn{0, 300, *ResourceLoader::getTexture2D("button")};
};

#endif //MAINMENU_H