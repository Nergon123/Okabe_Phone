#include "Messages.h"
#include "../GlobalVariables.h"
#include "../Input/Input.h"

#include "../System/TextManipulation.h"
#include "../UI/ListMenu.h"
#include "../UI/UIElements.h"
#include <algorithm>
// Parse SMS messages from SIM Card
// @param msgs Array of Message objects to store parsed messages
// @param count Number of messages parsed
void parseMessages(Message *&msgs, int &count) {

    NString response     = sendATCommand("AT+CMGL=\"ALL\",1");
    int     lastIndexOf  = 0;
    int     messageCount = 0;

    // Count the number of messages
    while ((lastIndexOf = response.indexOf("+CMGL:", lastIndexOf)) != -1) {
        messageCount++;
        lastIndexOf += 6; // Move past "+CMGL:"
    }

    if (messageCount > 0) {
        msgs        = new Message[messageCount];
        lastIndexOf = 0;

        for (int i = 0; i < messageCount; i++) {
            lastIndexOf = response.indexOf("+CMGL: ", lastIndexOf);
            if (lastIndexOf == -1) { break; }

            int firstComma  = response.indexOf(",", lastIndexOf);
            int secondComma = response.indexOf(",", firstComma + 1);
            // int thirdComma  = response.indexOf(",", secondComma + 1);
            // int fourthComma = response.indexOf(",", thirdComma + 1);

            // Extract index
            msgs[i].index = response.substring(lastIndexOf + 7, firstComma).toInt();

            // Extract status
            NString statusStr = response.substring(response.indexOf("\"", firstComma) + 1,
                                                   response.indexOf("\"", secondComma));
            if (statusStr.indexOf("UNREAD") != -1) { msgs[i].status = status::NEW; }
            else if (statusStr.indexOf("READ") != -1) { msgs[i].status = status::READED; }
            else {
                msgs[i].status = status::NEW; // Default fallback
            }

            // Extract sender number
            NString number = response.substring(
                response.indexOf("\"", secondComma) + 1,
                response.indexOf("\"", response.indexOf("\"", secondComma) + 1));

            // Extract date
            int dateStart = response.indexOf("/", lastIndexOf);
            msgs[i].date  = response.substring(dateStart + 1, response.indexOf(",", dateStart));

            NString checknumber;
            if (number.indexOf('+') != -1) {
                checknumber = number.substring(number.indexOf('+') + 3);
            }
            else { checknumber = number; }
            Contact curContact;
            curContact.phone = number;

            // For some reason when some number from some newsletter or service like carrier.
            // Will be recieved as number with letter p in it. (something like 543p2341)
            // it cannot be replied back to that number so it will be "SERVICE NUMBER".
            if (number.indexOf('p') != -1) { curContact.name = "SERVICE NUMBER"; }
            else { curContact.name = number; }
            curContact.index = -1;
            for (size_t u = 0; u < contacts.size(); u++) {
                if (contacts[u].phone.indexOf(checknumber) != -1) {
                    curContact = contacts[u];
                    break;
                }
            }
            msgs[i].contact = curContact;
            ESP_LOGD("MSG", "INDEX:   %d", msgs[i].index);
            ESP_LOGD("MSG", "STATUS:  %d", (int)msgs[i].status); // Cast enum to int
            ESP_LOGD("MSG", "DATE:    %s", msgs[i].date.c_str());
            ESP_LOGD("MSG", "NUMBER:  %s", number.c_str());
            ESP_LOGD("MSG", "CHKNMB:  %s", checknumber.c_str());
            ESP_LOGD("MSG", "CONTACT: %s", msgs[i].contact.name.c_str());
            lastIndexOf += 7; // Move past "+CMGL:"
        }
    }
    count = messageCount;
}

