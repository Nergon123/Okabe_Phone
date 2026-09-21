#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace pcm_volume {
constexpr uint32_t unity = 32768;

inline uint32_t gain(float volume) {
    // Treat NaN like mute, and clamp values outside the public 0..1 range.
    if (!(volume > 0.0f)) return 0;
    if (volume >= 1.0f) return unity;
    return static_cast<uint32_t>(volume * unity + 0.5f);
}

// Signed 16-bit native-endian PCM; gain must be in [0, unity].
inline void apply(void* data, size_t bytes, uint32_t gain) {
    if (gain == unity) return;
    if (gain == 0) {
        std::memset(data, 0, bytes);
        return;
    }
    auto* buffer = static_cast<uint8_t*>(data);
    for (size_t offset = 0; offset + sizeof(int16_t) <= bytes; offset += sizeof(int16_t)) {
        int16_t sample;
        std::memcpy(&sample, buffer + offset, sizeof(sample));
        // 32-bit multiplication covers the entire int16 range without overflow.
        sample = static_cast<int16_t>((static_cast<int32_t>(sample) *
                                      static_cast<int32_t>(gain)) / static_cast<int32_t>(unity));
        std::memcpy(buffer + offset, &sample, sizeof(sample));
    }
}
}
