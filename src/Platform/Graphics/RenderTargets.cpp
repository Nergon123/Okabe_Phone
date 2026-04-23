#include "RGB565BufferRenderTarget.h"
#include "SDL2RenderTarget.h"
#include "TFTESPIRenderTarget.h"
#ifndef PC
static TFTESPIRenderTarget tftTarget(240, 320, true);

RenderTarget* setupTFTESPIRenderTarget() { return &tftTarget; };
#else

RenderTarget* setupSDL2RenderTarget() { return new SDL2RenderTarget(240, 320, "Emulator"); };

#endif
