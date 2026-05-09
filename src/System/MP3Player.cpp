#include "MP3Player.h"
#include <signal.h>
#include <string.h>
#define MINIMP3_IMPLEMENTATION
#include <Platform/Audio/minimp3.h>
MP3Player::MP3Player(AudioSource* a) : audio(a) {}
static const char* TAG = "MP3Player";
bool               MP3Player::init(const NString& file_path) {
    // Add compile-time size check
    static_assert(sizeof(pcmCache) == MINIMP3_MAX_SAMPLES_PER_FRAME * sizeof(int16_t),
                  "pcmCache size mismatch!");

    // Add runtime debug
    ESP_LOGI(TAG, "MP3 buffer: %zu bytes", sizeof(mp3buf));
    ESP_LOGI(TAG, "PCM cache: %zu bytes (max %d samples per frame)", sizeof(pcmCache),
             MINIMP3_MAX_SAMPLES_PER_FRAME);
    ESP_LOGI(TAG, "init: %s", file_path.c_str());
    mp3dec_init(&mp3d);

    currentFile = VFS.open(file_path.c_str(), "r");
    if (!currentFile) {
        ESP_LOGE(TAG, "Failed to open file");
        return false;
    }

    // Get file size for duration estimation
    fileSizeBytes = currentFile->size();
    mp3bufFill    = currentFile->read(mp3buf, sizeof(mp3buf));
    ESP_LOGI(TAG, "Read %zu bytes for initial decode", mp3bufFill);

    if (mp3bufFill == 0) {
        ESP_LOGE(TAG, "Empty file");
        return false;
    }

    mp3dec_frame_info_t info;
    int16_t             pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];

    // Test decode with the actual buffer we'll use
    int samples = mp3dec_decode_frame(&mp3d, mp3buf, mp3bufFill, pcm, &info);
    ESP_LOGI(
        TAG,
        "Initial decode: samples=%d, hz=%d, channels=%d, frame_bytes=%d, bitrate=%d, layer=%d",
        samples, info.hz, info.channels, info.frame_bytes, info.bitrate_kbps, info.layer);

    if (samples <= 0 || info.frame_bytes == 0) {
        ESP_LOGE(TAG, "Failed to decode initial frame");
        // Try to resync and find a valid frame
        if (mp3bufFill > 100) {
            ESP_LOGW(TAG, "Trying to find valid frame by skipping bytes...");
            for (int i = 1; i < 100; i++) {
                samples = mp3dec_decode_frame(&mp3d, mp3buf + i, mp3bufFill - i, pcm, &info);
                if (samples > 0) {
                    ESP_LOGI(TAG, "Found valid frame after skipping %d bytes", i);
                    break;
                }
            }
        }
        if (samples <= 0) { return false; }
    }

    sampleRate = info.hz;
    channels   = info.channels;

    // Important: Calculate PCM cache size correctly
    size_t pcm_cache_size_bytes   = sizeof(pcmCache);
    size_t pcm_cache_size_samples = pcm_cache_size_bytes / sizeof(int16_t);

    ESP_LOGI(TAG, "Audio info: %d Hz, %d channels", sampleRate, channels);
    ESP_LOGI(TAG, "PCM cache: %zu bytes, %zu samples (max %zu frames)", pcm_cache_size_bytes,
             pcm_cache_size_samples, pcm_cache_size_samples / (MINIMP3_MAX_SAMPLES_PER_FRAME * 2));
    ESP_LOGI(TAG, "MINIMP3_MAX_SAMPLES_PER_FRAME = %d", MINIMP3_MAX_SAMPLES_PER_FRAME);

    // Reset file position to beginning
    currentFile->seek(0);

    // Clear the MP3 buffer - we'll fill it fresh during playback
    mp3bufFill = 0;

    // Print addresses & initial values of guards to make it easy to set watchpoints in gdb
    ESP_LOGI(TAG,
             "Guards addr: &guard_mp3buf_start=%p &mp3buf=%p &guard_mp3buf_end=%p "
             "&guard_pcm_start=%p &pcmCache=%p &guard_pcm_end=%p",
             (void*)&guard_mp3buf_start, (void*)mp3buf, (void*)&guard_mp3buf_end,
             (void*)&guard_pcm_start, (void*)pcmCache, (void*)&guard_pcm_end);
    ESP_LOGI(TAG, "Guards val: start=%08x end=%08x pcm_start=%08x pcm_end=%08x",
             guard_mp3buf_start, guard_mp3buf_end, guard_pcm_start, guard_pcm_end);

    stream.callback   = &MP3Player::audioCallback;
    stream.user       = this;
    stream.sampleRate = sampleRate;
    stream.channels   = channels;
    stream.format     = AUDIOFMT_S16;
    stream.state      = AUDIO_STOPPED;

    pcmFill  = 0;
    pcmPos   = 0;
    eof      = false;
    file_eof = false;

    // Reset position tracking
    totalSamplesOutput = 0;

    ESP_LOGI(TAG, "MP3Player initialized successfully");
    return true;
}
void MP3Player::play() {
    ESP_LOGI(TAG, "play() called, current state: %d", stream.state);
    if (stream.state == AUDIO_PLAYING) {
        ESP_LOGW(TAG, "Already playing");
        return;
    }

    stream.state = AUDIO_PLAYING;
    ESP_LOGI(TAG, "Starting audio playback");
    ESP_LOGI(TAG, "Audio object pointer is %p", audio);
    audio->play(&stream);
}

