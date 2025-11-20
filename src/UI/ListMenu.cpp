#include "ListMenu.h"
#include "../Platform/ESP32Memory.h"

#include "../Platform/Graphics/RGB565BufferRenderTarget.h"
#include "../System/FontManagement.h"

bool          lm_buffer = LISTMENU_BUFFER;
RenderTarget *lm_buffer_obj;

void pushBufferToScreen(int x, int y) {
    currentRenderTarget->pushBuffer(x, y, lm_buffer_obj->getWidth(), lm_buffer_obj->getHeight(),
                                    lm_buffer_obj->getBuffer(), 0, 0);

    currentRenderTarget->present();
}
void deleteBuffer() {
    if (lm_buffer_obj->getType() == RENDER_TARGET_TYPE_BUFFER) {
        lm_buffer_obj->~RenderTarget();
        lm_buffer_obj = nullptr;
        tft.setRenderTarget(currentRenderTarget);
    }
}

// Header for the list menu
// @param type Type of menu (0: messages, 1: contacts, 2: settings)
// @param title Title of the menu
// @param page Current page number
// @param pages Total number of pages
// @param y Y-coordinate for the header
void listMenu_header(int type, NString title, int page, int pages, int y) {
    res.DrawImage(R_LIST_HEADER_BACKGROUND, 0, {OP_UNDEF, y}, {0, 0}, {0, 0}, RES_MAIN);
    res.DrawImage(R_LIST_HEADER_ICONS, type, {OP_UNDEF, y}, {0, 0}, {0, 0}, RES_MAIN);

    changeFont(1);

    tft.setTextColor(0xFFFF);
    tft.setCursor(28, y + 19);
    tft.print(title);

    if (pages > 1) {
        changeFont(0);
        tft.setCursor(210, y + 15);
        tft.printf("%d/%d", page + 1, pages);
    }
}

/// Function to display a single entry in the list menu
/// @param lindex Index of the entry
/// @param x X-coordinate for the entry
/// @param y Y-coordinate for the entry
/// @param choice mOption object representing the menu option
/// @param esize Entry size
/// @param lines Boolean indicating if lines should be drawn between options
/// @param selected Boolean indicating if the entry is selected
/// @param unselected Boolean indicating if the entry is unselected (I dont fucking remember why
/// its not just selected = false)
void listMenu_entry(int lindex, int x, int y, mOption choice, int esize, bool lines, bool selected,
                    bool unselected) {
    uint16_t color_active = 0xFDD3;

    int yy = (lindex * esize) + y;

    if (selected) { tft.fillRect(0, yy, 240, esize, color_active); }
    else if (unselected) {
        res.DrawImage(R_LIST_MENU_BACKGROUND, 0, {.x = 0, .y = yy}, {.x = 0, .y = yy},
                      {.x = 0, .y = yy + esize}, RES_MAIN);
    }

    ImageData imgData = res.GetImageDataByImage(choice.image);
    res.DrawImage(choice.image, choice.icon_index, {x - imgData.width, yy}, {0, 0}, {0, 0},
                  choice.image.type);
    if (lines) {
        tft.drawLine(0, yy, 240, yy, 0);
        tft.drawLine(0, yy + esize, 240, yy + esize, 0);
    }
    changeFont(1);
    tft.setTextColor(0);
    tft.setCursor(x + 3, yy + 17);
    tft.print(choice.label);
}

