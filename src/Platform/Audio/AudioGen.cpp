#include "AudioGen.h"
#include "SDLAudio.h"
#include <GlobalVariables.h>
size_t multi_callback(void* user, void* out, size_t bytes) {
    MultiOscillator* osc     = (MultiOscillator*)user;
    int16_t*         dst     = (int16_t*)out;
    size_t           samples = bytes / sizeof(int16_t);

    for (size_t i = 0; i < samples; i += osc->channels) {
        int16_t sample = (int16_t)(osc->mix() * 32767.0f);

        for (uint8_t ch = 0; ch < osc->channels; ch++) { *dst++ = sample; }
    }

    return bytes;
}

void sample() {

    MultiOscillator synth(48000, 2);

    Voice* v = synth.addVoice(WAVE_TRIANGLE, 100.0f, 0.3f); // 100 Hz square

    AudioStream stream{.callback   = multi_callback,
                       .user       = &synth,
                       .sampleRate = 48000,
                       .channels   = 2,
                       .format     = AUDIOFMT_S16,
                       .state      = AUDIO_STOPPED};
#ifdef SDL
    Audio* audio = new SDLAudio();
#else
    Audio* audio = new NullAudio();
#endif
    audio->init();
    audio->play(&stream);
    for (;;) {
        v->amplitude = 1;
        for (int i = 0; i < 10; i++) {
            v->frequency = (sinf(i % 3) * 300) + 300;
            hw->delay(100);
        }
        v->amplitude = 0;
        hw->delay(1000);
    }
}