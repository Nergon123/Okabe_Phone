#include "MP3Player.h"

#include <signal.h>
#include <string.h>

#define MINIMP3_IMPLEMENTATION
#include <Platform/Audio/minimp3.h>

static const char* TAG = "MP3Player";

MP3Player::MP3Player(AudioSource* a)
    : audio(a)
{
}

bool MP3Player::init(const NString& file_path)
{
    static_assert(
        sizeof(pcmCache) ==
            MINIMP3_MAX_SAMPLES_PER_FRAME * sizeof(int16_t),
        "pcmCache size mismatch!"
    );

    ESP_LOGI(TAG, "MP3 buffer: %zu bytes", sizeof(mp3buf));
    ESP_LOGI(
        TAG,
        "PCM cache: %zu bytes (%d int16 samples)",
        sizeof(pcmCache),
        MINIMP3_MAX_SAMPLES_PER_FRAME
    );

    ESP_LOGI(TAG, "init: %s", file_path.c_str());

    currentFile = VFS.open(file_path.c_str(), "r");

    if (!currentFile)
    {
        ESP_LOGE(TAG, "Failed to open file");
        return false;
    }

    fileSizeBytes = currentFile->size();

    /*
     * Probe the first valid MP3 frame to discover:
     *
     *   - sample rate
     *   - channel count
     *   - bitrate
     *
     * This decode is NOT part of actual playback.
     */
    mp3dec_init(&mp3d);

    mp3bufStart = 0;
    mp3bufFill = currentFile->read(mp3buf, sizeof(mp3buf));

    ESP_LOGI(
        TAG,
        "Read %zu bytes for initial decode",
        mp3bufFill
    );

    if (mp3bufFill == 0)
    {
        ESP_LOGE(TAG, "Empty file");
        return false;
    }

    mp3dec_frame_info_t info{};
    int16_t probePcm[MINIMP3_MAX_SAMPLES_PER_FRAME];

    int samples = mp3dec_decode_frame(
        &mp3d,
        mp3buf,
        (int)mp3bufFill,
        probePcm,
        &info
    );

    ESP_LOGI(
        TAG,
        "Initial decode: samples=%d, hz=%d, channels=%d, "
        "frame_bytes=%d, bitrate=%d, layer=%d",
        samples,
        info.hz,
        info.channels,
        info.frame_bytes,
        info.bitrate_kbps,
        info.layer
    );

    /*
     * MP3 files can contain ID3 data or garbage before the first frame.
     * If the initial decode fails, search a small range for sync.
     */
    if (samples <= 0 || info.frame_bytes <= 0)
    {
        ESP_LOGW(TAG, "Initial frame invalid, searching for MP3 sync");

        bool found = false;

        for (size_t offset = 1;
             offset < 100 && offset < mp3bufFill;
             ++offset)
        {
            mp3dec_init(&mp3d);

            mp3dec_frame_info_t testInfo{};

            samples = mp3dec_decode_frame(
                &mp3d,
                mp3buf + offset,
                (int)(mp3bufFill - offset),
                probePcm,
                &testInfo
            );

            if (samples > 0 &&
                testInfo.frame_bytes > 0 &&
                testInfo.hz > 0 &&
                testInfo.channels > 0)
            {
                info = testInfo;

                ESP_LOGI(
                    TAG,
                    "Found valid MP3 frame after skipping %zu bytes",
                    offset
                );

                found = true;
                break;
            }
        }

        if (!found)
        {
            ESP_LOGE(TAG, "Could not locate valid MP3 frame");
            return false;
        }
    }

    sampleRate = info.hz;
    channels = info.channels;

    if (info.bitrate_kbps > 0)
        lastBitrate = info.bitrate_kbps;

    ESP_LOGI(
        TAG,
        "Audio info: %d Hz, %d channels",
        sampleRate,
        channels
    );

    const size_t pcmCacheBytes = sizeof(pcmCache);
    const size_t pcmCacheSamples =
        pcmCacheBytes / sizeof(int16_t);

    ESP_LOGI(
        TAG,
        "PCM cache: %zu bytes, %zu int16 samples",
        pcmCacheBytes,
        pcmCacheSamples
    );

    /*
     * IMPORTANT:
     *
     * We decoded one frame above only for metadata.
     *
     * MP3 Layer III has decoder state between frames, therefore we MUST
     * reset minimp3 before rewinding and beginning real playback.
     */
    mp3dec_init(&mp3d);

    currentFile->seek(0);

    mp3bufStart = 0;
    mp3bufFill = 0;

    pcmPos = 0;
    pcmFill = 0;

    eof = false;
    file_eof = false;

    framesDecoded = 0;
    loopIterations = 0;
    totalSamplesOutput = 0;

    /*
     * Configure AudioStream.
     *
     * minimp3 output is signed 16-bit PCM.
     */
    stream.callback = &MP3Player::audioCallback;
    stream.user = this;
    stream.sampleRate = sampleRate;
    stream.channels = channels;
    stream.format = AUDIOFMT_S16;
    stream.state = AUDIO_STOPPED;

    ESP_LOGI(
        TAG,
        "Guards addr: "
        "&guard_mp3buf_start=%p "
        "&mp3buf=%p "
        "&guard_mp3buf_end=%p "
        "&guard_pcm_start=%p "
        "&pcmCache=%p "
        "&guard_pcm_end=%p",
        (void*)&guard_mp3buf_start,
        (void*)mp3buf,
        (void*)&guard_mp3buf_end,
        (void*)&guard_pcm_start,
        (void*)pcmCache,
        (void*)&guard_pcm_end
    );

    ESP_LOGI(
        TAG,
        "Guards val: start=%08x end=%08x "
        "pcm_start=%08x pcm_end=%08x",
        guard_mp3buf_start,
        guard_mp3buf_end,
        guard_pcm_start,
        guard_pcm_end
    );

    ESP_LOGI(TAG, "MP3Player initialized successfully");

    return true;
}

