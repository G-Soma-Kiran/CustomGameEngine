#include"CAssetManager.hpp"
#include<iostream>
#include<format>
#include<stdexcept>

void AssetManager::m_addTexture(const std::string& textureName,const std::string& texturePath , bool isGlobal)
{
    std::string finalTextureName;
    if(isGlobal)
    {
        finalTextureName = std::string("Global_") + textureName;
    }
    else
    {
        finalTextureName = textureName;
    }

    std::shared_ptr<sf::Texture> texture;

    {
        std::lock_guard<std::mutex> lock(m_assetsMutex);

        if (m_textureNameToTextures.contains(finalTextureName))
        {
            throw std::runtime_error(
                "Texture with name '" + finalTextureName + "' already exists.");
        }

        auto pathIt = m_pathToTextures.find(texturePath);

        if (pathIt != m_pathToTextures.end())
        {
            texture = pathIt->second;
        }
    }

    if (!texture)
    {
        // Disk work is done without holding the lock so the main thread is never blocked by it.
        texture = std::make_shared<sf::Texture>();

        if (!texture->loadFromFile(texturePath))
        {
            throw std::runtime_error(
                "Failed to load texture from file: " + texturePath);
        }
    }

    std::lock_guard<std::mutex> lock(m_assetsMutex);

    // If another thread cached the same path meanwhile, use that one.
    auto [pathIt, insertedPath] = m_pathToTextures.try_emplace(texturePath, texture);
    texture = pathIt->second;

    auto [nameIt, insertedName] = m_textureNameToTextures.try_emplace(finalTextureName, texture);
    if (!insertedName)
    {
        throw std::runtime_error(
            "Texture with name '" + finalTextureName + "' already exists.");
    }
}

void AssetManager::m_addSound(const std::string& soundName,const std::string& soundPath , bool isGlobal)
{
    std::string finalSoundName;
    if(isGlobal)
    {
        finalSoundName = std::string("Global_") + soundName;
    }
    else
    {
        finalSoundName = soundName;
    }

    std::shared_ptr<sf::SoundBuffer> soundBuffer;

    {
        std::lock_guard<std::mutex> lock(m_assetsMutex);

        if (m_soundNameToSoundBuffers.contains(finalSoundName))
        {
            throw std::runtime_error(
                "Sound with name '" + finalSoundName + "' already exists.");
        }

        auto pathIt = m_pathToSoundBuffers.find(soundPath);

        if (pathIt != m_pathToSoundBuffers.end())
        {
            soundBuffer = pathIt->second;
        }
    }

    if (!soundBuffer)
    {
        soundBuffer = std::make_shared<sf::SoundBuffer>();

        if (!soundBuffer->loadFromFile(soundPath))
        {
            throw std::runtime_error(
                "Failed to load sound from file: " + soundPath);
        }
    }

    std::lock_guard<std::mutex> lock(m_assetsMutex);

    auto [pathIt, insertedPath] = m_pathToSoundBuffers.try_emplace(soundPath, soundBuffer);
    soundBuffer = pathIt->second;

    auto [nameIt, insertedName] = m_soundNameToSoundBuffers.try_emplace(finalSoundName, soundBuffer);
    if (!insertedName)
    {
        throw std::runtime_error(
            "Sound with name '" + finalSoundName + "' already exists.");
    }
}

void AssetManager::m_addMusic(const std::string& musicName,const std::string& musicPath , bool isGlobal)
{
    std::string finalMusicName;
    if(isGlobal)
    {
        finalMusicName = std::string("Global_") + musicName;
    }
    else
    {
        finalMusicName = musicName;
    }

    {
        std::lock_guard<std::mutex> lock(m_assetsMutex);

        if (m_musicNameToMusics.contains(finalMusicName))
        {
            throw std::runtime_error(
                "Music with name '" + finalMusicName + "' already exists.");
        }
    }

    // Music is streamed from disk and holds playback state (position, status, volume),
    // so unlike textures / sound buffers it is NOT shared between names by path.
    auto music = std::make_shared<sf::Music>();

    if (!music->openFromFile(musicPath))
    {
        throw std::runtime_error(
            "Failed to open music from file: " + musicPath);
    }

    std::lock_guard<std::mutex> lock(m_assetsMutex);

    auto [it, inserted] = m_musicNameToMusics.try_emplace(finalMusicName, music);
    if (!inserted)
    {
        throw std::runtime_error(
            "Music with name '" + finalMusicName + "' already exists.");
    }
}