void MP3Player::stop() {
    stream.state = AUDIO_STOPPED;
    if (audio) { audio->stop(); }
}

size_t MP3Player::audioCallback(void* user, void* out, size_t bytes) {
    return static_cast<MP3Player*>(user)->onAudio(out, bytes);
}

size_t MP3Player::onAudio(void* out, size_t bytes) {
    uint8_t* dst  = static_cast<uint8_t*>(out);
    size_t   done = 0;

    // ESP_LOGV(TAG, "onAudio: requested %zu bytes, state=%d, eof=%d, file_eof=%d", bytes,
    //          stream.state, eof, file_eof);
    if (eof) {
        memset(dst, 0, bytes);
        if (loop) {
            eof      = 0;
            file_eof = 0;
            setTimeMs(0);
            return bytes;
        }

        return bytes;
    }
    // If not playing or already at EOF, fill with silence
    if (stream.state != AUDIO_PLAYING || eof) {
        memset(dst, 0, bytes);
        return bytes;
    }

    const size_t mp3BufCap       = sizeof(mp3buf);
    const size_t pcmCacheBytes   = sizeof(pcmCache); // bytes
    const size_t pcmCacheSamples = pcmCacheBytes / sizeof(int16_t);

    // Sanity checks before we start the loop
    if (mp3bufFill > mp3BufCap) {
        ESP_LOGE(TAG, "ERROR: mp3bufFill corrupted (%zu > %zu)", mp3bufFill, mp3BufCap);
        mp3bufFill = 0;
        eof        = true;
    }
    if (pcmFill > pcmCacheBytes) {
        ESP_LOGE(TAG, "ERROR: pcmFill corrupted on entry (%zu > %zu)", pcmFill, pcmCacheBytes);
        pcmFill = 0;
        pcmPos  = 0;
        eof     = true;
    }

    // Log addresses and initial canary state (helps root-cause memory layout corruption)
    loopIterations++;
    // ESP_LOGV(TAG,
    //          "onAudio loop %zu: addr:this=%p, &mp3buf=%p, &mp3bufFill=%p, &guard_mp3buf_end=%p,
    //          "
    //          "&pcmCache=%p, "
    //          "&guard_pcm_end=%p, &pcmFill=%p",
    //          loopIterations, (void*)this, (void*)&mp3buf, (void*)&mp3bufFill,
    //          (void*)&guard_mp3buf_end, (void*)&pcmCache, (void*)&guard_pcm_end,
    //          (void*)&pcmFill);
    // ESP_LOGV(TAG, "canaries: start=%08x, mp3_end=%08x, pcm_start=%08x, pcm_end=%08x",
    //          guard_mp3buf_start, guard_mp3buf_end, guard_pcm_start, guard_pcm_end);

    while (done < bytes) {
        // 1) Copy leftover PCM from cache
        if (pcmPos < pcmFill) {
            if (pcmFill > pcmCacheBytes) {
                ESP_LOGE(TAG, "ERROR: pcmFill corrupted (%zu > %zu)", pcmFill, pcmCacheBytes);
                eof = true;
                break;
            }

            size_t available = pcmFill - pcmPos;
            size_t to_copy   = (bytes - done < available) ? (bytes - done) : available;

            if (pcmPos + to_copy > pcmCacheBytes) {
                ESP_LOGE(TAG,
                         "ERROR: copy would exceed PCM cache bounds: pos=%zu + copy=%zu > %zu",
                         pcmPos, to_copy, pcmCacheBytes);
                eof = true;
                break;
            }

            memcpy(dst + done, (uint8_t*)pcmCache + pcmPos, to_copy);
            pcmPos += to_copy;
            done += to_copy;
            continue;
        }

        // 2) No cached PCM available, reset cache pointers
        pcmPos  = 0;
        pcmFill = 0;

        // 3) Fill mp3 input circular buffer if possible (handle wrap)
        if (mp3bufFill < mp3BufCap && !file_eof) {
            size_t write_pos  = (mp3bufStart + mp3bufFill) % mp3BufCap;
            size_t max_contig = mp3BufCap - write_pos;
            size_t to_read    = mp3BufCap - mp3bufFill; // total we want

            // First read into the tail part
            size_t r1 = currentFile->read(mp3buf + write_pos,
                                          (to_read < max_contig) ? to_read : max_contig);
            if (r1 > max_contig) {
                ESP_LOGE(TAG, "read overflow r1>max_contig");
                eof = true;
                break;
            }
            mp3bufFill += r1;

            // If we requested more and wrapped, read into beginning
            size_t remaining_to_read = to_read - r1;
            if (remaining_to_read > 0) {
                size_t r2 = currentFile->read(mp3buf, remaining_to_read);
                if (r2 > remaining_to_read) {
                    ESP_LOGE(TAG, "read overflow r2>remaining");
                    eof = true;
                    break;
                }
                mp3bufFill += r2;
                if (r1 + r2 == 0) {
                    file_eof = true;
                    ESP_LOGV(TAG, "File EOF reached");
                }
            }
            else if (r1 == 0) {
                file_eof = true;
                ESP_LOGV(TAG, "File EOF reached");
            }

            //    ESP_LOGV(TAG, "Read r1=%zu r2=%zu, mp3bufFill=%zu (start=%zu)", r1,
            //             (size_t)remaining_to_read, mp3bufFill, mp3bufStart);
        }

        // 4) If we still have no MP3 data, break and pad silence
        if (mp3bufFill == 0) {
            if (file_eof) { eof = true; }
            break;
        }

        // 5) Prepare a contiguous input slice for decoder (handle circular buffer)
        mp3dec_frame_info_t info{};
        size_t              contiguous_bytes = 0;
        uint8_t*            decode_ptr       = nullptr;
        // If data is contiguous starting at mp3bufStart
        if (mp3bufStart + mp3bufFill <= mp3BufCap) {
            decode_ptr       = mp3buf + mp3bufStart;
            contiguous_bytes = mp3bufFill;
        }
        else {
            // Need to assemble into a temporary buffer
            contiguous_bytes = mp3bufFill;
            static uint8_t tmp[MP3_BUF_SIZE];
            size_t         first = mp3BufCap - mp3bufStart;
            memcpy(tmp, mp3buf + mp3bufStart, first);
            memcpy(tmp + first, mp3buf, mp3bufFill - first);
            decode_ptr = tmp;
        }

        /* Guarded local PCM buffer: guard placed after decoded samples to detect overruns */
        int16_t   local_pcm[MINIMP3_MAX_SAMPLES_PER_FRAME + 4];
        uint32_t* guard = reinterpret_cast<uint32_t*>(local_pcm + MINIMP3_MAX_SAMPLES_PER_FRAME);
        const uint32_t GUARD_MAGIC = 0xDEADBEEF;
        *guard                     = GUARD_MAGIC;

        int samples = mp3dec_decode_frame(&mp3d, decode_ptr, contiguous_bytes, local_pcm, &info);
        //  ESP_LOGV(TAG, "mp3 decode: samples=%d, frame_bytes=%d, hz=%d, ch=%d (start=%zu
        //  fill=%zu)",
        //           samples, info.frame_bytes, info.hz, info.channels, mp3bufStart, mp3bufFill);

        // Check guard to ensure decoder did not write beyond local_pcm buffer
        if (*guard != GUARD_MAGIC) {
            ESP_LOGE(TAG, "Decoder overflow detected: guard overwritten (0x%08x)", *guard);
            eof = true;
            break;
        }

        if (samples > 0) {
            int    ch            = (info.channels > 0) ? info.channels : channels;
            size_t total_samples = (size_t)samples * (size_t)ch; // int16_t samples total

            if (total_samples > pcmCacheSamples) {
                ESP_LOGE(TAG, "Decoded samples (%zu) exceed cache samples (%zu)", total_samples,
                         pcmCacheSamples);
                eof = true;
                break;
            }

            size_t pcm_bytes = total_samples * sizeof(int16_t);
            if (pcm_bytes == 0) { continue; }

            // Copy decoded PCM from guarded local buffer into the member PCM cache
            {
                // Copy samples safely (per-int16_t), and never overwrite the PCM guard.
                size_t samples_to_copy = total_samples;
                if (samples_to_copy > pcmCacheSamples) {
                    ESP_LOGE(TAG, "Truncating decoded samples from %zu to %zu to avoid overflow",
                             samples_to_copy, pcmCacheSamples);
                    samples_to_copy = pcmCacheSamples;
                }

                // Perform per-sample copy (safe) rather than memcpy to avoid any edge-case vector
                // overshoot
                for (size_t s = 0; s < samples_to_copy; ++s) { pcmCache[s] = local_pcm[s]; }

                if (samples_to_copy != total_samples) {
                    ESP_LOGE(TAG, "Decoded frame too large, dropped %zu samples",
                             total_samples - samples_to_copy);
                    eof = true; // abort playback to prevent repeated corruption
                    break;
                }

                pcmFill = samples_to_copy * sizeof(int16_t); // bytes available in cache
                pcmPos  = 0;
            }

            // Increment decoded-frame counter and occasionally log progress
            framesDecoded++;
            if ((framesDecoded & 0x3FF) == 0) { // every 1024 frames
                ESP_LOGI(TAG, "Decoded frames=%zu, sampleRate=%d, channels=%d, mp3bufFill=%zu",
                         framesDecoded, sampleRate, channels, mp3bufFill);
            }

            // Update bitrate estimate from frame header (helps with seeking accuracy)
            if (info.bitrate_kbps > 0) { lastBitrate = info.bitrate_kbps; }

            // Validate canaries after copying PCM
            if (guard_mp3buf_start != 0xC0FFEE11 || guard_mp3buf_end != 0xB16B00B5 ||
                guard_pcm_start != 0xABCD1234 || guard_pcm_end != 0x1234ABCD) {
                ESP_LOGE(TAG,
                         "Canary corrupted AFTER memcpy: start=%08x, mp3_end=%08x, "
                         "pcm_start=%08x, pcm_end=%08x",
                         guard_mp3buf_start, guard_mp3buf_end, guard_pcm_start, guard_pcm_end);
                // Dump nearby memory to help root cause
                size_t   dumpStart = reinterpret_cast<uintptr_t>(&guard_mp3buf_start);
                uint8_t* p         = reinterpret_cast<uint8_t*>(dumpStart);
                ESP_LOGE(TAG, "Memory dump around guards (32 bytes):");
                for (size_t i = 0; i < 32; ++i) { ESP_LOGE(TAG, "%02x", p[i]); }
                ESP_LOGE(TAG, "loopIterations=%zu framesDecoded=%zu mp3bufFill=%zu pcmFill=%zu",
                         loopIterations, framesDecoded, mp3bufFill, pcmFill);
                // Break immediately so debugger can catch the exact moment of corruption
                raise(SIGTRAP);
                abort();
                break;
            }

            // Consume the mp3 input bytes for this frame
            if (info.frame_bytes > 0) {
                if ((size_t)info.frame_bytes > mp3bufFill) {
                    ESP_LOGE(TAG, "FATAL: frame_bytes=%d > mp3bufFill=%zu", info.frame_bytes,
                             mp3bufFill);
                    eof = true;
                    break;
                }

                // Advance the circular buffer start by frame_bytes instead of memmoving memory
                mp3bufStart = (mp3bufStart + (size_t)info.frame_bytes) % mp3BufCap;
                mp3bufFill -= (size_t)info.frame_bytes;

                // Validate canaries immediately after index update
                if (guard_mp3buf_start != 0xC0FFEE11 || guard_mp3buf_end != 0xB16B00B5 ||
                    guard_pcm_start != 0xABCD1234 || guard_pcm_end != 0x1234ABCD) {
                    ESP_LOGE(TAG,
                             "Canary corrupted AFTER advancing circular start: start=%08x, "
                             "mp3_end=%08x, "
                             "pcm_start=%08x, pcm_end=%08x",
                             guard_mp3buf_start, guard_mp3buf_end, guard_pcm_start, guard_pcm_end);
                    // Dump nearby memory to help root cause
                    size_t   dumpStart = reinterpret_cast<uintptr_t>(&guard_mp3buf_start);
                    uint8_t* p         = reinterpret_cast<uint8_t*>(dumpStart);
                    ESP_LOGE(TAG, "Memory dump around MP3 guards (64 bytes):");
                    for (size_t i = 0; i < 64; ++i) { ESP_LOGE(TAG, "%02x", p[i]); }
                    ESP_LOGE(TAG,
                             "loopIterations=%zu framesDecoded=%zu mp3bufFill=%zu pcmFill=%zu "
                             "mp3bufStart=%zu",
                             loopIterations, framesDecoded, mp3bufFill, pcmFill, mp3bufStart);
                    // Break immediately so debugger can catch the exact moment of corruption
                    raise(SIGTRAP);
                    abort();
                    break;
                }

                // Validate canaries immediately after consuming bytes
                if (guard_mp3buf_start != 0xC0FFEE11 || guard_mp3buf_end != 0xB16B00B5 ||
                    guard_pcm_start != 0xABCD1234 || guard_pcm_end != 0x1234ABCD) {
                    ESP_LOGE(TAG,
                             "Canary corrupted AFTER memmove: start=%08x, mp3_end=%08x, "
                             "pcm_start=%08x, pcm_end=%08x",
                             guard_mp3buf_start, guard_mp3buf_end, guard_pcm_start, guard_pcm_end);
                    // Dump nearby memory to help root cause
                    size_t   dumpStart = reinterpret_cast<uintptr_t>(&guard_mp3buf_start);
                    uint8_t* p         = reinterpret_cast<uint8_t*>(dumpStart);
                    ESP_LOGE(TAG, "Memory dump around MP3 guards (64 bytes):");
                    for (size_t i = 0; i < 64; ++i) { ESP_LOGE(TAG, "%02x", p[i]); }
                    ESP_LOGE(TAG,
                             "loopIterations=%zu framesDecoded=%zu mp3bufFill=%zu pcmFill=%zu",
                             loopIterations, framesDecoded, mp3bufFill, pcmFill);
                    // Break immediately so debugger can catch the exact moment of corruption
                    raise(SIGTRAP);
                    abort();
                    break;
                }
            }

            // Update sample rate / channels if changed
            if (info.hz != 0 && info.hz != sampleRate) { sampleRate = info.hz; }
            if (info.channels != 0 && info.channels != channels) { channels = info.channels; }

            continue; // go back to copy decoded PCM to 'out'
        }
        else {
            // Decode returned 0: either need more data or a bad frame
            if (info.frame_bytes > 0 && (size_t)info.frame_bytes <= mp3bufFill) {
                // skip reported bad bytes
                size_t remaining = mp3bufFill - (size_t)info.frame_bytes;
                if (remaining > 0) { memmove(mp3buf, mp3buf + info.frame_bytes, remaining); }
                mp3bufFill = remaining;
                ESP_LOGV(TAG, "Skipped %d bad bytes, mp3bufFill=%zu", info.frame_bytes,
                         mp3bufFill);
                continue;
            }

            // Not enough data: try to read more, otherwise EOF
            if (file_eof) {
                eof = true;
                break;
            }
            // If not at EOF, loop will attempt to read more data on next iteration
            continue;
        }
    }

    // Pad remaining with silence if we didn't fill entire buffer
    if (done < bytes) {
        memset(dst + done, 0, bytes - done);
        ESP_LOGV(TAG, "Padded %zu bytes with silence", bytes - done);
    }

    // Track samples output (each sample is 2 bytes for 16-bit audio)
    totalSamplesOutput += (done / sizeof(int16_t));

    return bytes;
}

