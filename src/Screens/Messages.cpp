#include "Messages.h"
#include "GlobalVariables.h"
#include "Input/Input.h"
#include "System/TextManipulation.h"
#include "UI/ListMenu.h"
#include "UI/UIElements.h"
#include <System/LanguageSystem.h>
#include <algorithm>
// messages menu
void messages() {
    res.DrawImage(R_MENU_BACKGROUND);
    res.DrawImage(R_MAIL_MENU_L_HEADER);
    res.DrawImage(R_MENU_L_HEADER);
    NString entries[] = {getTranslation(TextKey::MSGS_INBOX),
                         getTranslation(TextKey::MSGS_OUTBOX)};
    int     ch        = choiceMenu(entries, ArraySize(entries), false);
    if (ch < 2 && ch >= 0) { inbox(ch); }
    currentScreen = SCREENS::MAINMENU;
}

/*
 * Outgoing message activity
 *
 * In this function, the user can write and send a message to a contact.
 *
 * @param contact Contact object containing contact information
 * @param subject Subject of the message
 * @param content Content of the message
 * @param sms Boolean indicating if the message is an SMS
 */
void messageActivityOut(Contact contact, NString subject, NString content, bool sms) {
    // size of accessible height of current viewport
    const int TLVP = 269;
    tft.resetViewport();
    size_t limit = 0;
    if (sms) { limit = 160; }
    else { limit = 400; }
    int curx     = 0;
    int cury     = 0;
    int text_pos = 0;
    int position = 0;
    drawStatusBar();
    drawHeader(getTranslation(TextKey::MSGS_OUTGOING), LM_MESSAGES);
    // jump in pixels per one button press
    int y_jump = 22;
    // offset of screen in height
    int y_scr  = 0;
    int y_text = 18;
    int min_y  = y_scr;
    tft.setCursor(30, 45);
    tft.setTextSize(1);
    changeFont(1);
    tft.setTextColor(0);
    currentRenderTarget->present();
    bool exit = false;
    while (!exit) {
        position = 0;
        res.DrawImage(R_LIST_MENU_BACKGROUND, 0, {0, 0}, {0, 0}, {0, 0});
        // drawImage(0, 0, BACKGROUND_IMAGE_CUTTED, true);
        // tft.fillScreen(0xFFFF);
        // position += 24;
        res.DrawImage(R_IN_MSG_MAIL_ICONS, 2, {OP_UNDEF, position + y_scr});
        // drawImage(0, position + y_scr, in_mail[2], true);
        tft.setCursor(24, position + y_text + y_scr);
        changeFont(1);

        tft.println(!contact.name.isEmpty()    ? contact.name
                    : !contact.phone.isEmpty() ? contact.phone
                    : !contact.email.isEmpty() ? contact.email
                                               : "UNKNOWN");
        content = SplitString(content);
        if (!subject.isEmpty() || !sms) {
            position += 24;
            res.DrawImage(R_IN_MSG_MAIL_ICONS, 3, {OP_UNDEF, position + y_scr}, {0, 0}, {0, 0});
            tft.setCursor(24, position + y_text + y_scr);
            changeFont(1);
            tft.println(subject);
        }
        position += 24;
        tft.drawLine(0, position + y_scr, 240, position + y_scr, 0);
        changeFont(1);
        tft.print(content);
        int     input = -1;
        NString a[]   = {getTranslation(TextKey::MSGS_OUT_MENU_CONTINUE),
                         getTranslation(TextKey::MSGS_OUT_MENU_SEND),
                         getTranslation(TextKey::MSGS_OUT_MENU_DELETE),
                         getTranslation(TextKey::MSGS_OUT_MENU_SAVE)};
        int     u;
        tft.drawLine(curx + 1, 2 + position + y_scr + cury, 1 + curx, y_scr + cury + position + 20,
                     TFT_BLACK);
        currentRenderTarget->present();
        while (input == -1) {

            if (y_scr < min_y) { min_y = y_scr; }
            input = buttonsHelding();
            switch (input) {
            case DOWN:
                // if (y_scr > -height)
                //   y_scr -= y_jump;
                changeFont(1);
                text_pos = findCharPosX(content, text_pos, DOWN);
                findSplitPosition(content, text_pos, curx, cury);
                break;
            case UP:
                // if (y_scr < 0)
                //   y_scr += y_jump;
                changeFont(1);
                text_pos = findCharPosX(content, text_pos, UP);
                findSplitPosition(content, text_pos, curx, cury);
                break;
            case RIGHT:

                if (text_pos >= 0) {
                    changeFont(1);
                    findSplitPosition(content, ++text_pos, curx, cury);
                }

                break;
            case LEFT:
                if (text_pos > 0) {
                    changeFont(1);
                    findSplitPosition(content, --text_pos, curx, cury);
                }

                break;
            case BACK:

                u = choiceMenu(a, 4, true);
                tft.setTextColor(0);
                switch (u) {
                case -1:
                    input = -2;
                    exit  = true;
                    break;
                case 1:
                    ESP_LOGI("INFO", "SEND MESSAGE");
                    if (sendATCommand("AT+CMGF=1").indexOf("OK") != -1) {
                        sendATCommand("AT+CMGS=\"" + contact.phone + "\"");
                        sendATCommand(content + char(26));
                    }
                    return;
                    break;
                case 2: ESP_LOGI("INFO", "DELETE");
#warning Delete Message not implemented
                    break;
                case 3:
                    ESP_LOGI("INFO", "Save to Drafts")
#warning Save to Drafts not implemented
                        ;
                    break;

                default: break;
                }
                break;
            default:
                if (input >= '0' && input <= '9') {
                    NString l = textInput(input, 0, 0, 1);
                    input     = buttonsHelding();
                    if (l != 0) {

                        if (content.length() < limit) {
                            if (l != '\r') {
                                if (l != '\b') {
                                    content = content.substring(0, text_pos) + l +
                                              content.substring(text_pos, content.length());
                                    text_pos += l[0] > 0x7F ? 2 : 1;
                                }
                                else {
                                    uint8_t deletedCharLength =
                                        content[text_pos - 1] >= 0x7F ? 2 : 1;
                                    content = content.substring(0, text_pos - deletedCharLength) +
                                              content.substring(text_pos, content.length());
                                    input   = BACK;
                                }
                            }
                        }
                        if (tft.getCursorY() > TLVP) {
                            y_scr -= y_jump;
                            if (y_scr < min_y) { min_y = y_scr; }
                            input = BACK;
                        }
                        if (y_scr != min_y) {
                            y_scr = min_y;
                            input = BACK;
                        }
                        changeFont(1);
                        findSplitPosition(content, text_pos, curx, cury);
                        input = BACK;
                    }
                }
                break;
            }
        }
        // tft.resetViewport();

        // tft.setViewport(0, 51, 240, 269);
    }
    /////////tft.deleteSprite()();
    tft.resetViewport();
}

