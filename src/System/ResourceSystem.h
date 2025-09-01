#pragma once
#include <SD.h>
#include <TFT_eSPI.h>
#include <vector>
extern TFT_eSprite screen_buffer;
// Removes that alignment of 4 bytes. So 8 bit variable next to 32 bit variable will not take 32 bits
#pragma pack(push, 1)
struct Header {
    const char MAGIC[6] = "NerPh";
    uint8_t    version  = 1;
    uint16_t   imageCount;
    uint16_t   colors[8];
    uint16_t   params[16];
    Header() : MAGIC{'N', 'e', 'r', 'P', 'h', '\0'}, version(0), imageCount(0) {
        for (int i = 0; i < 8; ++i) {
            colors[i] = 0;
        }
        for (int i = 0; i < 16; ++i) {
            params[i] = 0;
        }
    }
};

struct ImageData {
    uint16_t id;
    uint8_t  count;
    uint16_t x, y, width, height;
    uint32_t offset;
    uint8_t  flags;
    // Flags:
    // 0b 0 0 0 0 0 0 0 0
    //    | | | | | | | |
    //    | | | | | | | +-- - Transparent
    //    | | | | | | +---- - Dynamic position (position not applicable)
    //    | | | | | +------ - Reserved (not used)
    //    | | | | +-------- - Reserved (not used)
    //    | | | +---------- - Reserved (not used)
    //    | | +------------ - Reserved (not used)
    //    | +-------------- - Reserved (not used)
    //    +---------------- - Reserved (not used)
    uint16_t transpColor;
    char     name[32];

    ImageData()
        : id(0), count(1), x(0), y(0), width(16), height(16), offset(0), flags(0), transpColor(0), name("") {}
    ImageData(uint16_t id)
        : id(id), count(1), x(0), y(0), width(16), height(16), offset(0), flags(0), transpColor(0), name("") {}

    ImageData(uint16_t id, uint8_t count, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t offset, uint8_t flags, uint16_t transpColor)
        : id(id), count(count), x(x), y(y), width(width), height(height), offset(offset), flags(flags), transpColor(transpColor), name("") {}

    ImageData(uint16_t id, uint8_t count, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t offset, uint8_t flags, uint16_t transpColor, const char *name)
        : id(id), count(count), x(x), y(y), width(width), height(height), offset(offset), flags(flags), transpColor(transpColor) {
        strncpy(this->name, name, sizeof(this->name) - 1);
        this->name[sizeof(this->name) - 1] = '\0';
    }
};
#pragma pack(pop)

struct ImageBuffer {
    uint16_t *pointer;
    bool      freeNeeded;
};

struct Coords {
    int x, y;
};

extern Coords czero;
extern Coords cnone;

enum MainResID {

    R_DEFAULT_WALLPAPER      = 0,
    R_STATUSBAR_BACKGROUND   = 1,
    R_BATTERY_CHARGE         = 2,
    R_SIGNAL_STRENGTH        = 3,
    R_RESERVED1              = 4,
    R_MENU_BACKGROUND        = 5,
    R_MENU_MAIL_ICON         = 6,
    R_MENU_CONTACTS_ICON     = 7,
    R_MENU_E_ICON            = 8,
    R_MENU_SETTINGS_ICON     = 9,
    R_LIST_HEADER_BACKGROUND = 10,
    R_LIST_HEADER_ICONS      = 11,
    R_IN_MSG_MAIL_ICONS      = 12,
    R_LIST_MAIL_ICONS        = 13,
    R_MAIL_MENU_L_HEADER     = 14,
    R_SETTING_MENU_L_HEADER  = 15,
    R_MENU_L_HEADER          = 16,
    R_FULL_NOTIFICATION      = 17,
    R_CTXT_MENU_BACKGROUND   = 18,
    R_LIST_MENU_BACKGROUND   = 19,
    R_RESERVED2              = 20,
    R_CALL_ANIM_DOTS         = 21,
    R_VOICE_ONLY_LABEL       = 22,
    R_PHONE_ICON             = 23,
    R_PHONE_ICON_LIGHTNING   = 24,
    R_FILE_MANAGER_ICONS     = 25,
    R_RESERVED3              = 26,
    R_BOOT_LOGO              = 27,
    R_NULL_IMAGE             = 0xFFFF,

};

enum ResourceType {
    RES_MAIN       = 0,
    RES_WALLPAPERS = 1,
};

class ResourceSystem {
  public:
    Header                 Headers[2];
    std::vector<ImageData> Images[2];
    File                   Files[2];
    uint8_t               *cache[2];

    ImageData GetImageDataByID(uint16_t id, uint8_t type = RES_MAIN);

    void        CopyToRam(uint8_t type = RES_MAIN);
    void        Init(File Main, File Wallpapers = File());
    Coords      GetCoordsByID(uint16_t id, uint8_t type = RES_MAIN);
    bool        DrawImage(uint16_t id, uint8_t index = 0, Coords xy = {-1, -1}, Coords startpos = {0, 0}, Coords endpos = {0, 0}, uint8_t type = RES_MAIN, bool is_screen_buffer = false, TFT_eSprite &sbuffer = screen_buffer);
    bool        DrawImage(uint16_t id, uint8_t index, bool is_screen_buffer, TFT_eSprite &sbuffer = screen_buffer);
    ImageBuffer GetRGB565(ImageData img, size_t size = 0, uint32_t start = 0, uint8_t type = RES_MAIN);

  private:
    const char *names[3] = {
        "Main", "Wallpapers", "Fonts"};
    void failure(const char *msg = "Unknown Error", bool important = false);
    void parseResourceFile(File file, Header &header, uint8_t type = RES_MAIN, bool important = false);
};
void                  drawWallpaper();
extern ResourceSystem res;