#pragma once
#include "RenderTargets.h"
#include <functional>

#ifndef PC
#include <TFT_eSPI.h>
TFT_eSPI _tft;
#endif

class TFTESPIRenderTarget : public RenderTarget {
public:
    TFTESPIRenderTarget(int w,int h)
        : RenderTarget(RENDER_TARGET_TYPE_SCREEN, w,h, nullptr) {
            init();
        }

    void init() override {
        _tft.init();
        _tft.fillScreen(TFT_BLACK);
    }

    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        _tft.drawPixel(x, y, color);
    }

    void pushBuffer(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* data,
                    bool transparent, uint16_t transpColor) override {
        if (transparent) _tft.pushImage(x, y, w, h, data, transpColor);
        else _tft.pushImage(x, y, w, h, data);
    }

    void setAddrWindow(uint16_t xs, uint16_t ys, uint16_t w, uint16_t h) override {
        _tft.setAddrWindow(xs, ys, w, h);
    }

    void pushColors(uint16_t* data, uint32_t len, bool swap = false) override {
        _tft.pushColors(data, len, swap);       

    }

    void writeColor(uint16_t color, uint32_t len) override {
        _tft.writeColor(color, len);
    }

    void fillScreen(uint16_t color) override {
        _tft.fillScreen(color);
    }

    void present() override {} // no buffering on TFT
    void deinit() override {}  // no cleanup needed

};


#ifndef PC

inline RenderTarget *setupTFTESPIRenderTarget() { return new TFTESPIRenderTarget(240, 320); }
#endif