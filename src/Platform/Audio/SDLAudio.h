#pragma once
#include "Audio.h"
#ifdef PC
#include <SDL2/SDL.h>

class SDLAudio : public AudioSource {
  public:
    SDLAudio();
    ~SDLAudio() override;

    void init() override;
    void play(AudioStream* stream) override;
    void stop() override;
    void setVolume(float volume) override;

  private:
    static void sdl_callback(void* userdata, Uint8* stream, int len);

    SDL_AudioDeviceID device  = 0;
    AudioStream*      current = nullptr;
    float             volume  = 1.0f;
};
#endif
