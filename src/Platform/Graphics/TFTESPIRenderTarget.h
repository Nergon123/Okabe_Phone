#pragma once
#include "RenderTargets.h"
#include <functional>

class TFTESPIRenderTarget : public RenderTarget {
public:
    // callbacks: draw(x,y,color), push(x,y,w,h,data,transparent,transpColor)
    TFTESPIRenderTarget(std::function<void(int16_t,int16_t,uint16_t)> drawCb,
                        std::function<void(int16_t,int16_t,int16_t,int16_t,const uint16_t*,bool,uint16_t)> pushCb,
                        std::function<void()> initCb = nullptr,
                        std::function<void()> deinitCb = nullptr,
                        int16_t w = 0, int16_t h = 0)
        : RenderTarget(RENDER_TARGET_TYPE_SCREEN, w, h, nullptr),
          drawCallback(std::move(drawCb)), pushCallback(std::move(pushCb)),
          initCallback(std::move(initCb)), deinitCallback(std::move(deinitCb))
    {}

    virtual ~TFTESPIRenderTarget() = default;

    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        if (drawCallback) drawCallback(x, y, color);
    }

    void pushBuffer(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data, bool transparent, uint16_t transpColor) override {
        if (pushCallback) pushCallback(x, y, w, h, data, transparent, transpColor);
    }
    void idle() override {
        // no-op 
    }
    void init() override { if (initCallback) initCallback(); }
    void deinit() override { if (deinitCallback) deinitCallback(); }

private:
    std::function<void(int16_t,int16_t,uint16_t)> drawCallback;
    std::function<void(int16_t,int16_t,int16_t,int16_t,const uint16_t*,bool,uint16_t)> pushCallback;
    std::function<void()> initCallback;
    std::function<void()> deinitCallback;
};

// Helper: if TFT_eSPI is available, provide a convenience constructor binding its methods.
// This block is optional and only compiled when the header exists.
#if __has_include(<TFT_eSPI.h>)
#include <TFT_eSPI.h>
inline TFTESPIRenderTarget *makeTFTESPIRenderTarget(TFT_eSPI *tft) {
    if (!tft) return nullptr;
    int16_t w = 0, h = 0;
    // try to retrieve dimensions if methods exist
    #if defined(TFT_WIDTH) && defined(TFT_HEIGHT)
        w = TFT_WIDTH;
        h = TFT_HEIGHT;
    #endif

    auto drawCb = [tft](int16_t x, int16_t y, uint16_t c){ tft->drawPixel(x, y, c); };
    auto pushCb = [tft](int16_t x, int16_t y, int16_t w_, int16_t h_, const uint16_t *data, bool transparent, uint16_t transpColor){
        // TFT_eSPI has pushImage; it doesn't support transparency itself,
        // so if transparent==true we fallback to per-pixel draw.
        if (!transparent) {
            tft->pushImage(x, y, w_, h_, data);
        } else {
            tft->pushImage(x, y, w_, h_, data, transpColor);
        }
    };
    auto initCb = [tft](){ tft.init(); };
    auto deinitCb = [tft](){ /* optional: nothing */ };

    TFTESPIRenderTarget *rt = new TFTESPIRenderTarget(drawCb, pushCb, initCb, deinitCb, w, h);
    return rt;
}
#endif