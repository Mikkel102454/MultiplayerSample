#include "sound_manager.h"

#include <vector>
#include <algorithm>

#include "manager/console_manager.h"
#include "util/dev/console/console.h"

namespace {
    bool hasAudioExt(const std::filesystem::path& p) {
        auto ext = p.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return (char)std::tolower(c); });
        return ext == ".wav" || ext == ".ogg" || ext == ".mp3" || ext == ".flac";
    }

    int extPriority(const std::filesystem::path& p) {
        auto ext = p.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return (char)std::tolower(c); });
        // Prefer ogg for size, wav for zero decode latency, mp3 last (typical)
        if (ext == ".ogg")  return 0;
        if (ext == ".wav")  return 1;
        if (ext == ".flac") return 2;
        if (ext == ".mp3")  return 3;
        return 99;
    }
}

void SoundManager::init(std::string baseDir) {
    if (mInitialized) return;

    mBaseDir = std::filesystem::path(baseDir);
    mInitialized = true;

    rescan();
}

void SoundManager::shutdown() {
    if (!mInitialized) return;

    stopMusic();

    for (auto& [_, snd] : mSounds) {
        UnloadSound(snd);
    }
    mSounds.clear();

    mRegistry.clear();
    mInitialized = false;
}

void SoundManager::update() {
    if (!mInitialized) return;

    if (mMusicLoaded) {
        UpdateMusicStream(mMusic);
    }
}

void SoundManager::rescan() {
    if (!mInitialized) return;

    mRegistry.clear();

    if (!std::filesystem::exists(mBaseDir)) {
        ConsoleManager::get().log(WARNING, "SoundManager: sounds directory does not exist: %s", mBaseDir.c_str());
        return;
    }

    struct Candidate {
        std::filesystem::path path;
        int pri = 99;
    };
    std::unordered_map<std::string, Candidate> best;

    for (auto it = std::filesystem::recursive_directory_iterator(mBaseDir);
         it != std::filesystem::recursive_directory_iterator();
         ++it) {

        if (!it->is_regular_file()) continue;

        const auto& p = it->path();
        if (!hasAudioExt(p)) continue;

        std::error_code ec;
        auto rel = std::filesystem::relative(p, mBaseDir, ec);
        if (ec) continue;

        auto relNoExt = rel;
        relNoExt.replace_extension(); // drop extension
        std::string key = keyFromPathRelativeNoExt(relNoExt);

        Candidate cand{ p, extPriority(p) };

        auto found = best.find(key);
        if (found == best.end() || cand.pri < found->second.pri) {
            best[key] = cand;
        }
    }

    for (auto& [key, cand] : best) {
        mRegistry.emplace(key, cand.path);
    }

    ConsoleManager::get().log(INFO, "SoundManager: rescanned sounds directory: %s", mBaseDir.string().c_str());
}

bool SoundManager::has(std::string_view key) {
    if (!mInitialized) return false;
    return mRegistry.contains(normalizeKey(key));
}

void SoundManager::play(std::string_view key) {
    play(key, PlayOptions{});
}

void SoundManager::play(std::string_view key, PlayOptions opt) {
    if (!mInitialized) return;

    const std::string k = normalizeKey(key);

    if (isMusicKey(k)) {
        ConsoleManager::get().log(WARNING, "SoundManager: play() called with a music key. Use playMusic(): %s", k.c_str());
        return;
    }

    if (!mRegistry.contains(k)) {
        ConsoleManager::get().log(WARNING, "SoundManager: unknown sound key: %s", k.c_str());
        return;
    }

    Sound& snd = getOrLoadSound(k);

    const Bus bus = (opt.bus == Bus::Auto) ? inferBusFromKey(k) : opt.bus;

    const float vol = finalGain(bus, opt.volume);
    SetSoundVolume(snd, clamp01(vol));
    SetSoundPitch(snd, opt.pitch);
    SetSoundPan(snd, clampPan(opt.pan));
    PlaySound(snd);
}

void SoundManager::playMusic(std::string_view key) {
    playMusic(key, MusicOptions{});
}