const sf::Texture& AssetManager::getTexture(const std::string& textureName) const
{
    std::lock_guard<std::mutex> lock(m_assetsMutex);

    auto it = m_textureNameToTextures.find(textureName);
    if(it != m_textureNameToTextures.end())
    {
        return *it->second;
    }
    else
    {
        it = m_textureNameToTextures.find(std::string("Global_"+textureName));
        if(it != m_textureNameToTextures.end())
        {
            return *it->second;
        }
        throw std::runtime_error(std::format("There is no texture named {} in AssetManager\n" , textureName));
    }

}

std::shared_ptr<sf::SoundBuffer> AssetManager::m_getSoundBufferPtr(const std::string& soundName) const
{
    std::lock_guard<std::mutex> lock(m_assetsMutex);

    auto it = m_soundNameToSoundBuffers.find(soundName);
    if(it != m_soundNameToSoundBuffers.end())
    {
        return it->second;
    }
    else
    {
        it = m_soundNameToSoundBuffers.find(std::string("Global_"+soundName));
        if(it != m_soundNameToSoundBuffers.end())
        {
            return it->second;
        }
        throw std::runtime_error(std::format("There is no sound named {} in AssetManager\n" , soundName));
    }
}

const sf::SoundBuffer& AssetManager::getSoundBuffer(const std::string& soundName) const
{
    return *this->m_getSoundBufferPtr(soundName);
}

sf::Music& AssetManager::getMusic(const std::string& musicName) const
{
    std::lock_guard<std::mutex> lock(m_assetsMutex);

    auto it = m_musicNameToMusics.find(musicName);
    if(it != m_musicNameToMusics.end())
    {
        return *it->second;
    }
    else
    {
        it = m_musicNameToMusics.find(std::string("Global_"+musicName));
        if(it != m_musicNameToMusics.end())
        {
            return *it->second;
        }
        throw std::runtime_error(std::format("There is no music named {} in AssetManager\n" , musicName));
    }
}

void AssetManager::m_loadAsset(const Asset& asset , bool globally)
{
    if(asset.type == "Texture")
    {
        this->m_addTexture(asset.name , asset.filePath , globally);
    }
    else if(asset.type == "Sound")
    {
        this->m_addSound(asset.name , asset.filePath , globally);
    }
    else if(asset.type == "Music")
    {
        this->m_addMusic(asset.name , asset.filePath , globally);
    }
}

void AssetManager::m_loadAssets( const std::optional<std::vector<Asset>>& assets  , bool globally)
{
    if(assets)
    {
        if(globally)
        {
            // Global assets are loaded on a separate thread, progress is available through getGlobalLoadStatus / getGlobalLoadProgress.
            this->m_loadAssetsAsync(assets.value());
        }
        else
        {
            // Local assets are loaded on the calling (main) thread.
            for( const auto& asset : assets.value() )
            {
                this->m_loadAsset(asset , false);
            }
        }
    }
}

void AssetManager::m_loadAssetsAsync(const std::vector<Asset>& assets)
{
    if(m_globalLoadStatus.load() == LoadStatus::Loading)
    {
        throw std::runtime_error("A global asset load is already in progress.");
    }

    // The previous loader thread (if any) has already finished its work, just reclaim it.
    if(m_loaderThread.joinable())
    {
        m_loaderThread.join();
    }

    {
        std::lock_guard<std::mutex> lock(m_globalErrorMutex);
        m_globalLoadError.clear();
    }

    m_globalTotalAssets.store(assets.size());
    m_globalLoadedAssets.store(0);

    if(assets.empty())
    {
        m_globalLoadStatus.store(LoadStatus::Finished);
        return;
    }

    m_globalLoadStatus.store(LoadStatus::Loading);

    m_loaderThread = std::jthread([this , assets](std::stop_token stopToken)
    {
        try
        {
            for( const auto& asset : assets )
            {
                if(stopToken.stop_requested())
                {
                    return;
                }

                this->m_loadAsset(asset , true);
                m_globalLoadedAssets.fetch_add(1);
            }
            m_globalLoadStatus.store(LoadStatus::Finished);
        }
        catch(const std::exception& e)
        {
            {
                std::lock_guard<std::mutex> lock(m_globalErrorMutex);
                m_globalLoadError = e.what();
            }
            m_globalLoadStatus.store(LoadStatus::Failed);
        }
    });
}

AssetManager::LoadStatus AssetManager::getGlobalLoadStatus() const
{
    return m_globalLoadStatus.load();
}

