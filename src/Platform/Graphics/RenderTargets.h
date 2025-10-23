#pragma once 
#include <cstdint>

enum RENDER_TARGET_TYPE {
    RENDER_TARGET_TYPE_NONE = 0,
    RENDER_TARGET_TYPE_BUFFER,
    RENDER_TARGET_TYPE_SCREEN
};

class RenderTarget {
public:
    RenderTarget(RENDER_TARGET_TYPE t = RENDER_TARGET_TYPE_NONE, int16_t w = 0, int16_t h = 0, uint16_t *buf = nullptr)
        : type(t), width(w), height(h), buffer(buf) {}

    virtual ~RenderTarget() = default;

    // Core drawing operations - implemented by concrete render targets
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    virtual void pushBuffer(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data, bool transparent, uint16_t transpColor) = 0;

    // Optional lifecycle hooks with default no-op implementations
    virtual void init() {}
    virtual void deinit() {}
    virtual void idle() {}
    virtual void present() {}
    // Accessors
    RENDER_TARGET_TYPE getType() const { return type; }
    int16_t getWidth() const { return width; }
    int16_t getHeight() const { return height; }
    uint16_t* getBuffer() const { return buffer; }

    protected:
    RENDER_TARGET_TYPE type;
    int16_t width;
    int16_t height;
    uint16_t *buffer;
};

#ifdef PC
RenderTarget* setupSDL2RenderTarget(int16_t width, int16_t height, const char* title);
#else
 RenderTarget *setupTFTESPIRenderTarget();
#endif

