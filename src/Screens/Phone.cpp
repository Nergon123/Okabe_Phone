#include "Phone.h"
#include <System/LanguageSystem.h>
// Check if SIM card can make calls
bool isAbleToCall = false;
// Check if someone answered our call
bool isAnswered = false;
// Check if SIM Serial in use
volatile bool simIsBusy = false;
// check if there call in progress
volatile bool ongoingCall = false;
// check if there SIM card available
volatile bool simIsUsable = false;
// Variable that indicate if there incoming call
bool isCalling = false;

/*
 * incoming call screen
 *
 * In this function, the user can answer or decline an incoming call.
 * @param contact Contact object containing contact information
 */
void incomingCall(Contact contact) {
    drawWallpaper();
    res.DrawImage(R_FULL_NOTIFICATION);
    changeFont(1);
    tft.setTextColor(0);
    tft.setCursor(15, 170);
    tft.print(contact.name);
    tft.setTextColor(0xf800);
    tft.setCursor(90, 140);
    changeFont(4);
    tft.setTextSize(1);
    tft.print(getTranslation(TextKey::TEL_RECIEVING_CALL));
    res.DrawImage(R_PHONE_ICON);
    writeCustomFont(55, 185, contact.phone, 1);
    res.DrawImage(R_PHONE_ICON_LIGHTNING, 0);
    int button = buttonsHelding();
    while (isCalling) {
        button = buttonsHelding();
        switch (button) {
        case ANSWER:
            if (sendATCommand("ATA").indexOf("NO CARRIER") == -1) { callActivity(contact); }
            else {
                return;
                currentScreen = SCREENS::MAINSCREEN;
            }
            break;
        case DECLINE:
            sendATCommand("ATH");
            isCalling     = false;
            currentScreen = SCREENS::MAINSCREEN;
            return;

        default: break;
        }
    }
}

