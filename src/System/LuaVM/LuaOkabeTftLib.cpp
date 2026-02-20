#include "LuaOkabeLib.h"
#include "lua/lua.hpp"
#include <GlobalVariables.h>

extern "C" {
static int tft_fillscreen(lua_State *L) {
    lua_Integer color = luaL_checkinteger(L, 1);
    tft.fillScreen(color);
    return 0;
}

static int tft_screensize(lua_State *L) {
    int16_t w = tft.width();
    int16_t h = tft.height();

    lua_pushinteger(L, w);
    lua_pushinteger(L, h);
    return 2;
}

static int tft_setcursor(lua_State *L) {
    lua_Integer x = luaL_checkinteger(L, 1);
    lua_Integer y = luaL_checkinteger(L, 2);

    tft.setCursor(x, y);
    return 0;
}

static int tft_movecursor(lua_State *L) {
    lua_Integer x    = luaL_checkinteger(L, 1);
    lua_Integer y    = luaL_checkinteger(L, 2);
    int         oldx = tft.getCursorX();
    int         oldy = tft.getCursorY();

    tft.setCursor(oldx + x, oldy + y);
    return 0;
}

static int tft_getcursor(lua_State *L) {
    int x = tft.getCursorX();
    int y = tft.getCursorY();

    lua_pushinteger(L, x);
    lua_pushinteger(L, y);

    return 2;
}

static int tft_settextcolor(lua_State *L) {
    lua_Integer color = luaL_checkinteger(L, 1);
    ESP_LOGI("LuaTftLib", "C White: %i", TFT_WHITE);
    ESP_LOGI("LuaTftLib", "Lua White: %i", color);
    tft.setTextColor(color);
    return 0;
}

static int tft_settextfont(lua_State *L) {
    lua_Integer fontid = luaL_checkinteger(L, 1);
    tft.setTextFont(fontid);
    return 0;
}

static int tft_set_text_size(lua_State *L) {
    int size = luaL_checkint(L, 1);
    ESP_LOGI("LuaTftLib", "set_text_size: %i", size);
    tft.setTextSize(size);
    return 0;
}

static int tft_drawpixel(lua_State *L) {
    lua_Integer x     = luaL_checkinteger(L, 1);
    lua_Integer y     = luaL_checkinteger(L, 2);
    lua_Integer color = luaL_checkinteger(L, 3);
    tft.drawPixel(x, y, color);
    return 0;
}

static int tft_drawrect(lua_State *L) {
    lua_Integer x     = luaL_checkinteger(L, 1);
    lua_Integer y     = luaL_checkinteger(L, 2);
    lua_Integer w     = luaL_checkinteger(L, 3);
    lua_Integer h     = luaL_checkinteger(L, 4);
    lua_Integer color = luaL_checkinteger(L, 5);

    tft.drawRect(x, y, w, h, color);

    return 0;
}

static int tft_fillrect(lua_State *L) {
    lua_Integer x     = luaL_checkinteger(L, 1);
    lua_Integer y     = luaL_checkinteger(L, 2);
    lua_Integer w     = luaL_checkinteger(L, 3);
    lua_Integer h     = luaL_checkinteger(L, 4);
    lua_Integer color = luaL_checkinteger(L, 5);

    tft.fillRect(x, y, w, h, color);

    return 0;
}

static int tft_drawline(lua_State *L) {
    lua_Integer x0    = luaL_checkinteger(L, 1);
    lua_Integer y0    = luaL_checkinteger(L, 2);
    lua_Integer x1    = luaL_checkinteger(L, 3);
    lua_Integer y1    = luaL_checkinteger(L, 4);
    lua_Integer color = luaL_checkinteger(L, 5);

    tft.drawLine(x0, y0, x1, y1, color);

    return 0;
}

static int tft_filltriangle(lua_State *L) {
    lua_Integer x0    = luaL_checkinteger(L, 1);
    lua_Integer y0    = luaL_checkinteger(L, 2);
    lua_Integer x1    = luaL_checkinteger(L, 3);
    lua_Integer y1    = luaL_checkinteger(L, 4);
    lua_Integer x2    = luaL_checkinteger(L, 5);
    lua_Integer y2    = luaL_checkinteger(L, 6);
    lua_Integer color = luaL_checkinteger(L, 7);

    tft.fillTriangle(x0, y0, x1, y1, x2, y2, color);

    return 0;
}

static int tft_simple_print(lua_State *L, int isnewline) {
    int n = lua_gettop(L);
    int i;
    for (i = 1; i <= n; i++) {
        // if (i>1) printf("\t");
        if (lua_isstring(L, i)) { tft.printf("%s", lua_tostring(L, i)); }
        else if (lua_isnil(L, i)) { tft.printf("%s", "nil"); }
        else if (lua_isboolean(L, i)) { tft.printf("%s", lua_toboolean(L, i) ? "true" : "false"); }
        else { tft.printf("%s:%p", luaL_typename(L, i), lua_topointer(L, i)); }
    }

    if (isnewline) { tft.printf("\n"); }

    return 0;
}

static int tft_print(lua_State *L) { return tft_simple_print(L, 0); }

static int tft_println(lua_State *L) { return tft_simple_print(L, 1); }

static int tft_present(lua_State *L) {
    currentRenderTarget->present();
    return 0;
}

struct lua_colors {
    const char *name;
    const int   value;
};

static const struct lua_colors TFT_COLORS[] = {
    {"black", 0x0000},    {"navy", 0x000F},        {"darkgreen", 0x03E0}, {"darkcyan", 0x03EF},
    {"maroon", 0x7800},   {"purple", 0x780F},      {"olive", 0x7BE0},     {"lightgrey", 0xD69A},
    {"darkgrey", 0x7BEF}, {"blue", 0x001F},        {"green", 0x07E0},     {"cyan", 0x07FF},
    {"red", 0xF800},      {"magenta", 0xF81F},     {"yellow", 0xFFE0},    {"white", 0xFFFF},
    {"orange", 0xFDA0},   {"greenyellow", 0xB7E0}, {"pink", 0xFE19},      {"brown", 0x9A60},
    {"gold", 0xFEA0},     {"silver", 0xC618},      {"skyblue", 0x867D},   {"violet", 0x915C},
    {NULL, NULL}};

static const struct luaL_Reg tftlib[] = {{"fillscreen", tft_fillscreen},
                                         {"screensize", tft_screensize},
                                         {"setcursor", tft_setcursor},
                                         {"movecursor", tft_movecursor},
                                         {"getcursor", tft_getcursor},
                                         {"set_text_color", tft_settextcolor},
                                         {"set_text_font", tft_settextfont},
                                         {"set_text_size", tft_set_text_size},
                                         {"set_text_size", tft_set_text_size},
                                         {"drawrect", tft_drawrect},
                                         {"fillrect", tft_fillrect},
                                         {"drawline", tft_drawline},
                                         {"filltriangle", tft_filltriangle},
                                         {"print", tft_print},
                                         {"println", tft_println},
                                         {"present", tft_present},
                                         {NULL, NULL}};

int luaopen_tft(lua_State *L) {
    luaL_register(L, OKABE_TFTLIBNAME, tftlib);

    lua_newtable(L);
    const struct lua_colors *c = TFT_COLORS;
    for (; c->name; c++) {
        lua_pushstring(L, c->name);
        lua_pushinteger(L, c->value);
        lua_settable(L, -3);
    }

    lua_setfield(L, -2, "color");
    return 1;
}
}