uint32_t MP3Player::getTimeMs() {
    if (sampleRate == 0 || channels == 0) {
        return 0; // Not initialized
    }

    // Calculate time from total samples output divided by sample rate and channels
    // totalSamplesOutput counts int16_t values (one per channel per sample)
    // So actual audio samples = totalSamplesOutput / channels
    // time(ms) = (totalSamplesOutput / channels / sampleRate) * 1000
    //          = (totalSamplesOutput * 1000) / (sampleRate * channels)
    uint64_t timeMs = (totalSamplesOutput * 1000ULL) / ((uint64_t)sampleRate * channels);

    // Cap at reasonable 32-bit value
    if (timeMs > 0xFFFFFFFFULL) { return 0xFFFFFFFF; }

    return (uint32_t)timeMs;
}

void MP3Player::setTimeMs(uint32_t timeMs) {
    if (!currentFile || sampleRate == 0 || channels == 0) {
        ESP_LOGW(TAG, "setTimeMs: Player not initialized");
        return;
    }

    ESP_LOGI(TAG, "Seeking to %u ms", timeMs);

    // Calculate estimated file position based on bitrate
    // Formula: file_pos = (timeMs / 1000.0) * (bitrate_kbps * 1000) / 8
    // Simplified: file_pos = timeMs * bitrate_kbps / 8
    uint32_t estimatedPos = (timeMs * lastBitrate) / 8;

    // Clamp to file size
    if (estimatedPos > fileSizeBytes) { estimatedPos = fileSizeBytes; }

    // Seek to the estimated position
    currentFile->seek(estimatedPos, SEEK_SET);

    // Reset decoder state
    mp3dec_init(&mp3d);

    // Reset buffers
    mp3bufStart = 0;
    mp3bufFill  = 0;
    pcmPos      = 0;
    pcmFill     = 0;
    file_eof    = false;

    // Initialize totalSamplesOutput to the target time position
    // so getTimeMs() returns the correct absolute playback time
    totalSamplesOutput = ((uint64_t)timeMs * sampleRate * channels) / 1000;

    // Try to find and decode the next valid MP3 frame
    mp3bufFill = currentFile->read(mp3buf, sizeof(mp3buf));
    if (mp3bufFill == 0) {
        ESP_LOGE(TAG, "setTimeMs: Failed to read data after seek");
        eof = true;
        return;
    }

    ESP_LOGI(TAG, "Sought to file position %u, readBuffer %zu bytes", estimatedPos, mp3bufFill);
}