void MP3Player::play()
{
    ESP_LOGI(
        TAG,
        "play() called, current state: %d",
        stream.state
    );

    if (stream.state == AUDIO_PLAYING)
    {
        ESP_LOGW(TAG, "Already playing");
        return;
    }

    if (!audio)
    {
        ESP_LOGE(TAG, "AudioSource is null");
        return;
    }

    stream.state = AUDIO_PLAYING;

    ESP_LOGI(TAG, "Starting audio playback");
    ESP_LOGI(TAG, "Audio object pointer is %p", audio);

    audio->play(&stream);
}

void MP3Player::stop()
{
    stream.state = AUDIO_STOPPED;

    if (audio)
        audio->stop();
}

size_t MP3Player::audioCallback(
    void* user,
    void* out,
    size_t bytes
)
{
    if (!user || !out || bytes == 0)
        return 0;

    return static_cast<MP3Player*>(user)->onAudio(
        out,
        bytes
    );
}

size_t MP3Player::onAudio(
    void* out,
    size_t bytes
)
{
    uint8_t* dst = static_cast<uint8_t*>(out);
    size_t done = 0;

    if (!dst || bytes == 0)
        return 0;

    /*
     * Loop handling.
     */
    if (eof)
    {
        if (loop)
        {
            ESP_LOGI(TAG, "Looping MP3");

            setTimeMs(0);

            eof = false;
            file_eof = false;
        }
        else
        {
            memset(dst, 0, bytes);
            return bytes;
        }
    }

    if (stream.state != AUDIO_PLAYING)
    {
        memset(dst, 0, bytes);
        return bytes;
    }

    const size_t mp3BufCap = sizeof(mp3buf);

    const size_t pcmCacheBytes = sizeof(pcmCache);
    const size_t pcmCacheSamples =
        pcmCacheBytes / sizeof(int16_t);

    /*
     * Basic corruption checks.
     */
    if (mp3bufFill > mp3BufCap)
    {
        ESP_LOGE(
            TAG,
            "mp3bufFill corrupted: %zu > %zu",
            mp3bufFill,
            mp3BufCap
        );

        eof = true;
        memset(dst, 0, bytes);

        return bytes;
    }

    if (pcmFill > pcmCacheBytes ||
        pcmPos > pcmFill)
    {
        ESP_LOGE(
            TAG,
            "PCM state corrupted: pos=%zu fill=%zu cap=%zu",
            pcmPos,
            pcmFill,
            pcmCacheBytes
        );

        pcmPos = 0;
        pcmFill = 0;
        eof = true;

        memset(dst, 0, bytes);

        return bytes;
    }

    ++loopIterations;

    while (done < bytes)
    {
        /*
         * --------------------------------------------------------
         * 1. Drain already-decoded PCM.
         * --------------------------------------------------------
         */
        if (pcmPos < pcmFill)
        {
            size_t available = pcmFill - pcmPos;
            size_t wanted = bytes - done;

            size_t toCopy =
                (available < wanted)
                    ? available
                    : wanted;

            if (pcmPos + toCopy > pcmCacheBytes)
            {
                ESP_LOGE(
                    TAG,
                    "PCM copy overflow: pos=%zu copy=%zu cap=%zu",
                    pcmPos,
                    toCopy,
                    pcmCacheBytes
                );

                eof = true;
                break;
            }

            memcpy(
                dst + done,
                reinterpret_cast<uint8_t*>(pcmCache) + pcmPos,
                toCopy
            );

            pcmPos += toCopy;
            done += toCopy;

            continue;
        }

        /*
         * PCM cache exhausted.
         */
        pcmPos = 0;
        pcmFill = 0;

        /*
         * --------------------------------------------------------
         * 2. Fill compressed MP3 circular buffer.
         * --------------------------------------------------------
         */
        if (mp3bufFill < mp3BufCap && !file_eof)
        {
            size_t freeBytes = mp3BufCap - mp3bufFill;

            size_t writePos =
                (mp3bufStart + mp3bufFill) % mp3BufCap;

            size_t firstCapacity =
                mp3BufCap - writePos;

            size_t firstRead =
                (freeBytes < firstCapacity)
                    ? freeBytes
                    : firstCapacity;

            size_t r1 = 0;
            size_t r2 = 0;

            if (firstRead > 0)
            {
                r1 = currentFile->read(
                    mp3buf + writePos,
                    firstRead
                );

                mp3bufFill += r1;
            }

            /*
             * If the free portion wraps to the beginning,
             * fill the second part.
             */
            size_t stillFree =
                mp3BufCap - mp3bufFill;

            if (r1 == firstRead &&
                stillFree > 0 &&
                writePos + firstRead >= mp3BufCap)
            {
                r2 = currentFile->read(
                    mp3buf,
                    stillFree
                );

                mp3bufFill += r2;
            }

            if (r1 == 0 && r2 == 0)
            {
                file_eof = true;
            }
        }

        /*
         * Nothing compressed remains.
         */
        if (mp3bufFill == 0)
        {
            if (file_eof)
                eof = true;

            break;
        }

        /*
         * --------------------------------------------------------
         * 3. Present contiguous MP3 data to minimp3.
         * --------------------------------------------------------
         */
        mp3dec_frame_info_t info{};

        uint8_t* decodePtr = nullptr;
        size_t decodeBytes = mp3bufFill;

        /*
         * minimp3 expects contiguous input.
         *
         * If the circular buffer wraps, reconstruct it into tmp.
         */
        static uint8_t tmp[MP3_BUF_SIZE];

        if (mp3bufStart + mp3bufFill <= mp3BufCap)
        {
            decodePtr = mp3buf + mp3bufStart;
        }
        else
        {
            size_t first =
                mp3BufCap - mp3bufStart;

            size_t second =
                mp3bufFill - first;

            memcpy(
                tmp,
                mp3buf + mp3bufStart,
                first
            );

            memcpy(
                tmp + first,
                mp3buf,
                second
            );

            decodePtr = tmp;
        }

        /*
         * Local decoder output plus overflow guard.
         */
        int16_t localPcm[
            MINIMP3_MAX_SAMPLES_PER_FRAME + 4
        ];

        uint32_t* localGuard =
            reinterpret_cast<uint32_t*>(
                localPcm +
                MINIMP3_MAX_SAMPLES_PER_FRAME
            );

        constexpr uint32_t LOCAL_GUARD =
            0xDEADBEEF;

        *localGuard = LOCAL_GUARD;

        int samples = mp3dec_decode_frame(
            &mp3d,
            decodePtr,
            (int)decodeBytes,
            localPcm,
            &info
        );

        if (*localGuard != LOCAL_GUARD)
        {
            ESP_LOGE(
                TAG,
                "minimp3 overflow: local guard overwritten"
            );

            eof = true;
            break;
        }

        /*
         * Useful startup diagnostics.
         */
        if (framesDecoded < 30)
        {
            ESP_LOGI(
                TAG,
                "DEC #%zu: samples=%d frame=%d "
                "hz=%d ch=%d start=%zu fill=%zu input=%zu",
                framesDecoded,
                samples,
                info.frame_bytes,
                info.hz,
                info.channels,
                mp3bufStart,
                mp3bufFill,
                decodeBytes
            );
        }

        /*
         * --------------------------------------------------------
         * 4. Successful MP3 frame.
         * --------------------------------------------------------
         */
        if (samples > 0)
        {
            int frameChannels =
                (info.channels > 0)
                    ? info.channels
                    : channels;

            if (frameChannels <= 0)
            {
                ESP_LOGE(TAG, "Decoder returned invalid channel count");
                eof = true;
                break;
            }

            /*
             * minimp3 returns number of samples PER CHANNEL.
             *
             * Stereo:
             *
             *     1152 * 2 = 2304 int16 values
             */
            size_t totalSamples =
                static_cast<size_t>(samples) *
                static_cast<size_t>(frameChannels);

            if (totalSamples > pcmCacheSamples)
            {
                ESP_LOGE(
                    TAG,
                    "Decoded frame too large: "
                    "%zu samples > %zu",
                    totalSamples,
                    pcmCacheSamples
                );

                eof = true;
                break;
            }

            size_t pcmBytes =
                totalSamples * sizeof(int16_t);

            memcpy(
                pcmCache,
                localPcm,
                pcmBytes
            );

            pcmPos = 0;
            pcmFill = pcmBytes;

            ++framesDecoded;

            if (info.bitrate_kbps > 0)
                lastBitrate = info.bitrate_kbps;

            if (info.hz > 0)
                sampleRate = info.hz;

            if (info.channels > 0)
                channels = info.channels;

            /*
             * Consume exactly the compressed bytes belonging
             * to the decoded frame.
             */
            if (info.frame_bytes > 0)
            {
                size_t consumed =
                    static_cast<size_t>(
                        info.frame_bytes
                    );

                if (consumed > mp3bufFill)
                {
                    ESP_LOGE(
                        TAG,
                        "Decoder consumed beyond input: "
                        "%zu > %zu",
                        consumed,
                        mp3bufFill
                    );

                    eof = true;
                    break;
                }

                mp3bufStart =
                    (mp3bufStart + consumed)
                    % mp3BufCap;

                mp3bufFill -= consumed;
            }

            /*
             * Verify member guards.
             */
            if (
                guard_mp3buf_start != 0xC0FFEE11 ||
                guard_mp3buf_end   != 0xB16B00B5 ||
                guard_pcm_start    != 0xABCD1234 ||
                guard_pcm_end      != 0x1234ABCD
            )
            {
                ESP_LOGE(
                    TAG,
                    "Memory guard corrupted: "
                    "%08x %08x %08x %08x",
                    guard_mp3buf_start,
                    guard_mp3buf_end,
                    guard_pcm_start,
                    guard_pcm_end
                );

                raise(SIGTRAP);
                abort();
            }

            /*
             * Loop again.
             *
             * Next iteration drains pcmCache into dst.
             */
            continue;
        }

        /*
         * --------------------------------------------------------
         * 5. Decoder produced no PCM.
         * --------------------------------------------------------
         *
         * IMPORTANT:
         *
         * mp3buf is circular. Never memmove() from mp3buf[0]
         * here because mp3bufStart may not be zero.
         */
        if (
            info.frame_bytes > 0 &&
            static_cast<size_t>(info.frame_bytes)
                <= mp3bufFill
        )
        {
            size_t skipped =
                static_cast<size_t>(
                    info.frame_bytes
                );

            mp3bufStart =
                (mp3bufStart + skipped)
                % mp3BufCap;

            mp3bufFill -= skipped;

            ESP_LOGW(
                TAG,
                "Decoder produced no PCM; "
                "skipped %zu bytes, start=%zu fill=%zu",
                skipped,
                mp3bufStart,
                mp3bufFill
            );

            continue;
        }

        /*
         * Decoder needs more compressed input.
         *
         * If the file has already ended, we're finished.
         */
        if (file_eof)
        {
            eof = true;
            break;
        }

        /*
         * There is no progress possible if:
         *
         *  - buffer is full
         *  - decoder consumed nothing
         *
         * Prevent an infinite loop.
         */
        if (mp3bufFill == mp3BufCap)
        {
            ESP_LOGW(
                TAG,
                "Decoder made no progress with full MP3 buffer; "
                "dropping one byte for resync"
            );

            mp3bufStart =
                (mp3bufStart + 1)
                % mp3BufCap;

            --mp3bufFill;
        }
    }

    /*
     * AudioSource expects the entire requested output buffer.
     * Fill any remainder with silence.
     */
    if (done < bytes)
    {
        memset(
            dst + done,
            0,
            bytes - done
        );
    }

    /*
     * totalSamplesOutput counts interleaved int16 values:
     *
     * stereo:
     *
     *     L R L R L R ...
     *
     * getTimeMs() therefore divides this by channels.
     */
    totalSamplesOutput +=
        done / sizeof(int16_t);

    return bytes;
}

