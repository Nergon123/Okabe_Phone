#pragma once
#include <stddef.h>
#include <stdint.h>

enum AudioState : uint8_t { AUDIO_PLAYING, AUDIO_STOPPED, AUDIO_EOF };

// AudioCallback is called from a real-time audio thread/task.
// Must not allocate, block, lock, or call non-RT-safe APIs.
// Must return number of bytes written to `out`.
typedef size_t (*AudioCallback)(void* user, void* out, size_t bytes);

enum AudioFormat : uint8_t {
    AUDIOFMT_U8,  // unsigned 8-bit
    AUDIOFMT_S16, // signed 16-bit little-endian
    AUDIOFMT_S32, // signed 32-bit
    AUDIOFMT_F32  // 32-bit float (-1.0 .. 1.0)
};

struct AudioStream {
    AudioCallback callback;
    void*         user;

    uint32_t    sampleRate;
    uint8_t     channels;
    AudioFormat format;

    volatile AudioState state;
};

class AudioSource {
  public:
    virtual ~AudioSource() = default;

    virtual void init()                    = 0;
    virtual void play(AudioStream* stream) = 0;
    virtual void stop()                    = 0;
    virtual void setVolume(float volume)   = 0; // 0.0–1.0
};

class NullAudio : public AudioSource {
  public:
    void init() override {}
    void play(AudioStream* stream) override { (void)stream; }
    void stop() override {}
    void setVolume(float volume) override { (void)volume; }
};