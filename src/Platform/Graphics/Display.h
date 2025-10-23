#ifndef OKABE_TFT_ESPI_STUB_H
#define OKABE_TFT_ESPI_STUB_H
#include "../NString.h"
#include "Defines.h"
#include "Fonts/Arial.h"
#include "RenderTargets.h"
#include <stdarg.h>
typedef struct {
    const uint8_t *data;
    uint8_t        width;
} gchar_t;
typedef struct {
    gchar_t *data;
    uint8_t  width;
    uint8_t  height;
} font_t;

// color definitions
#define TFT_BLACK       0x0000                     /*   0,   0,   0 */
#define TFT_NAVY        0x000F                     /*   0,   0, 128 */
#define TFT_DARKGREEN   0x03E0                     /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF                     /*   0, 128, 128 */
#define TFT_MAROON      0x7800                     /* 128,   0,   0 */
#define TFT_PURPLE      0x780F                     /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0                     /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xD69A                     /* 211, 211, 211 */
#define TFT_DARKGREY    0x7BEF                     /* 128, 128, 128 */
#define TFT_BLUE        0x001F                     /*   0,   0, 255 */
#define TFT_GREEN       0x07E0                     /*   0, 255,   0 */
#define TFT_CYAN        0x07FF                     /*   0, 255, 255 */
#define TFT_RED         0xF800                     /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F                     /* 255,   0, 255 */
#define TFT_YELLOW      0xFFE0                     /* 255, 255,   0 */
#define TFT_WHITE       0xFFFF                     /* 255, 255, 255 */
#define TFT_ORANGE      0xFDA0                     /* 255, 180,   0 */
#define TFT_GREENYELLOW 0xB7E0                     /* 180, 255,   0 */
#define TFT_PINK        0xFE19 /* 255, 192, 203 */ // Lighter pink, was 0xFC9F
#define TFT_BROWN       0x9A60                     /* 150,  75,   0 */
#define TFT_GOLD        0xFEA0                     /* 255, 215,   0 */
#define TFT_SILVER      0xC618                     /* 192, 192, 192 */
#define TFT_SKYBLUE     0x867D                     /* 135, 206, 235 */
#define TFT_VIOLET      0x915C                     /* 180,  46, 226 */

// Minimal, platform-agnostic stub of TFT_eSPI used for porting/testing.
// Implement or extend methods as needed by your project.
class TFT_STUB {
  public:
    TFT_STUB(int16_t w = 240, int16_t h = 320);

    RenderTarget *activeRenderTarget = nullptr;
    void          setRenderTarget(RenderTarget *target);

    // Font rendering
    void     renderGlyph(char c, int16_t x, int16_t y);
    uint16_t textcolor = TFT_WHITE; // Text foreground color
    uint8_t  textsize  = 1;         // Font size multiplier

    // init/begin
    void init(uint8_t tabColour = 0);
    void begin(uint8_t tabColour = 0) { init(tabColour); }
    // basic drawing
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color);
    virtual void fillScreen(uint16_t color);
    virtual void pushImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data);
    // window / pixel push (no-op default)
    virtual void setAddrWindow(uint16_t xs, uint16_t ys, uint16_t w, uint16_t h);
    virtual void pushColors(const uint16_t *data, int16_t len, bool swap = false);
    // rotation/origin
    void    setRotation(uint8_t r);
    uint8_t getRotation() const;

    // viewport support: define a drawing sub-region (x, y, w, h).
    // When a viewport is active, drawing coordinates passed to drawPixel,
    // setAddrWindow and similar functions are interpreted as coordinates
    // relative to the viewport origin (0..w-1, 0..h-1). The implementation
    // translates and clips to the absolute display coordinates.
    void setViewport(int16_t x, int16_t y, int16_t w, int16_t h);
    void resetViewport();
    void pushImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data,
                   uint16_t transpColor);
    // size getters
    int16_t width() const;
    int16_t height() const;

    // simple colour helpers
    uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) const;
    void     drawchar(int16_t x, int16_t y, char c, font_t font, uint16_t color, uint16_t bg,
                      uint8_t size);
    // text helpers (minimal)
    void setCursor(int16_t x, int16_t y);

    void printf(const char *fmt, ...);
    void print(const char *str);
    void println(const char *str);

    // Generic print/println to accept Arduino NString, NStringSumHelper and
    // std::string without depending on Arduino NString implementation.

    // keep std::string overloads declared below (definitions in .cpp)

    // Specific overloads to avoid ambiguous conversions
    void print(char c);
    void println(char c);
    // std::string overloads were removed to avoid overload conflicts; use
    // print(const char*) or print(NString) instead.
    // print/println for Arduino `NString` or our compat `NString` type
    void print(const NString &s);
    void println(const NString &s);

    // Text wrapping
    bool textwrap = true;

    // richer text API used in project
    void setTextColor(uint16_t color, uint16_t bg = 0, bool opaque = false);
    void setTextSize(uint8_t size);
    void setTextFont(uint8_t font);
    void setTextWrap(bool wrap, bool clip = true);
    int  getCursorX() const;
    int  getCursorY() const;

    // viewport getters used throughout the project
    int  getViewportWidth() const;
    int  getViewportHeight() const;
    int  getViewportX() const;
    int  getViewportY() const;
    bool getViewportDatum() const;

    // drawing primitives used across the codebase
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                      uint16_t color);

    // simple text measurements used in layout
    int textWidth(const std::string &s) const;
    int fontHeight() const;

    // attribute/settings hooks
    void setAttribute(int attr, bool value);

  protected:
    int16_t _init_w, _init_h;
    int16_t _w, _h;
    uint8_t _rotation;
    int16_t  _addrX1, _addrY1, _addrX2, _addrY2;
    int16_t  _cursor_x, _cursor_y;
    uint16_t _textcolor;
    // viewport state
    int16_t _vp_x, _vp_y, _vp_w, _vp_h;
    bool    _vp_active;
    bool    _vp_datum = false;
};

#endif // OKABE_TFT_ESPI_STUB_H