#pragma once
#include "RGB565BufferRenderTarget.h"
#include <algorithm>
#include <op_lcd.h>

// Framebuffer composition with one accumulated dirty rectangle per present().
class IDFRenderTarget : public RGB565BufferRenderTarget {
    bool buffered = true;
    bool dirty = false;
    int dirtyX = 0, dirtyY = 0, dirtyEndX = 0, dirtyEndY = 0;
    int wx = 0, wy = 0;
    uint16_t ww = 0, wh = 0;
    uint32_t cursor = 0;
    void flush(int x, int y, int w, int h) {
        if (w <= 0 || h <= 0) return;
        auto vp = getViewport();
        x += vp.x; y += vp.y;
        int endX = std::min(x + w, int(vp.x + vp.w));
        int endY = std::min(y + h, int(vp.y + vp.h));
        x = std::max(x, int(vp.x)); y = std::max(y, int(vp.y));
        if (x >= endX || y >= endY) return;
        if (!buffered) {
            ESP_ERROR_CHECK(okabe::drawLCD(x, y, endX - x, endY - y, buffer + y * width + x, width));
        } else if (!dirty) {
            dirty = true;
            dirtyX = x; dirtyY = y; dirtyEndX = endX; dirtyEndY = endY;
        } else {
            dirtyX = std::min(dirtyX, x); dirtyY = std::min(dirtyY, y);
            dirtyEndX = std::max(dirtyEndX, endX); dirtyEndY = std::max(dirtyEndY, endY);
        }
    }
    void writeWindow(const uint16_t* data, uint16_t color, uint32_t len, bool swap) {
        if (!ww || !wh) return;
        len = std::min(len, uint32_t(ww) * wh - cursor);
        if (!len) return;
        const uint32_t start = cursor;
        auto vp = getViewport();
        while (len) {
            int col = cursor % ww;
            int row = cursor / ww;
            uint32_t count = std::min(len, uint32_t(ww - col));
            int x = wx + col + vp.x, y = wy + row + vp.y;
            int left = std::max(x, int(vp.x));
            int right = std::min(x + int(count), int(vp.x + vp.w));
            if (y >= vp.y && y < vp.y + vp.h && left < right) {
                auto* dest = buffer + y * width + left;
                if (!data) std::fill_n(dest, right - left, color);
                else if (!swap) memcpy(dest, data + left - x, (right - left) * sizeof(uint16_t));
                else for (int i = left; i < right; ++i) {
                    uint16_t pixel = data[i - x];
                    *dest++ = (pixel >> 8) | (pixel << 8);
                }
            }
            if (data) data += count;
            cursor += count; len -= count;
        }
        int firstRow = start / ww, lastRow = (cursor - 1) / ww;
        if (firstRow == lastRow) flush(wx + start % ww, wy + firstRow, cursor - start, 1);
        else flush(wx, wy + firstRow, ww, lastRow - firstRow + 1);
    }
public:
    IDFRenderTarget() : RGB565BufferRenderTarget(240, 320) { type = RENDER_TARGET_TYPE_SCREEN; }
    void init() override {
        ESP_ERROR_CHECK(okabe::initLCD());
        fillScreen(0);
        present();
    }
    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        RGB565BufferRenderTarget::drawPixel(x, y, (color >> 8) | (color << 8));
        flush(x, y, 1, 1);
    }
    void pushBuffer(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* pixels,
                    bool transparent, uint16_t key) override {
        if (!pixels || w <= 0 || h <= 0) return;
        RGB565BufferRenderTarget::pushBuffer(x, y, w, h, pixels, transparent, key);
        flush(x, y, w, h);
    }
    void fillScreen(uint16_t color) override {
        RGB565BufferRenderTarget::fillScreen((color >> 8) | (color << 8));
        auto vp = getViewport();
        flush(0, 0, vp.w, vp.h);
    }
    void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) override {
        auto vp = getViewport();
        // TFT_STUB has already translated this window to screen coordinates.
        wx = int(x) - vp.x; wy = int(y) - vp.y; ww = w; wh = h; cursor = 0;
    }
    void writeColor(uint16_t color, uint32_t len) override {
        writeWindow(nullptr, (color >> 8) | (color << 8), len, false);
    }
    void pushColors(uint16_t* data, uint32_t len, bool swap = false) override {
        if (data) writeWindow(data, 0, len, swap);
    }
    void setUseBuffer(bool value) override {
        if (buffered && !value) present();
        buffered = value;
    }
    bool getUseBuffer() override { return buffered; }
    void present() override {
        if (!dirty) return;
        ESP_ERROR_CHECK(okabe::drawLCD(dirtyX, dirtyY, dirtyEndX - dirtyX, dirtyEndY - dirtyY,
                                      buffer + dirtyY * width + dirtyX, width));
        dirty = false;
    }
};
