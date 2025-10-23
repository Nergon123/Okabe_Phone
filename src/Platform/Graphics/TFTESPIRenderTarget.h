#pragma once
#include "RenderTargets.h"
#include <functional>

#ifndef PC
#include <TFT_eSPI.h>
TFT_eSPI _tft;
#endif

class TFTESPIRenderTarget : public RenderTarget {
  public:
    // callbacks: draw(x,y,color), push(x,y,w,h,data,transparent,transpColor)
    TFTESPIRenderTarget(int16_t w = 0, int16_t h = 0)
        : RenderTarget(RENDER_TARGET_TYPE_SCREEN, w, h, nullptr) {}

    virtual ~TFTESPIRenderTarget() = default;

    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        _tft.drawPixel(x, y, color);
    }

    void pushBuffer(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data,
                    bool transparent, uint16_t transpColor) override {
        if (transparent) { _tft.pushImage(x, y, w, h, data, transpColor); }
        else { _tft.pushImage(x, y, w, h, data); }
    }
    void idle() override {
        // no-op
    }
    void init() override {
        _tft.init();
    }
    void deinit() override {
        // no-op
    }

};


#ifndef PC

inline RenderTarget *setupTFTESPIRenderTarget() {
    return new TFTESPIRenderTarget(240, 320);
}
#endif