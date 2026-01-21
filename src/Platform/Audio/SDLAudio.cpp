#include "SDLAudio.h"
#include <cstring>

#ifdef __linux__

SDLAudio::SDLAudio() {}
SDLAudio::~SDLAudio() {
    stop();
}

static SDL_AudioFormat to_sdl_format(AudioFormat f) {
    switch (f) {
        case AUDIOFMT_U8:  return AUDIO_U8;
        case AUDIOFMT_S16: return AUDIO_S16SYS;
        case AUDIOFMT_S32: return AUDIO_S32SYS;
        case AUDIOFMT_F32: return AUDIO_F32SYS;
        default:           return AUDIO_S16SYS;
    }
}

void SDLAudio::sdl_callback(void* userdata, Uint8* out, int len) {
    SDLAudio* self = static_cast<SDLAudio*>(userdata);

    if (!self->current || self->current->state != AUDIO_PLAYING) {
        SDL_memset(out, 0, len);
        return;
    }

    AudioStream* s = self->current;
    size_t got = s->callback(s->user, out, len);

    if (got < (size_t)len) {
        SDL_memset(out + got, 0, len - got);
        s->state = AUDIO_EOF;
    }

    if (self->volume != 1.0f) {
        SDL_MixAudioFormat(
            out,
            out,
            to_sdl_format(s->format),
            len,
            (int)(self->volume * SDL_MIX_MAXVOLUME)
        );
    }
}

void SDLAudio::init() {
    SDL_InitSubSystem(SDL_INIT_AUDIO);
}

void SDLAudio::play(AudioStream* stream) {
    stop();

    SDL_AudioSpec want{};
    want.freq = stream->sampleRate;
    want.channels = stream->channels;
    want.format = to_sdl_format(stream->format);
    want.samples = 1024;
    want.callback = sdl_callback;
    want.userdata = this;

    device = SDL_OpenAudioDevice(nullptr, 0, &want, nullptr, 0);
    if (!device) return;

    current = stream;
    current->state = AUDIO_PLAYING;

    SDL_PauseAudioDevice(device, 0);
}

void SDLAudio::stop() {
    if (device) {
        SDL_CloseAudioDevice(device);
        device = 0;
    }
    current = nullptr;
}

void SDLAudio::setVolume(float v) {
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;
    volume = v;
}

#endif