void makeCall(Contact contact) {
    if (!checkSim()) { return; }
    isAnswered = false;
    sendATCommand("ATD" + contact.phone + ";");
    callActivity(contact);
}
void GetStateTask(void*) {
    while (true) {
        hw->delay(1000);
        stateCall = GetState();
    }
}
void callActivity(Contact contact) {
    ongoingCall = true;
    // bool calling = true;
    tft.fillScreen(0);
    sBarChanged = true;
    drawStatusBar();

    changeFont(1);
    tft.setTextColor(0xffff);

    tft.setTextSize(1);
    tft.setCursor(20, 140 + 60);
    tft.print(contact.name);

    tft.setTextSize(2);
    tft.setCursor(0, 180 + 60);
    writeCustomFont(5, 240, contact.phone);

    tft.setTextSize(1);
    tft.setCursor(85, 95);
    tft.print(getTranslation(TextKey::TEL_CALLING));
    stateCall = DIALING;
    TASK task = LaunchTask(GetStateTask, "GetStateTask", nullptr, 4096);
    hw->delay(50);
    currentRenderTarget->present();
    while (stateCall == DIALING) {
        for (int i = 7; i >= 0; i--) {
            spinAnim(55, 60, 12, 6, i);
            hw->delay(40);
            int buttonsH = buttonsHelding();
            if ((buttonsH >= '0' && buttonsH <= '9') || buttonsH == '*' || buttonsH == '#') {
                // TODO show dtmf numbers
                sendATCommand("AT+VTS=\"" + NString(buttonsH) + "\"");
            }
            if (buttonsH == DECLINE) {
                ongoingCall = false;
                sendATCommand("ATH");
                DeleteTask(task);
                return;
            }
        }
    }

    tft.fillScreen(0);
    sBarChanged = true;
    drawStatusBar(true);
    res.DrawImage(R_VOICE_ONLY_LABEL);
    currentRenderTarget->present();
    while (stateCall != DISCONNECT) {
        int buttonsH = buttonsHelding();
        if ((buttonsH >= '0' && buttonsH <= '9') || buttonsH == '*' || buttonsH == '#') {
            // TODO show dtmf numbers
            sendATCommand("AT+VTS=\"" + NString(buttonsH) + "\"");
        }
        if (buttonsH == DECLINE) {
            sendATCommand("ATH");
            ongoingCall = false;
        }
    }
    tft.fillRect(0, 26, 240, 294, TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    changeFont(2);
    NString callEnded = getTranslation(TextKey::TEL_END_OF_CALL);
    tft.setCursor(120 - tft.textWidth(callEnded) / 2, 150);
    tft.print(callEnded);
    currentRenderTarget->present();
    hw->delay(1000);
    drawStatusBar(true);
    ongoingCall   = false;
    isAnswered    = false;
    millSleep     = hw->millis();
    currentScreen = MAINSCREEN;
    DeleteTask(task);
}

/*
 * Contact list screen
 */
void contactss() {
    // if (!checkSim()) {
    //     currentScreen = SCREENS::MAINMENU;
    //     return;
    // }
    const NString contactMenuItems[] = {getTranslation(TextKey::TEL_CONTACTS_OPT_CALL),
                                        getTranslation(TextKey::TEL_CONTACTS_OPT_OUTGOING),
                                        getTranslation(TextKey::TEL_CONTACTS_OPT_EDIT),
                                        getTranslation(TextKey::TEL_CONTACTS_OPT_CREATE),
                                        getTranslation(TextKey::TEL_CONTACTS_OPT_DELETE)};

    bool exit = false;
    while (!exit) {

        NString contactNames[contacts.size()];
        for (size_t i = 0; i < contacts.size(); ++i) { contactNames[i] = contacts[i].name; }
        LM_RET_VALUE choice = listMenu(contactNames, contacts.size(), false, LM_CONTACTS,
                                       getTranslation(TextKey::LM_ADDRESS_BOOK));
        int          selectedContactIndex = choice.index;

        if (selectedContactIndex != LISTMENU_EXIT && contacts.size() > 0) {
            int contextMenuSelection = -1;
            if (choice.button != ANSWER) {
                contextMenuSelection = choiceMenu(contactMenuItems, 5, true);
            }
            else { contextMenuSelection = 0; }
            switch (contextMenuSelection) {

            case 0:
                // CALL
                makeCall(contacts[selectedContactIndex]);
                exit = true;
                break;
            case 1:
                // OUTGOING
                messageActivityOut(contacts[selectedContactIndex], "", "", true);
                break;
            case 2:
                // EDIT
                editContact(contacts[selectedContactIndex]);
                break;
            case 3:
                // CREATE
                editContact(Contact("", "", "", contacts.size()));
                break;
            case 4:
                // DELETE
                sendATCommand("AT+CPBW=" + NString(contacts[selectedContactIndex].index));
                populateContacts();
                break;
            }
        }
        else if (selectedContactIndex == LISTMENU_OPTIONS) {
            const NString choice[1] = {getTranslation(TextKey::TEL_CONTACTS_OPT_CREATE)};
            int           CMS       = choiceMenu({choice}, 1, true);
            if (!CMS) { editContact(Contact("", "", "", contacts.size())); }
            else { exit = true; }
        }
        else { exit = true; }
    }
    currentScreen = SCREENS::MAINMENU;
}

/*
 * Function to edit a contact
 * @param contact Contact object containing contact information
 */
void editContact(Contact contact) {
    std::vector<FIELD> fields = {FIELD(getTranslation(TextKey::TEL_CONTACTS_NAME), contact.name, false),
                                 FIELD(getTranslation(TextKey::TEL_CONTACTS_NUMBER), contact.phone, true)};
    if (!InputFieldS(getTranslation(TextKey::TEL_CONTACTS_EDIT), fields, LM_CONTACTS, 0, getTranslation(TextKey::SAVE_BUTTON), getTranslation(TextKey::CANCEL_BUTTON))) { return; }
    if (contact.phone.isEmpty()) { return; }
    if (contact.name.isEmpty()) { contact.name = contact.phone; }
    sendATCommand("AT+CPBS=\"SM\"");
    NString request = "AT+CPBW=" + NString(contact.index) + ",\"" + contact.phone + "\"," +
                      NString(contact.phone.indexOf("+") == 0 ? 145 : 129) + ",\"" + contact.name +
                      "\"";
    NString result  = sendATCommand(request);
    populateContacts();
}