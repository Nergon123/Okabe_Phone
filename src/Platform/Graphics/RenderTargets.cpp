#include "RGB565BufferRenderTarget.h"
#include "SDL2RenderTarget.h"
#include "TFTESPIRenderTarget.h"
#ifndef PC
RenderTarget *tft_target = nullptr;
RenderTarget *setupTFTESPIRenderTarget() {
    if (!tft_target) { return tft_target = new TFTESPIRenderTarget(240, 320, true); }
    else { return tft_target; }
};
#else

RenderTarget* setupSDL2RenderTarget() { return new SDL2RenderTarget(240, 320, "Emulator"); };

#endif
