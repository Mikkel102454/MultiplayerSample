#ifndef RESOURCE_LOADER_H
#define RESOURCE_LOADER_H
#include <map>
#include <string>

#include "raylib.h"

class ResourceLoader {
public:
    static void load();
    static void unload();

    static Texture2D* getTexture2D(std::string_view texture);

    static Font* getFont(std::string_view font);

private:
    inline static std::map<std::string, Texture2D> mTextures{};
    inline static std::map<std::string, Font> mFonts{};
};

#endif //RESOURCE_LOADER_H