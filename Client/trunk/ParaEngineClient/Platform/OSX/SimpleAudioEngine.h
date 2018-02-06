


namespace ParaEngine
{

class SimpleAudioEngine{

public:
    static SimpleAudioEngine* getInstance();

    SimpleAudioEngine();

    void stopAllEffects();
    void preloadEffect(const char* filename);
    void setBackgroundMusicVolume(float volume);
    void setEffectsVolume(float volume);
    float getBackgroundMusicVolume();

    unsigned int playEffect(const char* filename,bool loop=false);
    void playBackgroundMusic(const char* filename,bool loop=false);

    void rewindBackgroundMusic();

    void pauseEffect(unsigned int id);
    void stopBackgroundMusic();

private:
    ~SimpleAudioEngine();
};

}
