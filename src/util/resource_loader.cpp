#include "util/resource_loader.h"

#include "manager/console_manager.h"
#include "util/dev/console/console.h"

void ResourceLoader::load() {
    mTextures.emplace("button", LoadTexture(ASSETS_PATH "pixel_game/ui/button/button.png"));

    mFonts.emplace("main", LoadFontEx(ASSETS_PATH "pixel_game/fonts/VictorMono-Medium.ttf",
                                          96, nullptr, 0));
}

Texture2D* ResourceLoader::getTexture2D(std::string_view texture) {
    if (!mTextures.contains(texture.data())) {
        ConsoleManager::get().log(FATAL, "Tried to get texture that was not loaded: %s", texture.data());
        return nullptr;
    }
    return &mTextures.at(texture.data());
}

Font* ResourceLoader::getFont(std::string_view font) {
    if (!mFonts.contains(font.data())) {
        ConsoleManager::get().log(FATAL, "Tried to get font that was not loaded: %s", font.data());
        return nullptr;
    }
    return &mFonts.at(font.data());
}
