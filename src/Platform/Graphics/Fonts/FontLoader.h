#include "gfxfont.h"
#include <Platform/FileSystem/VFS.h>
#include <Platform/NString.h>
#include <deque>
struct FontHolder {
    NString                    path;
    NFile*                     file;
    std::vector<GFXfontPacked> font;
};
struct LoadedGlyph {
    uint16_t       charCode;
    GFXglyphPacked glyphData;
    uint8_t*       bitmapData;
    FontHolder*    fontHolder;
};

void         loadFont(const NString& path);
void         unloadFont(const NString& path);
LoadedGlyph* getGlyphData(const NString& path, uint16_t charCode);
int          getFontYAdvance(const NString& path);
FontHolder*  getFontHolder(const NString& path);