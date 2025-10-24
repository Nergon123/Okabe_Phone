#pragma once

#ifdef PC
#include "RenderTargets.h"
#include <SDL2/SDL.h>
#include <memory>

class SDL2RenderTarget : public RenderTarget {
  public:
    SDL2RenderTarget(int16_t w, int16_t h, const char* title = "SDL Window")
        : RenderTarget(RENDER_TARGET_TYPE_SCREEN, w, h, nullptr), window(nullptr),
          renderer(nullptr), texture(nullptr), windowX(0), windowY(0), windowW(w), windowH(h) {
        SDL_Init(SDL_INIT_VIDEO);
        window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h,
                                  SDL_WINDOW_SHOWN);
        if (window) {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            if (renderer) {
                texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565,
                                            SDL_TEXTUREACCESS_STREAMING, w, h);
                buffer  = new uint16_t[w * h];
                std::memset(buffer, 0, w * h * sizeof(uint16_t));
            }
        }
        present();
    }

    virtual ~SDL2RenderTarget() {
        if (texture) { SDL_DestroyTexture(texture); }
        if (renderer) { SDL_DestroyRenderer(renderer); }
        if (window) { SDL_DestroyWindow(window); }
        delete[] buffer;
        SDL_Quit();
    }

    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        if (!buffer || x < 0 || y < 0 || x >= width || y >= height) { return; }
        buffer[y * width + x] = color;
    }

void pushBuffer(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* data,
                bool transparent, uint16_t transpColor) override {
    if (!buffer || !data) return;

int startX = std::max(0, static_cast<int>(x));
int endX   = std::min(static_cast<int>(x + w), static_cast<int>(width));

int startY = std::max(0, static_cast<int>(y));
int endY   = std::min(static_cast<int>(y + h), static_cast<int>(height));


for (int ry = 0; ry < endY - startY; ++ry) {
    int dstY = startY + ry;
    int srcY = ry + (startY - y); // adjusted for clipping
    uint16_t* dstRow = buffer + dstY * width;
    const uint16_t* srcRow = data + srcY * w;
    for (int rx = 0; rx < endX - startX; ++rx) {
        int dstX = startX + rx;
        int srcX = rx + (startX - x);
        uint16_t srcPx = srcRow[srcX];
        if (transparent && srcPx == transpColor) continue;
        dstRow[dstX] = (srcPx >> 8) | (srcPx << 8);
    }
}
}

    void fillScreen(uint16_t color) override {
        for (int i = 0; i < width * height; ++i) { buffer[i] = color; }
        present();
    }
    void writeColor(uint16_t color, uint32_t len) override {
        if (len > (uint32_t)windowW * windowH) len = windowW * windowH;

        for (uint32_t i = 0; i < len; ++i) {
            int16_t px = windowX + (i % windowW);
            int16_t py = windowY + (i / windowW);
            if (px >= 0 && px < width && py >= 0 && py < height) {
                buffer[py * width + px] = color;
            }
        }
    }
    virtual void setAddrWindow(uint16_t xs, uint16_t ys, uint16_t w, uint16_t h) {
        windowX = xs;
        windowY = ys;
        windowW = w;
        windowH = h;
    }
    void pushColors( uint16_t* data, uint32_t len, bool swap = false) override {
        for (uint32_t i = 0; i < len; ++i) {
            uint16_t color = swap ? (data[i] >> 8) | (data[i] << 8) : data[i];
            writeColor(color, 1);
        }
    }

    void present() override {
        if (!texture || !renderer || !buffer) { return; }
        SDL_UpdateTexture(texture, NULL, buffer, width * sizeof(uint16_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    void init() override { /* Already initialized in constructor */ }
    void deinit() override { /* Cleanup handled in destructor */ }

    bool        isValid() const { return window && renderer && texture && buffer; }
    SDL_Window* getWindow() const { return window; }

  private:
    SDL_Window*   window;
    SDL_Renderer* renderer;
    SDL_Texture*  texture;
    int           windowX, windowY, windowH, windowW;
};

// Factory function that matches the existing pattern
inline RenderTarget* setupSDL2RenderTarget(int16_t width, int16_t height,
                                           const char* title = "SDL Window") {
    return new SDL2RenderTarget(width, height, title);
}

#endif // Platform check
