#include "ListMenu.h"
#include "../System/DrawGraphics.h"
#include "../System/FontManagement.h"
#include "../images.h"

// Header for the list menu
// @param type Type of menu (0: messages, 1: phone, 2: settings)
// @param title Title of the menu
// @param page Current page number
// @param pages Total number of pages
// @param y Y-coordinate for the header
void listMenu_header(int type, String title, int page, int pages, int y) {
    drawImage(0, y, BLUEBAR_IMAGE, true);
    drawImage(0, y, BLUEBAR_ICONS[type], true);
    screen_buffer.setTextColor(0xFFFF);
    changeFont(1, 1);
    screen_buffer.setCursor(28, y + 19);
    screen_buffer.print(title);
    if (pages > 1) {
        changeFont(0, 1);
        screen_buffer.setCursor(210, y + 15);
        screen_buffer.printf("%d/%d", page + 1, pages);
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
/// @param unselected Boolean indicating if the entry is unselected (I dont fucking remember why its not just selected = false)
void listMenu_entry(int lindex, int x, int y, mOption choice, int esize, bool lines, bool selected, bool unselected) {
    uint16_t color_active = 0xFDD3;

    int yy = (lindex * esize) + y;

    if (selected)
        screen_buffer.fillRect(0, yy, 240, esize, color_active);
    else if (unselected)
        drawImage(0, yy, SDImage(BACKGROUND_IMAGE.address + (yy * 240 * 2), 240, esize), true);
    drawImage(x - choice.icon.w, yy, choice.icon, true);

    if (lines) {
        screen_buffer.drawLine(0, yy, 240, yy, 0);
        screen_buffer.drawLine(0, yy + esize, 240, yy + esize, 0);
    }
    screen_buffer.setTextColor(0);
    screen_buffer.setCursor(x + 3, yy + 17);
    changeFont(1, 1);
    screen_buffer.print(choice.label);
}

/// Function to display a list menu
/// @param choices Array of mOption objects representing the menu options
/// @param icount Number of options in the menu
/// @param lines Boolean indicating if lines should be drawn between options
/// @param type Icon to display in the header 0: `messages` 1: `phone` 2: `settings`
/// @param label Title of the menu
/// @param forceIcons Boolean indicating if icons should be forced
/// @param findex Index of the selected option
int listMenu(mOption *choices, int icount, bool lines, int type, String label, bool forceIcons, int findex) {
    screen_buffer.setTextWrap(false, false);

    screen_buffer.createSprite(240, 294);
    screen_buffer.setTextSize(1);
    screen_buffer.setTextColor(0);

    changeFont(2, 1);
    int selected     = 0;
    int page         = 0;
    int pages        = 0;
    int y            = 0;
    int ly           = 25;
    int x            = 10;
    int old_selected = 0;
    drawImage(0, y + 25, SDImage(BACKGROUND_IMAGE.address + 0x2EE0, 240, 269), true);

    if (icount == 0) {
        listMenu_header(type, label, 0, 0, y);
        screen_buffer.setTextColor(0);
        changeFont(1, 1);
        screen_buffer.setCursor(75, 45);
        screen_buffer.print("< Empty >");
        screen_buffer.pushSprite(0, 26);
        screen_buffer.deleteSprite();
        while (buttonsHelding() != BACK)
            ;
        return -1;
    }
    int entry_size = screen_buffer.fontHeight();
    if (choices[0].icon.h > entry_size) {
        entry_size = choices[0].icon.h;
    }
    x += choices[0].icon.w;
    int per_page = 269 / entry_size;
    pages        = (icount + per_page - 1) / per_page;

    listMenu_header(type, label, page, pages, y);
    int startIndex = page * per_page;
    int endIndex   = std::min(startIndex + per_page, icount);

    for (int i = 0; i < (endIndex - startIndex); i++) {
        listMenu_entry(i, x, y + ly, choices[startIndex + i], entry_size, lines, false, false);
    }
    listMenu_entry(selected, x, y + ly, choices[selected + (page * per_page)], entry_size, lines, true, false);
    screen_buffer.pushSprite(0, 26);
    bool exit                = false;
    int  total_items_on_page = 0; // Move declaration outside the switch statement
    while (!exit) {
        int c = buttonsHelding();

        switch (c) {
        case SELECT:
            screen_buffer.deleteSprite();

            return selected + (page * per_page);
            break;
        case BACK:
            screen_buffer.deleteSprite();
            return -1;
            break;

        case UP:
            old_selected = selected;
            selected--;

            if (selected < 0) {
                if (page > 0) {

                    page--;
                    selected = per_page - 1;
                } else {

                    page     = pages - 1;
                    selected = (icount % per_page == 0) ? per_page - 1 : (icount % per_page) - 1;
                }

                drawImage(0, y + 25, SDImage(BACKGROUND_IMAGE.address + 0x2EE0, 240, 269), true);
                listMenu_header(type, label, page, pages, y);

                int startIndex = page * per_page;
                int endIndex   = std::min(startIndex + per_page, icount);

                for (int i = 0; i < (endIndex - startIndex); i++) {
                    listMenu_entry(i, x, y + ly, choices[startIndex + i], entry_size, lines, false, false);
                }
            } else {

                listMenu_entry(old_selected, x, y + ly, choices[old_selected + (page * per_page)], entry_size, lines, false, true);
            }

            listMenu_entry(selected, x, y + ly, choices[selected + (page * per_page)], entry_size, lines, true, false);
            screen_buffer.pushSprite(0, 26);

            break;

        case DOWN:

            old_selected = selected;
            selected++;

            total_items_on_page = std::min(per_page, icount - (page * per_page)); // Update value here

            if (selected >= total_items_on_page) {
                if (page < pages - 1) {

                    page++;
                    selected = 0;
                } else {
                    page     = 0;
                    selected = 0;
                }

                drawImage(0, y + ly, SDImage(BACKGROUND_IMAGE.address + 0x2EE0, 240, 269), true);
                listMenu_header(type, label, page, pages, y);
                int startIndex = page * per_page;
                int endIndex   = std::min(startIndex + per_page, icount);

                for (int i = 0; i < (endIndex - startIndex); i++) {
                    listMenu_entry(i, x, y + ly, choices[startIndex + i], entry_size, lines, false, false);
                }
            } else {

                listMenu_entry(old_selected, x, y + ly, choices[old_selected + (page * per_page)], entry_size, lines, false, true);
            }

            listMenu_entry(selected, x, y + ly, choices[selected + (page * per_page)], entry_size, lines, true, false);

            screen_buffer.pushSprite(0, 26);

            break;
        case RIGHT:
            if (pages > 1) {
                page     = (page + 1) % pages;
                selected = 0;
                drawImage(0, y + 25, SDImage(BACKGROUND_IMAGE.address + 0x2EE0, 240, 269), true);
                listMenu_header(type, label, page, pages, y);

                int startIndex = page * per_page;
                int endIndex   = std::min(startIndex + per_page, icount);

                for (int i = 0; i < (endIndex - startIndex); i++) {
                    listMenu_entry(i, x, y + ly, choices[startIndex + i], entry_size, lines, false, false);
                }
                listMenu_entry(selected, x, y + ly, choices[selected + (page * per_page)], entry_size, lines, true, false);
                screen_buffer.pushSprite(0, 26);
            }
            break;
        case LEFT:
            if (pages > 1) {
                page     = (page - 1 + pages) % pages;
                selected = 0;
                drawImage(0, y + 25, SDImage(BACKGROUND_IMAGE.address + 0x2EE0, 240, 269), true);
                listMenu_header(type, label, page, pages, y);

                int startIndex = page * per_page;
                int endIndex   = std::min(startIndex + per_page, icount);

                for (int i = 0; i < (endIndex - startIndex); i++) {
                    listMenu_entry(i, x, y + ly, choices[startIndex + i], entry_size, lines, false, false);
                }
                listMenu_entry(selected, x, y + ly, choices[selected + (page * per_page)], entry_size, lines, true, false);
                screen_buffer.pushSprite(0, 26);
            }
            break;
        }
    }
    screen_buffer.deleteSprite();
    return -1;
}

// Converter from old type of listMenu to new
// @param choices Array of strings representing the menu options
// @param icount Number of options in the menu
// @param images Boolean indicating if images should be used
// @param type Icon to display in the header 0: `messages` 1: `phone` 2: `settings`
// @param label Title of the menu
// @param forceIcons Boolean indicating if icons should be forced
// @param findex Index of the selected option
int listMenu(const String choices[], int icount, bool images, int type, String label, bool forceIcons, int findex) {
    mOption *optionArr = new mOption[icount];
    for (int i = 0; i < icount; i++) {
        optionArr[i].label = choices[i];
        optionArr[i].icon  = SDImage();
    }
    return listMenu(optionArr, icount, images, type, label, forceIcons, findex);
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
void renderlmng(mOption *choices, int x, int y, int icount, String label, int index, uint16_t color_active, uint16_t color_inactive) {
    uint max_per_page = ((320 - y - tft.fontHeight()) / tft.fontHeight());

    if (index >= max_per_page + lmng_offset) {
        tft.fillRect(x, y, 240 - x, 320 - y, 0);
        tft.fillScreen(0);
    } else if (index < lmng_offset) {
        lmng_offset = index;
        tft.fillRect(x, y, 240 - x, 320 - y, 0);
    }
    tft.setCursor(x, y);
    tft.setTextColor(color_inactive);
    tft.println(label + "  ");

    for (int i = lmng_offset; i < max_per_page + lmng_offset; i++) {
        if (i == index)
            tft.setTextColor(color_active);
        else
            tft.setTextColor(color_inactive);
        if (i < icount)
            tft.println("  " + choices[i].label);
    }
}

// listMenu that doesn't use images
// @param choices Array of mOption objects representing the menu options
// @param icount Number of options in the menu
// @param label Title of the menu
// @param y Y-coordinate for the menu
// @return Index of the selected option
// @note This function is used for non-graphical list menus
int listMenuNonGraphical(mOption *choices, int icount, String label, int y) {
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
            if (index < 0)
                index = icount - 1;
            renderlmng(choices, x, y, icount, label, index, color_active, color_inactive);
            break;
        case DOWN:
            index++;
            if (index >= icount)
                index = 0;
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
int choiceMenu(const String choices[], int count, bool context) {
    int      x              = 30;
    int      y              = 95;
    int      mul            = 20;
    uint16_t color_active   = 0xF9C0;
    uint16_t color_inactive = 0x0000;

    if (context) {
        drawImage(16, 100, CONTEXT_MENU_IMAGE);
        x = 40;
        y = 120;
    } else {
        drawImage(0, 68, WHITE_BOTTOM_IMAGE);
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

                drawImage(16, 100, CONTEXT_MENU_IMAGE);
                for (int i = 0; i < count; i++) {
                    tft.setCursor(x, y + (mul * i));
                    tft.print(choices[i]);
                }
            } else {
                tft.setTextSize(1);
                tft.setTextColor(color_inactive);

                drawImage(0, 68, WHITE_BOTTOM_IMAGE);
                for (int i = 0; i < count; i++) {
                    tft.setCursor(x, y + (mul * i));
                    tft.print(choices[i]);
                }
            }
            if (choice < 1) {
                choice = count - 1;
            } else {
                choice--;
            }
            tft.setTextColor(color_inactive);
            tft.setCursor(x, y + (mul * (choice + 1)));
            if (choice < count - 1)
                tft.print(choices[choice + 1]);
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

                drawImage(16, 100, CONTEXT_MENU_IMAGE);
                for (int i = 0; i < count; i++) {
                    tft.setCursor(x, y + (mul * i));
                    tft.print(choices[i]);
                }
            } else {
                tft.setTextSize(1);
                tft.setTextColor(color_inactive);

                drawImage(0, 68, WHITE_BOTTOM_IMAGE);
                for (int i = 0; i < count; i++) {
                    tft.setCursor(x, y + (mul * i));
                    tft.print(choices[i]);
                }
            }
            if (choice > count - 2) {
                choice = 0;
            } else {
                choice++;
            }

            tft.setTextColor(color_inactive);
            tft.setCursor(x, y + (mul * (choice - 1)));

            if (choice > 0)
                tft.print(choices[choice - 1]);

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