/// Function to display a list menu
/// @param choices Array of mOption objects representing the menu options
/// @param icount Number of options in the menu
/// @param lines Boolean indicating if lines should be drawn between options
/// @param type Icon to display in the header 0: `messages` 1: `phone` 2: `settings`
/// @param label Title of the menu
/// @param forceIcons Boolean indicating if icons should be forced
/// @param findex Force index of the selected option
int listMenu(std::vector<mOption> choices, int icount, bool lines, int type, NString label,
             bool forceIcons, int findex) {
    lm_buffer_obj = lm_buffer ? new RGB565BufferRenderTarget(240, 294) : currentRenderTarget;
    tft.resetViewport();
    const int bufOffset = 26;
    if (!lm_buffer) { tft.setViewport(0, bufOffset, 240, 294); }
    else { tft.setRenderTarget(lm_buffer_obj); }
    tft.setTextWrap(false, false);
    tft.setTextColor(0);
    tft.setTextSize(1);
    changeFont(2);

    int selected     = 0;
    int page         = 0;
    int pages        = 0;
    int y            = 0;
    int ly           = 25;
    int x            = 10;
    int old_selected = 0;
    res.DrawImage(R_LIST_MENU_BACKGROUND, 0);
    if (icount == 0) {
        listMenu_header(type, label, 0, 0, y);
        changeFont(1);
        tft.setTextColor(0);
        tft.setCursor(75, 45);
        tft.print("< Empty >");

        if (lm_buffer) { pushBufferToScreen(0, bufOffset); }

        while (true) {
            int bh = buttonsHelding();
            if (bh == SELECT) {
                tft.resetViewport();
                return LISTMENU_OPTIONS;
            }
            else if (bh != -1) {
                tft.resetViewport();
                return LISTMENU_EXIT;
            }
        };
        tft.resetViewport();
        return -1;
    }
    int entry_size = tft.fontHeight();

    if (choices[0].image.type != RES_NULLU8) {
        ImageData icon = res.GetImageDataByImage(choices[0].image);
        if (icon.height > entry_size) { entry_size = icon.height; }
        x += icon.width;
    }
    else if (forceIcons) { x += entry_size; }
    selected = findex;

    int per_page = 269 / entry_size;
    pages        = (icount + per_page - 1) / per_page;

    listMenu_header(type, label, page, pages, y);
    int startIndex = page * per_page;
    int endIndex   = std::min(startIndex + per_page, icount);

    for (int i = 0; i < (endIndex - startIndex); i++) {
        listMenu_entry(i, x, y + ly, choices[startIndex + i], entry_size, lines, false, false);
    }
    listMenu_entry(selected, x, y + ly, choices[selected + (page * per_page)], entry_size, lines,
                   true, false);
    pushBufferToScreen(0, bufOffset);

    bool exit                = false;
    int  total_items_on_page = 0;
    while (!exit) {
        int c = buttonsHelding();

        switch (c) {
        case SELECT:
            pushBufferToScreen(0, bufOffset);
            deleteBuffer();
            tft.resetViewport();
            return selected + (page * per_page);
            break;
        case BACK:
            pushBufferToScreen(0, bufOffset);
            deleteBuffer();
            tft.resetViewport();
            return LISTMENU_EXIT;
            break;

        case UP:
            old_selected = selected;
            selected--;

            if (selected < 0) {
                if (page > 0) {
                    page--;
                    selected = per_page - 1;
                }
                else {
                    page     = pages - 1;
                    selected = (icount % per_page == 0) ? per_page - 1 : (icount % per_page) - 1;
                }

                listMenu_header(type, label, page, pages, y);
                res.DrawImage(R_LIST_MENU_BACKGROUND);

                int startIndex = page * per_page;
                int endIndex   = std::min(startIndex + per_page, icount);

                for (int i = 0; i < (endIndex - startIndex); i++) {
                    listMenu_entry(i, x, y + ly, choices[startIndex + i], entry_size, lines, false,
                                   false);
                }
            }
            else {

                listMenu_entry(old_selected, x, y + ly, choices[old_selected + (page * per_page)],
                               entry_size, lines, false, true);
            }

            listMenu_entry(selected, x, y + ly, choices[selected + (page * per_page)], entry_size,
                           lines, true, false);
            pushBufferToScreen(0, bufOffset);

            break;

        case DOWN:

            old_selected = selected;
            selected++;

            total_items_on_page = std::min(per_page, icount - (page * per_page));

            if (selected >= total_items_on_page) {
                if (page < pages - 1) {
                    page++;
                    selected = 0;
                }
                else {
                    page     = 0;
                    selected = 0;
                }

                res.DrawImage(R_LIST_MENU_BACKGROUND);
                listMenu_header(type, label, page, pages, y);
                int startIndex = page * per_page;
                int endIndex   = std::min(startIndex + per_page, icount);

                for (int i = 0; i < (endIndex - startIndex); i++) {
                    listMenu_entry(i, x, y + ly, choices[startIndex + i], entry_size, lines, false,
                                   false);
                }
            }
            else {

                listMenu_entry(old_selected, x, y + ly, choices[old_selected + (page * per_page)],
                               entry_size, lines, false, true);
            }

            listMenu_entry(selected, x, y + ly, choices[selected + (page * per_page)], entry_size,
                           lines, true, false);

            pushBufferToScreen(0, bufOffset);

            break;
        case RIGHT:
            if (pages > 1) {
                page     = (page + 1) % pages;
                selected = 0;
                res.DrawImage(R_LIST_MENU_BACKGROUND);
                listMenu_header(type, label, page, pages, y);

                int startIndex = page * per_page;
                int endIndex   = std::min(startIndex + per_page, icount);

                for (int i = 0; i < (endIndex - startIndex); i++) {
                    listMenu_entry(i, x, y + ly, choices[startIndex + i], entry_size, lines, false,
                                   false);
                }
                listMenu_entry(selected, x, y + ly, choices[selected + (page * per_page)],
                               entry_size, lines, true, false);
                pushBufferToScreen(0, bufOffset);
            }
            break;
        case LEFT:
            if (pages > 1) {
                page     = (page - 1 + pages) % pages;
                selected = 0;
                res.DrawImage(R_LIST_MENU_BACKGROUND);
                listMenu_header(type, label, page, pages, y);

                int startIndex = page * per_page;
                int endIndex   = std::min(startIndex + per_page, icount);

                for (int i = 0; i < (endIndex - startIndex); i++) {
                    listMenu_entry(i, x, y + ly, choices[startIndex + i], entry_size, lines, false,
                                   false);
                }
                listMenu_entry(selected, x, y + ly, choices[selected + (page * per_page)],
                               entry_size, lines, true, false);
                pushBufferToScreen(0, bufOffset);
            }
            break;
        }
    }
    deleteBuffer();

    tft.resetViewport();
    return LISTMENU_EXIT;
}