// messages menu
void messages() {

    res.DrawImage(R_MENU_BACKGROUND);
    res.DrawImage(R_MAIL_MENU_L_HEADER);
    res.DrawImage(R_MENU_L_HEADER);
    NString entries[] = {"Inbox", "Outbox"};
    int     ch        = choiceMenu(entries, ArraySize(entries), false);
    inbox(ch);
    // Maybe in future there will be more entries...
    // switch (ch) {
    // case 0: inbox(false); break;
    // case 1: inbox(true); break;
    // default: break;
    //}

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
    const int TLVP = 238;

    size_t limit = 0;
    if (sms) { limit = 160; }
    else { limit = 400; }
    int     curx     = 0;
    int     cury     = 0;
    int     text_pos = 0;
    int     position = 0;
    drawStatusBar();
    res.DrawImage(R_LIST_HEADER_BACKGROUND);
    res.DrawImage(R_LIST_HEADER_ICONS, 0);
    // jump in pixels per one button press
    int y_jump = 22;
    // offset of screen in height
    int y_scr  = 0;
    int y_text = 18;
    int min_y  = y_scr;
    tft.setCursor(30, 45);
    tft.setTextSize(1);
    changeFont(1);
    changeFont(1);
    tft.setTextColor(0xffff);

    tft.print("Outgoing Mail");
    tft.setTextColor(0);

    // tft.setViewport(0, 51, 240, 269, true);
    bool exit = false;
    while (!exit) {
        position = 0;
        res.DrawImage(R_LIST_MENU_BACKGROUND, 0, {0, 0}, {0, 0}, {0, 0}, RES_MAIN);
        // drawImage(0, 0, BACKGROUND_IMAGE_CUTTED, true);
        // tft.fillScreen(0xFFFF);
        // position += 24;
        res.DrawImage(R_IN_MSG_MAIL_ICONS, 2, {OP_UNDEF, position + y_scr}, {0, 0}, {0, 0},
                      RES_MAIN);
        // drawImage(0, position + y_scr, in_mail[2], true);
        tft.setCursor(24, position + y_text + y_scr);
        tft.println(!contact.name.isEmpty()    ? contact.name
                    : !contact.phone.isEmpty() ? contact.phone
                    : !contact.email.isEmpty() ? contact.email
                                               : "UNKNOWN");
        content = SplitString(content);
        if (!subject.isEmpty() || !sms) {
            position += 24;
            res.DrawImage(R_IN_MSG_MAIL_ICONS, 3, {OP_UNDEF, position + y_scr}, {0, 0}, {0, 0},
                          RES_MAIN);
            tft.setCursor(24, position + y_text + y_scr);
            tft.println(subject);
        }
        position += 24;
        tft.drawLine(0, position + y_scr, 240, position + y_scr, 0);
        tft.print(content);
        int     input = -1;
        NString a[]   = {"Return", "Send Message", "Delete", "Save To Drafts"};
        int     u;
        tft.drawLine(curx + 1, 2 + position + y_scr + cury, 1 + curx, y_scr + cury + position + 20,
                     TFT_BLACK);
        // Serial.println("CURX:" + NString(curx) + " CURY:" + NString(cury));
        /////////tft.pushSprite(0, 51);
        while (input == -1) {

            if (y_scr < min_y) { min_y = y_scr; }
            input = buttonsHelding();
            switch (input) {
            case DOWN:
                // if (y_scr > -height)
                //   y_scr -= y_jump;
                text_pos = findCharPosX(content, text_pos, DOWN);
                findSplitPosition(content, text_pos, curx, cury);
                break;
            case UP:
                // if (y_scr < 0)
                //   y_scr += y_jump;
                text_pos = findCharPosX(content, text_pos, UP);
                findSplitPosition(content, text_pos, curx, cury);
                break;
            case RIGHT:
                if (text_pos >= 0) { findSplitPosition(content, ++text_pos, curx, cury); }

                break;
            case LEFT:
                if (text_pos > 0) { findSplitPosition(content, --text_pos, curx, cury); }

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
                    /////////tft.deleteSprite()();
                    return;
                    break;
                case 2: ESP_LOGI("INFO", "DELETE"); break;
                case 3: ESP_LOGI("INFO", "Save to Drafts"); break;

                default: break;
                }
                break;
            default:
                if (input >= '0' && input <= '9') {

                    char l = textInput(input, 0, 0, 1);

                    input = buttonsHelding();
                    if (l != 0) {

                        // ESP_LOGI("INFO","Y:" + NString(tft.getCursorY()));
                        //  drawCutoutFromSd(SDImage(0x639365, 240, 269, 0, false), 0, 260, 120,
                        //  20, 0, 240);

                        if (content.length() < limit) {
                            if (l != '\r') {
                                if (l != '\b') {

                                    content =
                                        content.substring(0, text_pos) + l +
                                        content.substring(text_pos, content.length());
                                    text_pos++;
                                }
                                else {
                                    content =
                                        content.substring(0, text_pos - 1) +
                                        content.substring(text_pos, content.length());
                                    input = BACK;
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
                        // ESP_LOGI("INFO","MINIMUM:" + NString(y_scr));
                        findSplitPosition(content, text_pos, curx, cury);
                        input = BACK;
                    }
                }
                break;
            }
        }
        // tft.resetViewport();

        // tft.setViewport(0, 51, 240, 269, true);
    }
    /////////tft.deleteSprite()();
    // tft.resetViewport();
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
    // returns if deleted
    if (sms) {
        NString response = sendATCommand("AT+CMGR=" + NString(index));
        if (response.indexOf("OK") == -1) {
            content = "======\nERROR LOADING FOLLOWING MESSAGE\n======";
        }
        int resIndex = 0;
        for (int i = 0; i < 7; i++) { resIndex = response.indexOf('\"', ++resIndex); }
        int endIndex = response.indexOf('\"', ++resIndex);
        ESP_LOGD("SMS", "resIndex:%d\nendIndex:%d", resIndex, endIndex);
        date = response.substring(resIndex, endIndex);
        date.replace(',', ' ');
        date = date.substring(0, date.lastIndexOf(':'));
        ESP_LOGD("SMS", "%s", date.c_str());
        content = response.substring(endIndex + 3, response.lastIndexOf("OK") - 2);
    }
    content.trim();
    const NString choices[4] = {"Reply", "Return", "Delete", "HEX to ASCII"};
    drawStatusBar();
    res.DrawImage(R_LIST_HEADER_BACKGROUND);
    res.DrawImage(R_LIST_HEADER_ICONS, 0);
    int y_jump = 22;
    int y_scr  = 0;
    changeFont(1);
    int y_text = 18;

    tft.setCursor(30, 45);
    tft.setTextSize(1);
    tft.setTextColor(0xffff);
    bool deleted = false;
    tft.print("Recieve Mail");
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
            case BACK:
                ch = choiceMenu(choices, 4, true);
                switch (ch) {
                case -1: exit = true; break;
                case 0: messageActivityOut(contact, "", "", true); break;

                case 2:
                    tft.resetViewport();
                    if (confirmation("ARE YOU SURE YOU WANT TO DELETE MESSAGE?")) {
                        sendATCommand("AT+CMGD=" + NString(index), 5000);
                        return true;
                    }
                    tft.setViewport(0, 51, 240, 269);
                    break;
                case 3: content = HEXTOASCII(content); break;
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
    return messageActivity(message.contact, message.date, message.subject, message.content,
                           message.index, false, true);
}

/*
 * Inbox and Outbox menu
 *
 * In this function, the user can view and select messages from the inbox or outbox.
 * @param outbox Boolean indicating if the menu is for the outbox
 */
void inbox(bool outbox) {
    const char *title;
    if (outbox) { title = "Outbox"; }
    else { title = "Inbox"; }

    int      count    = 0;
    bool     exit     = false;
    Message *messages = nullptr;
    while (!exit) {
        exit = true;

        parseMessages(messages, count);
        // Potential memory leak???
        std::vector<mOption> messList;
        for (int i = 0; i < count; i++) { messList.push_back(messages[i]); }

        std::reverse(messList.begin(), messList.end());
        int choice = -2;
        while (choice != -1) {
            choice = listMenu(messList, messList.size(), false, 0, title);
            if (choice >= 0) {
                exit = !messageActivity(messages[count - choice - 1]);
                if (!exit) { choice = -1; }
            }
        }
    }
}
