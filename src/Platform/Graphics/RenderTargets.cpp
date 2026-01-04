#include "RGB565BufferRenderTarget.h"
#include "SDL2RenderTarget.h"
#include "TFTESPIRenderTarget.h"
#ifndef PC
static TFTESPIRenderTarget tftTarget(240, 320, true);

RenderTarget* setupTFTESPIRenderTarget() { return &tftTarget; };
#else
static SDL2RenderTarget tftTarget(240, 320, "Emulator");

RenderTarget* setupSDL2RenderTarget() { return &tftTarget; };
#endif