// Converter from old type of listMenu to new
// @param choices Array of strings representing the menu options
// @param icount Number of options in the menu
// @param images Boolean indicating if images should be used
// @param type Icon to display in the header 0: `messages` 1: `phone` 2: `settings`
// @param label Title of the menu
// @param forceIcons Boolean indicating if icons should be forced
// @param findex Index of the selected option
int listMenu(const NString choices[], int icount, bool images, int type, NString label,
             bool forceIcons, int findex) {
    std::vector<mOption> optionArr;
    for (int i = 0; i < icount; i++) {
        mOption option = mOption(choices[i], Image(), 0);
        optionArr.push_back(option);
    }
    int result = listMenu(optionArr, icount, images, type, label, forceIcons, findex);
    return result;
}

int lmng_offset = 0;
// Render entry of listMenuNonGraphical()
// @param choices Array of mOption objects representing the menu options
// @param x X-coordinate for the entry
// @param y Y-coordinate for the entry
// @param icount Number of options in the menu
// @param label Title of the menu
// @param index Index of the selected option
// @param color_active Color for the active option
// @param color_inactive Color for the inactive options
void renderlmng(std::vector<mOption> choices, int x, int y, int icount, NString label, int index,
                uint16_t color_active, uint16_t color_inactive) {
    int max_per_page = ((320 - y - tft.fontHeight()) / tft.fontHeight());

    if (index >= max_per_page + lmng_offset) {
        tft.fillRect(x, y, 240 - x, 320 - y, 0);
        tft.fillScreen(0);
    }
    else if (index < lmng_offset) {
        lmng_offset = index;
        tft.fillRect(x, y, 240 - x, 320 - y, 0);
    }
    tft.setCursor(x, y);
    tft.setTextColor(color_inactive);
    tft.println(label + "  ");

    for (int i = lmng_offset; i < max_per_page + lmng_offset; i++) {
        if (i == index) { tft.setTextColor(color_active); }
        else { tft.setTextColor(color_inactive); }
        if (i < icount) { tft.println("  " + choices[i].label); }
    }
}

// listMenu that doesn't use images
// @param choices Array of mOption objects representing the menu options
// @param icount Number of options in the menu
// @param label Title of the menu
// @param y Y-coordinate for the menu
// @return Index of the selected option
// @note This function is used for non-graphical list menus
int listMenuNonGraphical(std::vector<mOption> choices, int icount, NString label, int y) {
    suspendCore(true);
    int x     = 0;
    int index = 0;
    changeFont(0);
    tft.fillRect(x, y, 240 - x, 320 - y, 0);
    // uint     max_per_page   = ((320 - y - tft.fontHeight()) / tft.fontHeight());
    uint16_t color_active   = TFT_RED;
    uint16_t color_inactive = TFT_WHITE;
    tft.setTextColor(color_inactive);
    renderlmng(choices, x, y, icount, label, index, color_active, color_inactive);

    bool exit = false;
    while (!exit) {
        int c = buttonsHelding(false);
        switch (c) {
        case UP:
            index--;
            if (index < 0) { index = icount - 1; }
            renderlmng(choices, x, y, icount, label, index, color_active, color_inactive);
            break;
        case DOWN:
            index++;
            if (index >= icount) { index = 0; }
            renderlmng(choices, x, y, icount, label, index, color_active, color_inactive);
            break;
        case SELECT:
            suspendCore(false);
            return index;
            break;
        case BACK:
            suspendCore(false);
            return -1;
            break;
        }
    }

    suspendCore(false);
    return -1;
}

