#pragma once
#ifdef PC
#include "RenderTargets.h"
#include <SDL2/SDL.h>
#include <memory>

class SDL2RenderTarget : public RenderTarget {
  public:
    SDL2RenderTarget(int16_t w, int16_t h, const char* title = "SDL Window")
        : RenderTarget(RENDER_TARGET_TYPE_SCREEN, w, h, nullptr), window(nullptr),
          renderer(nullptr), texture(nullptr), windowX(0), windowY(0), windowW(w * SDLScale),
          windowH(h * SDLScale) {
        SDL_Init(SDL_INIT_VIDEO);
        window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowW,
                                  windowH, SDL_WINDOW_SHOWN);
        if (window) {
            renderer = SDL_CreateRenderer(window, -1, 0);
            SDL_RenderSetScale(renderer, SDLScale, SDLScale);
            SDL_RenderSetLogicalSize(renderer, windowW, windowH);
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

    void setViewport(Viewport newVp) override {
        // clamp to screen dimensions
        if (newVp.x < 0) { newVp.x = 0; }
        if (newVp.y < 0) { newVp.y = 0; }
        if (newVp.x + newVp.w > width) { newVp.w = width - newVp.x; }
        if (newVp.y + newVp.h > height) { newVp.h = height - newVp.y; }
        vp = newVp;
    }

    Viewport getViewport() override { return vp; }

    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        if (!buffer) { return; }
        if (x < vp.x || y < vp.y || x >= vp.x + vp.w || y >= vp.y + vp.h) { return; }
        buffer[y * width + x] = (color >> 8) | (color << 8);
    }

    void pushBuffer(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* data,
                    bool transparent, uint16_t transpColor) override {
        if (!buffer || !data) { return; }

        int startX = std::max(x, vp.x);
        int endX   = std::min(x + w, vp.x + vp.w);
        int startY = std::max(y, vp.y);
        int endY   = std::min(y + h, vp.y + vp.h);

        for (int ry = 0; ry < endY - startY; ++ry) {
            int             dstY   = startY + ry;
            int             srcY   = ry + (startY - y);
            const uint16_t* srcRow = data + srcY * w;
            uint16_t*       dstRow = buffer + dstY * width;
            for (int rx = 0; rx < endX - startX; ++rx) {
                int      dstX = startX + rx;
                int      srcX = rx + (startX - x);
                uint16_t px   = (srcRow[srcX] >> 8) | (srcRow[srcX] << 8);
                if (transparent && px == transpColor) { continue; }
                dstRow[dstX] = px;
            }
        }
    }

    void fillScreen(uint16_t color) override {
        color = (color >> 8) | (color << 8);
        if (!buffer) { return; }
        for (int y = vp.y; y < vp.y + vp.h; ++y) {
            for (int x = vp.x; x < vp.x + vp.w; ++x) { buffer[y * width + x] = color; }
        }
        present();
    }

  private:
    SDL_Window*   window;
    SDL_Renderer* renderer;
    SDL_Texture*  texture;
    Viewport      vp; // viewport
}

// Factory function that matches the existing pattern
inline RenderTarget* setupSDL2RenderTarget(int16_t width, int16_t height,
                                           const char* title = "SDL Window") {
    return new SDL2RenderTarget(width, height, title);
}

#endif // Platform check
