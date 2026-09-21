#include "RGB565BufferRenderTarget.h"
#include "SDL2RenderTarget.h"
#ifndef PC
#include "IDFRenderTarget.h"
RenderTarget *tft_target = nullptr;
RenderTarget *setupIDFRenderTarget() {
    if (!tft_target) { return tft_target = new IDFRenderTarget(); }
    else { return tft_target; }
};
#else

RenderTarget* setupSDL2RenderTarget() { return new SDL2RenderTarget(240, 320, "Emulator"); };

#endif