// mini list menu
// @param choices Array of strings representing the menu options
// @param count Number of options in the menu
// @param context Boolean indicating if the menu is in context
int choiceMenu(const NString choices[], int count, bool context) {
    int      x              = 30;
    int      y              = 95;
    int      mul            = 20;
    uint16_t color_active   = 0xF9C0;
    uint16_t color_inactive = 0x0000;

    if (context) {
        res.DrawImage(R_CTXT_MENU_BACKGROUND);
        // drawImage(16, 100, CONTEXT_MENU_IMAGE);
        x = 40;
        y = 120;
    }
    else {
        // drawImage(0, 68, WHITE_BOTTOM_IMAGE);
        res.DrawImage(R_MENU_L_HEADER);
        x = 30;
        y = 95;
    }

    tft.setTextSize(1);
    tft.setTextColor(color_inactive);
    changeFont(1);

    for (int i = 0; i < count; i++) {
        tft.setCursor(x, y + (mul * i));
        tft.print(choices[i]);
    }

    int choice = 0;
    tft.setTextColor(color_active);
    tft.setCursor(x, y + (mul * choice));
    tft.print(choices[choice]);
    int yy = (y + (mul * choice)) - 10;
    int xx = x - 5;
    tft.fillTriangle(xx - 10, yy, xx - 10, yy + 10, xx - 2, yy + 5, color_active);
    bool exit = false;
    while (!exit) {
        switch (buttonsHelding()) {
        case BACK: {
            exit = true;

            return -1;
            break;
        }
        case UP: {
            // left
            if (context) {
                tft.setTextSize(1);
                tft.setTextColor(color_inactive);

                res.DrawImage(R_CTXT_MENU_BACKGROUND);
                for (int i = 0; i < count; i++) {
                    tft.setCursor(x, y + (mul * i));
                    tft.print(choices[i]);
                }
            }
            else {
                tft.setTextSize(1);
                tft.setTextColor(color_inactive);

                res.DrawImage(R_MENU_L_HEADER);
                for (int i = 0; i < count; i++) {
                    tft.setCursor(x, y + (mul * i));
                    tft.print(choices[i]);
                }
            }
            if (choice < 1) { choice = count - 1; }
            else { choice--; }
            tft.setTextColor(color_inactive);
            tft.setCursor(x, y + (mul * (choice + 1)));
            if (choice < count - 1) { tft.print(choices[choice + 1]); }
            if (choice == count - 1) {
                tft.setCursor(x, y);
                tft.print(choices[0]);
            }
            tft.setTextColor(color_active);
            tft.setCursor(x, y + (mul * choice));
            int yy = (y + (mul * choice)) - 10;
            int xx = x - 5;
            tft.fillTriangle(xx - 10, yy, xx - 10, yy + 10, xx - 2, yy + 5, color_active);
            tft.print(choices[choice]);
            delay(100);
            break;
        }
        case SELECT: {
            // middle

            exit = true;
            return choice;
            break;
        }
        case DOWN: {
            // right
            if (context) {
                tft.setTextSize(1);
                tft.setTextColor(color_inactive);

                res.DrawImage(R_CTXT_MENU_BACKGROUND);
                for (int i = 0; i < count; i++) {
                    tft.setCursor(x, y + (mul * i));
                    tft.print(choices[i]);
                }
            }
            else {
                tft.setTextSize(1);
                tft.setTextColor(color_inactive);

                res.DrawImage(R_MENU_L_HEADER);
                for (int i = 0; i < count; i++) {
                    tft.setCursor(x, y + (mul * i));
                    tft.print(choices[i]);
                }
            }
            if (choice > count - 2) { choice = 0; }
            else { choice++; }

            tft.setTextColor(color_inactive);
            tft.setCursor(x, y + (mul * (choice - 1)));

            if (choice > 0) { tft.print(choices[choice - 1]); }

            if (choice == 0) {
                tft.setCursor(x, y + (mul * (count - 1)));
                tft.print(choices[count - 1]);
            }
            tft.setTextColor(color_active);
            tft.setCursor(x, y + (mul * choice));
            int yy = (y + (mul * choice)) - 10;
            int xx = x - 5;
            tft.fillTriangle(xx - 10, yy, xx - 10, yy + 10, xx - 2, yy + 5, color_active);
            tft.print(choices[choice]);

            delay(100);
            break;
        }
        }
    }
    return -1;
}
