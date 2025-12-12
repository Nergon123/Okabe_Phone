#pragma once
#include <Defines.h>
#include <cstdint>
struct Viewport {
    int16_t x = 0;
    int16_t y = 0;
    int16_t w = 0;
    int16_t h = 0;

    Viewport() {}

    Viewport(int16_t _x, int16_t _y, int16_t _w, int16_t _h) : x(_x), y(_y), w(_w), h(_h) {}
};

#define NOT_IMPLEMENTED                                                                           \
    ESP_LOGW("VIRT RT", "%s is Not implemented for current renderTarget!", __func__);
enum RENDER_TARGET_TYPE {
    RENDER_TARGET_TYPE_NONE = 0,
    RENDER_TARGET_TYPE_BUFFER,
    RENDER_TARGET_TYPE_SCREEN
};

class RenderTarget {
  public:
    RenderTarget(RENDER_TARGET_TYPE t = RENDER_TARGET_TYPE_NONE, int16_t w = 0, int16_t h = 0,
                 uint16_t *buf = nullptr)
        : type(t), width(w), height(h), buffer(buf) {}

    virtual ~RenderTarget() = default;

    // Core drawing operations - implemented by concrete render targets
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) {
        NOT_IMPLEMENTED;
        (void)x;
        (void)y;
        (void)color;
    }
    virtual void pushBuffer(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data,
                            bool transparent, uint16_t transpColor) {
        NOT_IMPLEMENTED;
        (void)x;
        (void)y;
        (void)w;
        (void)h;
        (void)data;
        (void)transparent;
        (void)transpColor;
    }
    virtual void setViewport(Viewport vp) {
        NOT_IMPLEMENTED;

        (void)vp;
    };
    virtual Viewport getViewport() {
        NOT_IMPLEMENTED;

        return Viewport();
    };

    virtual void setAddrWindow(uint16_t xs, uint16_t ys, uint16_t w, uint16_t h) {
        NOT_IMPLEMENTED;

        (void)xs;
        (void)ys;
        (void)w;
        (void)h;
    }
    virtual void writeColor(uint16_t color, uint32_t len) {
        NOT_IMPLEMENTED;

        (void)color;
        (void)len;
    }
    virtual void pushColors(uint16_t *data, uint32_t len, bool swap = false) {
        NOT_IMPLEMENTED;

        (void)data;
        (void)len;
        (void)swap;
    }
    virtual void fillScreen(uint16_t color) {
        NOT_IMPLEMENTED;

        (void)color;
    }
    // Optional lifecycle hooks with default no-op implementations
    virtual void init() {}
    virtual void deinit() {}
    virtual void present() {}
    // Accessors
    RENDER_TARGET_TYPE getType() const { return type; }
    int16_t            getWidth() const { return width; }
    int16_t            getHeight() const { return height; }
    uint16_t          *getBuffer() const { return buffer; }

  protected:
    RENDER_TARGET_TYPE type;
    int16_t            width;
    int16_t            height;
    uint16_t          *buffer;
};

#ifdef PC
RenderTarget *setupSDL2RenderTarget(int16_t width, int16_t height, const char *title);
#else
RenderTarget *setupTFTESPIRenderTarget();
#endif
