#pragma once
#include<map>
#include<unordered_map>
#include<memory>
#include<optional>
#include<vector>
#include<string>
#include<cstddef>
#include<atomic>
#include<mutex>
#include<thread>
#include<SFML/Graphics.hpp>
#include<SFML/Audio.hpp>
#include<iostream>

struct Asset
{
    std::string name;
    std::string type;
    std::string filePath;
    Asset(const std::string& name , const std::string& type , const std::string& filePath) : name(name) , type(type) , filePath(filePath){} 
};

class AssetManager
{
    public:
        using SoundId = std::size_t;

        enum class LoadStatus
        {
            Idle,       // no global load was ever started
            Loading,    // loader thread is working
            Finished,   // every global asset is loaded
            Failed      // an asset failed to load, see getGlobalLoadError()
        };

    private:
        // A sound that is currently playing / paused. Keeps the buffer alive for as long as the sound exists.
        struct PlayingSound
        {
            std::string name;
            std::shared_ptr<sf::SoundBuffer> buffer;
            std::unique_ptr<sf::Sound> sound;
        };

        // Guards all the asset maps below, since global assets are inserted from the loader thread.
        mutable std::mutex m_assetsMutex;

        std::unordered_map<std::string , std::shared_ptr<sf::Texture>> m_textureNameToTextures;
        std::unordered_map<std::string , std::shared_ptr<sf::Texture>> m_pathToTextures;

        std::unordered_map<std::string , std::shared_ptr<sf::SoundBuffer>> m_soundNameToSoundBuffers;
        std::unordered_map<std::string , std::shared_ptr<sf::SoundBuffer>> m_pathToSoundBuffers;

        std::unordered_map<std::string , std::shared_ptr<sf::Music>> m_musicNameToMusics;

        // Main thread only.
        std::unordered_map<SoundId , PlayingSound> m_playingSounds;
        SoundId m_nextSoundId = 0;

        // Global loading state.
        std::atomic<LoadStatus> m_globalLoadStatus{LoadStatus::Idle};
        std::atomic<std::size_t> m_globalTotalAssets{0};
        std::atomic<std::size_t> m_globalLoadedAssets{0};
        mutable std::mutex m_globalErrorMutex;
        std::string m_globalLoadError;

        void m_addTexture(const std::string& textureName , const std::string& texturePath , bool isGlobal);        
        void m_addSound(const std::string& soundName , const std::string& soundPath , bool isGlobal);
        void m_addMusic(const std::string& musicName , const std::string& musicPath , bool isGlobal);
        void m_loadAsset(const Asset& asset , bool globally);
        void m_loadAssets( const std::optional<std::vector<Asset>>& assets , bool globally);
        void m_loadAssetsAsync(const std::vector<Asset>& assets);
        std::shared_ptr<sf::SoundBuffer> m_getSoundBufferPtr(const std::string& soundName) const;
        void m_removeFinishedSounds();

        // Must stay the LAST member: it is destroyed first, which stops and joins the loader thread
        // before the maps / mutexes it uses are destroyed.
        std::jthread m_loaderThread;

        friend class SceneManager;
    public:
        const sf::Texture& getTexture(const std::string& textureName) const;
        const sf::SoundBuffer& getSoundBuffer(const std::string& soundName) const;
        sf::Music& getMusic(const std::string& musicName) const;

        // ---- Global asset loading (runs on a separate thread) ----
        LoadStatus getGlobalLoadStatus() const;
        // 0 - 100. Counts finished assets out of the total of the current global load.
        float getGlobalLoadProgress() const;
        bool isGlobalLoadFinished() const;
        // Empty unless the status is Failed.
        std::string getGlobalLoadError() const;
        // Blocks until the loader thread is done. Throws std::runtime_error if the load failed.
        void waitForGlobalAssets();

        // ---- Sound playback ----
        // Starts a sound and returns an id to refer to it later. The manager owns the sf::Sound.
        SoundId playSound(const std::string& soundName , float volume = 100.f , bool loop = false);

        // Returns the sound while it is alive (playing or paused), nullptr once it has finished or been stopped.
        sf::Sound* getPlayingSound(SoundId id);
        bool isSoundPlaying(SoundId id);

        // Control by id. Return false if the sound no longer exists (e.g. it finished by itself).
        bool pauseSound(SoundId id);
        bool resumeSound(SoundId id);
        bool stopSound(SoundId id);

        // Control by name, affects every currently alive instance of that sound.
        void pauseSounds(const std::string& soundName);
        void resumeSounds(const std::string& soundName);
        void stopSounds(const std::string& soundName);

        // Control all sounds.
        void pauseAllSounds();
        void resumeAllSounds();
        void stopAllSounds();

        // Optional: call once per frame to free finished sounds. Finished sounds are also cleaned up on every playSound call.
        void update();
};