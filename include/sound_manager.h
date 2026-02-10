#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include <string>
#include <string_view>
#include <unordered_map>
#include <filesystem>

#include "raylib.h"

class SoundManager {
public:
    enum class Bus {
        Auto,
        Master,
        Sfx,
        Music,
        Ui,
        Ambience
    };

    struct PlayOptions {
        float volume = 1.0f;   // 0..1 (scaled by bus + master)
        float pitch  = 1.0f;   // ~0.5..2
        float pan    = 0.0f;   // -1..1 (left..right)
        Bus bus      = Bus::Auto;
    };

    struct MusicOptions {
        float volume = 1.0f;   // 0..1 (scaled by bus + master)
        float pitch  = 1.0f;
        bool loop    = true;
        Bus bus      = Bus::Auto;
    };

    // Lifecycle
    static void init(std::string baseDir = std::string(ASSETS_PATH "pixel_game/sounds/"));
    static void shutdown();
    static void update(); // call once per frame (streams music)

    // Registry / info
    static void rescan();
    static bool has(std::string_view key);

    // Playback (1D for now)
    static void play(std::string_view key);
    static void play(std::string_view key, PlayOptions opt);

    static void playMusic(std::string_view key);
    static void playMusic(std::string_view key, MusicOptions opt);

    static void stopMusic();

    // Volume control
    static void setMasterVolume(float v);
    static void setBusVolume(Bus bus, float v);
    static float getMasterVolume();
    static float getBusVolume(Bus bus);

private:
    static std::string normalizeKey(std::string_view key);
    static std::string keyFromPathRelativeNoExt(const std::filesystem::path& relNoExt);
    static bool isMusicKey(std::string_view key);
    static float clamp01(float v);
    static float clampPan(float v);

    static Bus inferBusFromKey(std::string_view key);

    static std::filesystem::path resolvePath(std::string_view key);
    static Sound& getOrLoadSound(std::string_view key);

    static float busGain(Bus bus);
    static float finalGain(Bus bus, float instanceVolume);

private:
    inline static bool mInitialized = false;
    inline static std::filesystem::path mBaseDir;

    // key ("sfx/explosion") -> absolute path (".../sfx/explosion.wav")
    inline static std::unordered_map<std::string, std::filesystem::path> mRegistry;

    // Cached loaded SFX
    inline static std::unordered_map<std::string, Sound> mSounds;

    // Single music channel for now (simple + typical)
    inline static bool mMusicLoaded = false;
    inline static std::string mCurrentMusicKey;
    inline static Music mMusic{};

    // Busses
    inline static float mMasterVolume = 1.0f;
    inline static std::unordered_map<Bus, float> mBusVolumes = {
        {Bus::Master,   1.0f},
        {Bus::Sfx,      1.0f},
        {Bus::Music,    1.0f},
        {Bus::Ui,       1.0f},
        {Bus::Ambience, 1.0f},
    };
};

#endif // SOUND_MANAGER_H