/*
 * Incoming message activity
 *
 * In this function, the user can read and reply to a message.
 * @param contact Contact object containing contact information
 * @param date Date of the message
 * @param subject Subject of the message
 * @param content Content of the message
 * @param index Index of the message
 * @param outcoming Boolean indicating if the message is outgoing
 * @param sms Boolean indicating if the message is an SMS
 * @return Boolean indicating if the message was deleted
 */
bool messageActivity(Contact contact, NString date, NString subject, NString content, int index,
                     bool outcoming, bool sms) {
    tft.setTextWrap(true);
    content.trim();
    const NString choices[] = {getTranslation(TextKey::MSGS_INC_MENU_REPLY), getTranslation(TextKey::MSGS_INC_MENU_DELETE)};
    drawStatusBar();
    drawHeader(getTranslation(TextKey::MSGS_INCOMING) ,LM_MESSAGES);
    int y_jump = 22;
    int y_scr  = 0;
    changeFont(1);
    int  y_text  = 18;
    bool deleted = false;
    tft.setTextSize(1);
    tft.setTextColor(0);
    tft.setViewport(0, 51, 240, 269);
    bool exit = false;
    while (!exit) {
        deleted = false;
        tft.setTextColor(0);
        res.DrawImage(R_LIST_MENU_BACKGROUND, 0, {0, 0});
        // tft.fillScreen(0xFFFF);
        res.DrawImage(R_IN_MSG_MAIL_ICONS, 0, {OP_UNDEF, y_scr});
        tft.setCursor(24, 0 + y_text + y_scr);
        tft.println(date);

        res.DrawImage(R_IN_MSG_MAIL_ICONS, 1, {OP_UNDEF, y_scr + 24});
        tft.setCursor(24, 24 + y_text + y_scr);
        tft.println(!contact.name.isEmpty()    ? contact.name
                    : !contact.phone.isEmpty() ? contact.phone
                    : !contact.email.isEmpty() ? contact.email
                                               : "UNKNOWN");
        content = SplitString(content);
        tft.drawLine(0, 48 + y_scr, 240, 48 + y_scr, 0);
        int height = measureStringHeight(content) + 50;
        tft.print(content);
        currentRenderTarget->present();
        int ch = -2;
        int r  = -1;
        while (r == -1) {
            r = buttonsHelding();
            switch (r) {
            case DOWN:
                if (y_scr > -height) { y_scr -= y_jump; }
                else { r = -1; }
                break;
            case UP:
                if (y_scr < 0) { y_scr += y_jump; }
                else { r = -1; }
                break;
            case BACK: exit = true; break;
            case SELECT:
                ch = choiceMenu(choices, ArraySize(choices), true);
                switch (ch) {
                case 0: messageActivityOut(contact, "", "", true); break;
                case 1:
                    tft.resetViewport();
                    if (confirmation(getTranslation(TextKey::MSGS_INC_MENU_DELETE_CONFIRM))) {
                        sendATCommand("AT+CMGD=" + NString(index), 5000);
                        return true;
                    }
                    tft.setViewport(0, 51, 240, 269);
                    break;
                }
                break;
            default: r = -1; break;
            }
        }
        tft.resetViewport();

        tft.setViewport(0, 51, 240, 269);
    }
    tft.resetViewport();
    return deleted;
}

