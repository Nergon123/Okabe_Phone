#pragma once
#include "RGB565BufferRenderTarget.h"
#include "RenderTargets.h"
#include <System/Generic.h>
#include <functional>
#ifndef PC
#include <TFT_eSPI.h>

class TFTESPIRenderTarget : public RenderTarget {
  public:
    TFT_eSPI      _tft;
    RenderTarget* bufferTargetToFlip;
    bool          _useBuffer = false;
    TFTESPIRenderTarget(int w, int h, bool useBuffer = false)
        : RenderTarget(RENDER_TARGET_TYPE_SCREEN, w, h, nullptr), _useBuffer(useBuffer) {};

    void init() override {
        if (_useBuffer && !bufferTargetToFlip) {
            bufferTargetToFlip = setupBufferRenderTarget(240, 320);
        }
        hw->setScreenBrightness(0);
        _tft.init();
        _tft.fillScreen(TFT_BLACK);
        _tft.endWrite();
        hw->setScreenBrightness(currentBrightness);
    }

    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        if (_useBuffer && bufferTargetToFlip) {
            bufferTargetToFlip->drawPixel(x, y, color);
            return;
        }
        _tft.startWrite();
        _tft.drawPixel(x, y, color);
        _tft.endWrite();
    }

    void pushBuffer(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* data,
                    bool transparent, uint16_t transpColor) override {
        if (_useBuffer && bufferTargetToFlip) {
            bufferTargetToFlip->pushBuffer(x, y, w, h, data, transparent, transpColor);
            return;
        }
        _tft.startWrite();
        if (transparent) {
            transpColor = (transpColor >> 8) | (transpColor << 8);
            _tft.pushImage(x, y, w, h, data, transpColor);
        }
        else { _tft.pushImage(x, y, w, h, data); }
        _tft.endWrite();
    }

    void setAddrWindow(uint16_t xs, uint16_t ys, uint16_t w, uint16_t h) override {
        if (_useBuffer && bufferTargetToFlip) {
            bufferTargetToFlip->setAddrWindow(xs, ys, w, h);
            return;
        }
        _tft.startWrite();
        _tft.setAddrWindow(xs, ys, w, h);
        _tft.endWrite();
    }

    void pushColors(uint16_t* data, uint32_t len, bool swap = false) override {
        if (_useBuffer && bufferTargetToFlip) {
            bufferTargetToFlip->pushColors(data, len, swap);
            return;
        }
        _tft.startWrite();
        _tft.pushColors(data, len, swap);
        _tft.endWrite();
    }
    void setViewport(Viewport vp) override {
        if (_useBuffer && bufferTargetToFlip) {
            bufferTargetToFlip->setViewport(vp);
            return;
        }
        _tft.setViewport(vp.x, vp.y, vp.w, vp.h);
    }
    Viewport getViewport() {
        if (_useBuffer && bufferTargetToFlip) { return bufferTargetToFlip->getViewport(); }
        return Viewport(_tft.getViewportX(), _tft.getViewportY(), _tft.getViewportWidth(),
                        _tft.getViewportHeight());
    };
    void writeColor(uint16_t color, uint32_t len) override {
        if (_useBuffer && bufferTargetToFlip) {
            bufferTargetToFlip->writeColor(color, len);
            return;
        }
        _tft.startWrite();
        _tft.writeColor(color, len);
        _tft.endWrite();
    }

    void fillScreen(uint16_t color) override {
        if (_useBuffer && bufferTargetToFlip) {
            bufferTargetToFlip->fillScreen(color);
            return;
        }
        _tft.startWrite();
        _tft.fillScreen(color);
        _tft.endWrite();
    }
    void setUseBuffer(bool useBuffer) override { _useBuffer = useBuffer; }
    bool getUseBuffer() override { return _useBuffer; }
    void present() override {
        if (_useBuffer && bufferTargetToFlip) {
            _tft.pushImage(0, 0, 240, 320, bufferTargetToFlip->getBuffer());
        }
    }
};

#endif