#pragma once
#include <Platform/Audio/AudioGen.h>
#include <System/MP3Player.h>

struct PlayAudio {
    bool           isPlaying;
    bool            isMP3;
    bool            isSynth;
    NString         currentFile;
    ulong           currentTime;
};