uint32_t MP3Player::getTimeMs()
{
    if (sampleRate == 0 ||
        channels == 0)
    {
        return 0;
    }

    uint64_t denominator =
        static_cast<uint64_t>(sampleRate) *
        static_cast<uint64_t>(channels);

    uint64_t timeMs =
        (totalSamplesOutput * 1000ULL) /
        denominator;

    if (timeMs > 0xFFFFFFFFULL)
        return 0xFFFFFFFF;

    return static_cast<uint32_t>(timeMs);
}

void MP3Player::setTimeMs(uint32_t timeMs)
{
    if (!currentFile ||
        sampleRate == 0 ||
        channels == 0)
    {
        ESP_LOGW(
            TAG,
            "setTimeMs: player not initialized"
        );

        return;
    }

    ESP_LOGI(
        TAG,
        "Seeking to %u ms",
        timeMs
    );

    /*
     * Estimate byte position from bitrate.
     *
     * bytes =
     *
     *   milliseconds * kbps / 8
     */
    uint64_t estimated =
        static_cast<uint64_t>(timeMs) *
        static_cast<uint64_t>(lastBitrate) /
        8ULL;

    if (estimated > fileSizeBytes)
        estimated = fileSizeBytes;

    currentFile->seek(
        static_cast<uint32_t>(estimated),
        SEEK_SET
    );

    /*
     * Seeking destroys MP3 Layer III decoder history.
     */
    mp3dec_init(&mp3d);

    mp3bufStart = 0;
    mp3bufFill = 0;

    pcmPos = 0;
    pcmFill = 0;

    file_eof = false;
    eof = false;

    /*
     * Make reported time immediately reflect the target.
     */
    totalSamplesOutput =
        (
            static_cast<uint64_t>(timeMs) *
            sampleRate *
            channels
        ) / 1000ULL;

    /*
     * Prime compressed buffer at new position.
     */
    mp3bufFill =
        currentFile->read(
            mp3buf,
            sizeof(mp3buf)
        );

    if (mp3bufFill == 0)
    {
        ESP_LOGE(
            TAG,
            "setTimeMs: unable to read after seek"
        );

        eof = true;
        return;
    }

    /*
     * The bitrate-based seek may land in the middle of a frame.
     * onAudio() will resynchronize by skipping invalid bytes.
     */
    ESP_LOGI(
        TAG,
        "Sought to file position %llu, "
        "read %zu bytes",
        static_cast<unsigned long long>(
            estimated
        ),
        mp3bufFill
    );
}