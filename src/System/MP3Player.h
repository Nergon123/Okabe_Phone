#pragma once

#include <Platform/FileSystem/VFS.h>
#include <Platform/NString.h>
#include <stddef.h>
#include <stdint.h>

extern "C" {
#include <Platform/Audio/minimp3.h>
}

#include <Platform/Audio/Audio.h>

class MP3Player {
  public:
    explicit MP3Player(Audio* audio);

    bool init(const NString& file_path);
    void play();
    void stop();

  private:
    static size_t audioCallback(void* user, void* out, size_t bytes);
    size_t        onAudio(void* out, size_t bytes);

  private:
    Audio*      audio;
    AudioStream stream{};

    mp3dec_t mp3d;
    NFile*   currentFile = nullptr;

    int sampleRate = 0;
    int channels   = 0;

    bool eof      = false;
    bool file_eof = false; // Track if we've reached end of file

    // decode scratch buffers (owned by player, reused)
    // Larger input buffer: docs recommend ~16KB for reliable sync (10 consecutive frames)
    static constexpr size_t MP3_BUF_SIZE       = 16384;
    uint32_t                guard_mp3buf_start = 0xC0FFEE11; // canary before mp3buf
    uint8_t                 mp3buf[MP3_BUF_SIZE];
    uint32_t                guard_mp3buf_end = 0xB16B00B5; // canary after mp3buf
    size_t                  mp3bufFill       = 0;
    size_t                  mp3bufStart      = 0; // circular buffer start index (bytes)

    // PCM cache for one frame
    // Single frame max: MINIMP3_MAX_SAMPLES_PER_FRAME (2304) * sizeof(int16_t)
    // IMPORTANT: mp3dec_decode_frame returns the number of samples PER CHANNEL.
    // Total int16_t samples = samples_returned * channels.
    static constexpr size_t PCM_CACHE_SIZE  = MINIMP3_MAX_SAMPLES_PER_FRAME * sizeof(int16_t);
    uint32_t                guard_pcm_start = 0xABCD1234; // canary before PCM cache
    int16_t                 pcmCache[MINIMP3_MAX_SAMPLES_PER_FRAME];
    uint32_t                guard_pcm_end = 0x1234ABCD; // canary after PCM cache
    size_t                  pcmPos        = 0;          // bytes copied
    size_t                  pcmFill       = 0;          // bytes available

    // Diagnostic counters
    size_t framesDecoded  = 0; // incremented on each successful decode
    size_t loopIterations = 0; // incremented every onAudio loop iteration (for diagnostics)
};
