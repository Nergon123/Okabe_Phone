#pragma once

#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#include "RenderTargets.h"
#include <SDL2/SDL.h>
#include <memory>

class SDL2RenderTarget : public RenderTarget {
public:
    SDL2RenderTarget(int16_t w, int16_t h, const char* title = "SDL Window")
        : RenderTarget(RENDER_TARGET_TYPE_SCREEN, w, h, nullptr),
          window(nullptr), renderer(nullptr), texture(nullptr)
    {
        SDL_Init(SDL_INIT_VIDEO);
        window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                w, h, SDL_WINDOW_SHOWN);
        if (window) {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            if (renderer) {
                texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565,
                                          SDL_TEXTUREACCESS_STREAMING, w, h);
                buffer = new uint16_t[w * h];
                std::memset(buffer, 0, w * h * sizeof(uint16_t));
            }
        }
        present();
    }

    virtual ~SDL2RenderTarget() {
        if (texture) SDL_DestroyTexture(texture);
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        delete[] buffer;
        SDL_Quit();
    }

    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        if (!buffer || x < 0 || y < 0 || x >= width || y >= height) return;
        buffer[y * width + x] = color;
        present();
    }

    void pushBuffer(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data,
                   bool transparent, uint16_t transpColor) override {
        if (!buffer || !data) return;
        ESP_LOGI("SDL2RenderTarget", "Pushing buffer at (%d,%d) size %dx%d", x, y, w, h);
        for (int ry = 0; ry < h; ++ry) {
            int16_t dstY = y + ry;
            if (dstY < 0 || dstY >= height) continue;
            for (int rx = 0; rx < w; ++rx) {
                int16_t dstX = x + rx;
                if (dstX < 0 || dstX >= width) continue;
                uint16_t srcPx = data[ry * w + rx];
                if (transparent && srcPx == transpColor) continue;
                buffer[dstY * width + dstX] = srcPx;
            }
        }
        present();
    }
    void idle() override {
        // Small delay and pump events so the SDL window remains responsive
        SDL_Delay(1);
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                // User closed window — exit the process to avoid undefined state
                SDL_Quit();
                std::exit(0);
            }
            // Ignore other events here; the application can poll SDL directly
        }
    }
    void present() override {
        if (!texture || !renderer || !buffer) return;
        SDL_UpdateTexture(texture, NULL, buffer, width * sizeof(uint16_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        // Poll events each time we present to keep the window manager happy.
        // This prevents the window from becoming "Not Responding" when the
        // application is busy in other loops.
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                SDL_Quit();
                std::exit(0);
            }
        }
    }

    void init() override { /* Already initialized in constructor */ }
    void deinit() override { /* Cleanup handled in destructor */ }

    bool isValid() const { return window && renderer && texture && buffer; }
    SDL_Window* getWindow() const { return window; }

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
};

// Factory function that matches the existing pattern
inline RenderTarget* setupSDL2RenderTarget(int16_t width, int16_t height, const char* title = "SDL Window") {
    return new SDL2RenderTarget(width, height, title);
}

#endif // Platform check