/*
 * Incoming message activity
 *
 * In this function, the user can read and reply to a message.
 * @param message Message object containing message information
 * @return Boolean indicating if the message was deleted
 */
bool messageActivity(Message message) {
    return messageActivity(message.contact, message.longdate, message.subject, message.content,
                           message.index, false, true);
}

/*
 * Inbox and Outbox menu
 *
 * In this function, the user can view and select messages from the inbox or outbox.
 * @param outbox Boolean indicating if the menu is for the outbox
 */
void inbox(bool outbox) {
    bool exit = false;
    while (!exit) {
        exit = true;
        InfoWindow(getTranslation(TextKey::IW_INBOX_LOAD_MSGS),
                   getTranslation(TextKey::IW_TITLE_INFO), false, TFT_BLUE);
        std::vector<Message> msgs = parseMessages();
        std::reverse(msgs.begin(), msgs.end());
        std::vector<mOption> messList;
        for (int i = 0; i < msgs.size(); i++) { messList.push_back(msgs[i]); }
        int choice = LISTMENU_NULL;
        while (choice != LISTMENU_EXIT) {
            choice = listMenu(messList, messList.size(), false, LM_MESSAGES,
                              outbox ? getTranslation(TextKey::MSGS_OUTBOX)
                                     : getTranslation(TextKey::MSGS_INBOX),
                              false, choice)
                         .index;
            if (choice >= 0) {
                exit = !messageActivity(msgs[choice]);
                if (!exit) { choice = -1; }
            }
        }
    }
}
