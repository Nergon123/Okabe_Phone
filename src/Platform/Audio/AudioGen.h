#include <Platform/NString.h>
#include <random>
#include <vector>
enum WaveformType { WAVE_SINE, WAVE_SQUARE, WAVE_TRIANGLE, WAVE_NOISE };
struct Voice {
    WaveformType type;
    float        frequency;
    float        amplitude;
    float        phase;
    uint16_t     id;
    Voice(WaveformType t = WAVE_SINE, float f = 440.0f, float a = 1.0f, int id = -1)
        : type(t), frequency(f), amplitude(a), phase(0.0f), id(id) {}
};
class MultiOscillator {
  public:
    MultiOscillator(uint32_t sr = 48000, uint8_t ch = 2) : channels(ch), sampleRate(sr) {
        rng.seed(12345);
    }

    Voice* addVoice(WaveformType type, float freq, float amp = 1.0f) {
        voices.emplace_back(type, freq, amp);
        return &voices.back(); // return pointer to the newly added voice
    }
    void clearVoices() { voices.clear(); }
    void deleteVoice(int id) {
        for (size_t i = 0; i < voices.size(); i++) {
            if (voices.at(i).id == id) { voices.erase(voices.begin() + i); }
        }
    }
    float nextSample(size_t voiceIndex) {
        if (voiceIndex >= voices.size()) { return 0.0f; }

        Voice& v        = voices[voiceIndex];
        float  value    = 0.0f;
        float  phaseInc = 2.0f * 3.14159265f * v.frequency / sampleRate;

        switch (v.type) {
        case WAVE_SINE: value = sinf(v.phase); break;
        case WAVE_SQUARE: value = (v.phase < 3.14159265f) ? 1.0f : -1.0f; break;
        case WAVE_TRIANGLE:
            value = 2.0f * (v.phase / (2.0f * 3.14159265f)) - 1.0f;
            value = 2.0f * (fabsf(value) - 0.5f);
            break;
        case WAVE_NOISE: value = noiseDist(rng); break;
        }

        v.phase += phaseInc;
        if (v.phase > 2.0f * 3.14159265f) { v.phase -= 2.0f * 3.14159265f; }

        return value * v.amplitude;
    }

    // Mix all voices into one sample
    float mix() {
        float out = 0.0f;
        for (size_t i = 0; i < voices.size(); i++) { out += nextSample(i); }
        // Optional normalization to prevent clipping
        if (!voices.empty()) { out /= voices.size(); }
        return out;
    }

    uint8_t channels;

  private:
    std::vector<Voice>                    voices;
    uint32_t                              sampleRate;
    std::mt19937                          rng;
    std::uniform_real_distribution<float> noiseDist{-1.0f, 1.0f};
};
void sampleOSC();
void PlayMP3Sample(NString path);