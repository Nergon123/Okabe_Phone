#include "AudioGen.h"
#include "SDLAudio.h"
#include <GlobalVariables.h>
#include <System/MP3Player.h>
size_t multi_callback(void* user, void* out, size_t bytes)
{
    auto* osc = static_cast<MultiOscillator*>(user);
    auto* dst = static_cast<int16_t*>(out);

    size_t frames = bytes / (sizeof(int16_t) * 2);

    for (size_t i = 0; i < frames; ++i)
    {
        int16_t s = (int16_t)(osc->mix() * 20000.0f);

        dst[i * 2 + 0] = s;
        dst[i * 2 + 1] = s;
    }

    return frames * sizeof(int16_t) * 2;
}
void PlayMP3Sample(NString path) {
    audioSource->init();
    MP3Player* player = new MP3Player(audioSource);
    player->init(path);
    player->play();
    player->setLoop(true);
}

void sampleOSC() {

    MultiOscillator synth(48000, 2);

    Voice* v = synth.addVoice(WAVE_SINE, 1000.0f, 0.3f); // 100 Hz square

    AudioStream stream{.callback   = multi_callback,
                       .user       = &synth,
                       .sampleRate = 48000,
                       .channels   = 1,
                       .format     = AUDIOFMT_S16,
                       .state      = AUDIO_STOPPED};
    
    audioSource->init();
    audioSource->play(&stream);
    

    for (;;) {
        v->amplitude = 0.7;
        for (int i = 0; i < 20; i++) {
            v->frequency = 1000;
            hw->delay(50);
            v->frequency = 1000;
            hw->delay(50);
        }
        v->amplitude = 0;
        hw->delay(1000);
    }
}