void SoundManager::playMusic(std::string_view key, MusicOptions opt) {
    if (!mInitialized) return;

    const std::string k = normalizeKey(key);

    if (!mRegistry.contains(k)) {
        ConsoleManager::get().log(WARNING, "SoundManager: unknown music key: %s", k.c_str());
        return;
    }

    if (!isMusicKey(k)) {
        ConsoleManager::get().log(WARNING, "SoundManager: playMusic() called with non-music key (expected under music/): %s", k.c_str());
    }

    if (mMusicLoaded) {
        StopMusicStream(mMusic);
        UnloadMusicStream(mMusic);
        mMusicLoaded = false;
        mCurrentMusicKey.clear();
    }

    const auto path = mRegistry.at(k);
    mMusic = LoadMusicStream(path.string().c_str());
    mMusicLoaded = true;
    mCurrentMusicKey = k;

    const Bus bus = (opt.bus == Bus::Auto) ? inferBusFromKey(k) : opt.bus;

    SetMusicPitch(mMusic, opt.pitch);
    SetMusicVolume(mMusic, clamp01(finalGain(bus, opt.volume)));

    PlayMusicStream(mMusic);

    if (!opt.loop) {
        ConsoleManager::get().log(WARNING, "SoundManager: MusicOptions.loop=false not implemented yet (will loop): %s", k.c_str());
    }
}

void SoundManager::stopMusic() {
    if (!mInitialized) return;
    if (!mMusicLoaded) return;

    StopMusicStream(mMusic);
    UnloadMusicStream(mMusic);
    mMusicLoaded = false;
    mCurrentMusicKey.clear();
}

void SoundManager::setMasterVolume(float v) {
    mMasterVolume = clamp01(v);

    if (mMusicLoaded) {
        SetMusicVolume(mMusic, clamp01(finalGain(Bus::Music, 1.0f)));
    }
}

void SoundManager::setBusVolume(Bus bus, float v) {
    mBusVolumes[bus] = clamp01(v);

    if (bus == Bus::Music && mMusicLoaded) {
        SetMusicVolume(mMusic, clamp01(finalGain(Bus::Music, 1.0f)));
    }
}

float SoundManager::getMasterVolume() {
    return mMasterVolume;
}

float SoundManager::getBusVolume(Bus bus) {
    auto it = mBusVolumes.find(bus);
    if (it == mBusVolumes.end()) return 1.0f;
    return it->second;
}

std::string SoundManager::normalizeKey(std::string_view key) {
    std::string k(key);

    std::replace(k.begin(), k.end(), '\\', '/');

    while (!k.empty() && (k.front() == '/')) k.erase(k.begin());
    while (!k.empty() && (k.back() == '/')) k.pop_back();

    return k;
}

std::string SoundManager::keyFromPathRelativeNoExt(const std::filesystem::path& relNoExt) {
    std::string k = relNoExt.generic_string();
    return normalizeKey(k);
}

bool SoundManager::isMusicKey(std::string_view key) {
    const std::string k = normalizeKey(key);
    return k.rfind("music/", 0) == 0;
}

float SoundManager::clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

float SoundManager::clampPan(float v) {
    if (v < -1.0f) return -1.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

SoundManager::Bus SoundManager::inferBusFromKey(std::string_view key) {
    const std::string k = normalizeKey(key);

    if (k.rfind("music/", 0) == 0)    return Bus::Music;
    if (k.rfind("ui/", 0) == 0)       return Bus::Ui;
    if (k.rfind("ambience/", 0) == 0) return Bus::Ambience;
    if (k.rfind("sfx/", 0) == 0)      return Bus::Sfx;

    return Bus::Sfx;
}

std::filesystem::path SoundManager::resolvePath(std::string_view key) {
    const std::string k = normalizeKey(key);
    auto it = mRegistry.find(k);
    if (it == mRegistry.end()) return {};
    return it->second;
}

Sound& SoundManager::getOrLoadSound(std::string_view key) {
    const std::string k = normalizeKey(key);

    auto it = mSounds.find(k);
    if (it != mSounds.end()) return it->second;

    const auto path = resolvePath(k);
    if (path.empty()) {
        ConsoleManager::get().log(WARNING, "SoundManager: failed to resolve sound key: %s", k.c_str());
    }

    Sound snd = LoadSound(path.string().c_str());
    auto [insertedIt, _] = mSounds.emplace(k, snd);
    return insertedIt->second;
}

float SoundManager::busGain(Bus bus) {
    // Master bus volume is treated separately via mMasterVolume,
    // but we keep it here in case we want to route through it later.
    auto it = mBusVolumes.find(bus);
    if (it == mBusVolumes.end()) return 1.0f;
    return it->second;
}

float SoundManager::finalGain(Bus bus, float instanceVolume) {
    return clamp01(mMasterVolume * busGain(bus) * instanceVolume);
}