float AssetManager::getGlobalLoadProgress() const
{
    std::size_t total = m_globalTotalAssets.load();
    if(total == 0)
    {
        return m_globalLoadStatus.load() == LoadStatus::Finished ? 100.f : 0.f;
    }
    return 100.f * static_cast<float>(m_globalLoadedAssets.load()) / static_cast<float>(total);
}

bool AssetManager::isGlobalLoadFinished() const
{
    return m_globalLoadStatus.load() == LoadStatus::Finished;
}

std::string AssetManager::getGlobalLoadError() const
{
    std::lock_guard<std::mutex> lock(m_globalErrorMutex);
    return m_globalLoadError;
}

void AssetManager::waitForGlobalAssets()
{
    if(m_loaderThread.joinable())
    {
        m_loaderThread.join();
    }

    if(m_globalLoadStatus.load() == LoadStatus::Failed)
    {
        throw std::runtime_error("Global asset load failed: " + this->getGlobalLoadError());
    }
}

void AssetManager::m_removeFinishedSounds()
{
    for(auto it = m_playingSounds.begin(); it != m_playingSounds.end(); )
    {
        if(it->second.sound->getStatus() == sf::SoundSource::Status::Stopped)
        {
            it = m_playingSounds.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

AssetManager::SoundId AssetManager::playSound(const std::string& soundName , float volume , bool loop)
{
    this->m_removeFinishedSounds();

    PlayingSound playingSound;
    playingSound.name = soundName;
    playingSound.buffer = this->m_getSoundBufferPtr(soundName);
    playingSound.sound = std::make_unique<sf::Sound>(*playingSound.buffer);
    playingSound.sound->setVolume(volume);
    playingSound.sound->setLooping(loop);
    playingSound.sound->play();

    SoundId id = m_nextSoundId++;
    m_playingSounds.try_emplace(id , std::move(playingSound));
    return id;
}

sf::Sound* AssetManager::getPlayingSound(SoundId id)
{
    auto it = m_playingSounds.find(id);
    if(it == m_playingSounds.end())
    {
        return nullptr;
    }

    if(it->second.sound->getStatus() == sf::SoundSource::Status::Stopped)
    {
        m_playingSounds.erase(it);
        return nullptr;
    }
    return it->second.sound.get();
}

bool AssetManager::isSoundPlaying(SoundId id)
{
    sf::Sound* sound = this->getPlayingSound(id);
    return sound && sound->getStatus() == sf::SoundSource::Status::Playing;
}

bool AssetManager::pauseSound(SoundId id)
{
    sf::Sound* sound = this->getPlayingSound(id);
    if(!sound) return false;
    sound->pause();
    return true;
}

bool AssetManager::resumeSound(SoundId id)
{
    sf::Sound* sound = this->getPlayingSound(id);
    if(!sound) return false;
    sound->play();
    return true;
}

bool AssetManager::stopSound(SoundId id)
{
    auto it = m_playingSounds.find(id);
    if(it == m_playingSounds.end()) return false;
    it->second.sound->stop();
    m_playingSounds.erase(it);
    return true;
}

void AssetManager::pauseSounds(const std::string& soundName)
{
    for(auto& [id , playingSound] : m_playingSounds)
    {
        if(playingSound.name == soundName && playingSound.sound->getStatus() == sf::SoundSource::Status::Playing)
        {
            playingSound.sound->pause();
        }
    }
}

void AssetManager::resumeSounds(const std::string& soundName)
{
    for(auto& [id , playingSound] : m_playingSounds)
    {
        if(playingSound.name == soundName && playingSound.sound->getStatus() == sf::SoundSource::Status::Paused)
        {
            playingSound.sound->play();
        }
    }
}

void AssetManager::stopSounds(const std::string& soundName)
{
    for(auto it = m_playingSounds.begin(); it != m_playingSounds.end(); )
    {
        if(it->second.name == soundName)
        {
            it->second.sound->stop();
            it = m_playingSounds.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void AssetManager::pauseAllSounds()
{
    for(auto& [id , playingSound] : m_playingSounds)
    {
        if(playingSound.sound->getStatus() == sf::SoundSource::Status::Playing)
        {
            playingSound.sound->pause();
        }
    }
}

void AssetManager::resumeAllSounds()
{
    for(auto& [id , playingSound] : m_playingSounds)
    {
        if(playingSound.sound->getStatus() == sf::SoundSource::Status::Paused)
        {
            playingSound.sound->play();
        }
    }
}

void AssetManager::stopAllSounds()
{
    m_playingSounds.clear();
}

void AssetManager::update()
{
    this->m_removeFinishedSounds();
}