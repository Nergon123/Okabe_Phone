#include "Phone.h"

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
    drawImage(0, 90, FULL_SCREEN_NOTIFICATION_IMAGE);
    changeFont(1);
    tft.setTextColor(0);
    tft.setCursor(15, 170);
    tft.print(contact.name);
    tft.setTextColor(0xf800);
    tft.setCursor(90, 140);
    changeFont(4);
    tft.setTextSize(1);
    tft.print("Ca l l ing");
    drawImage(45, 105, PHONE_ICON);

    writeCustomFont(55, 185, contact.phone, 1);
    drawImage(73, 90, LIGHTNING_ANIMATION[0]);
    int button = buttonsHelding();
    while (isCalling) {
        button = buttonsHelding();

        switch (button) {
        case ANSWER:
            if (sendATCommand("ATA").indexOf("NO CARRIER") == -1) {
                callActivity(contact);
            } else
                currentScreen = SCREENS::MAINSCREEN;
            break;
        case DECLINE:
            currentScreen = SCREENS::MAINSCREEN;
            return;
            break;

        default:
            break;
        }
    }
}

/*
 * Function to make a call
 * @param contact Contact object containing contact information
 */
void makeCall(Contact contact) {
    if (!checkSim()) {
        return;
    }
    isAnswered = true;
    sendATCommand("ATD" + contact.phone + ";");
    callActivity(contact);
}

/*
 * call activity screen
 * @param contact Contact object containing contact information
 */
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
    // tft.print(contact.number);
    writeCustomFont(5, 240, contact.phone);

    tft.setTextSize(1);
    tft.setCursor(85, 95);
    tft.print("Calling...");
    stateCall = GetState();
    delay(50);
    bool hang = false;
    while (stateCall == DIALING) {
        for (int i = 7; i >= 0; i--) {
            spinAnim(55, 60, 12, 6, i);
            delay(40);
            if (buttonsHelding() == DECLINE) {
                hang = true;
                // calling = false;
                break;
            } else if (buttonsHelding() == UP) {
                // calling = false;
                break;
            }
        }
    }
    if (hang) {
        // if hang up
        sendATCommand("ATH");
        ongoingCall = false;

        return;
    }

    tft.fillScreen(0);
    sBarChanged = true;
    drawStatusBar();
    drawImage(40, 143, VOICE_ONLY_LABEL);
    while (stateCall != DISCONNECT) {
        if (buttonsHelding() == DECLINE) {
            sendATCommand("ATH");
            ongoingCall = false;
        }
        stateCall = GetState();
    }
    tft.fillScreen(0);
    drawStatusBar();
    ongoingCall = false;
    isAnswered  = false;
}


/*
* Contact list screen
*/
void contactss() {
    // if (!checkSim()) {
    //     currentScreen = SCREENS::MAINMENU;
    //     return;
    // }
    const String contactMenuItems[] = {
        "Call",
        "Outgoing",
        "Edit",
        "Create",
        "Delete"};

    String contactNames[contactCount];
    for (int i = 0; i < contactCount; ++i) {
        contactNames[i] = contacts[i].name;
    }

    bool exit = false;
    while (!exit) {

        String contactNames[contactCount];
        for (int i = 0; i < contactCount; ++i) {
            contactNames[i] = contacts[i].name;
        }
        int selectedContactIndex = listMenu(contactNames, contactCount, false, 1, "Address Book");

        if (selectedContactIndex != -1) {
            int contextMenuSelection = -1;

            contextMenuSelection = choiceMenu(contactMenuItems, 5, true);

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

                editContact(Contact("", "", "", lastContactIndex + 1));
                break;
            case 4:
                // DELETE
                sendATCommand("AT+CPBW=" + String(contacts[selectedContactIndex].index));
                populateContacts();
                break;
            }

        } else {

            const String choice[1] = {"Create"};
            int          CMS       = choiceMenu({choice}, 1, true);
            if (!CMS) {
                editContact(Contact("", "", "", lastContactIndex + 1));
            } else
                exit = true;
        }
    }
    currentScreen = SCREENS::MAINMENU;
}

/*
 * Function to edit a contact
 * @param contact Contact object containing contact information
 */
void editContact(Contact contact) {
    int pos       = 0;
    int textboxes = 2;
    int buttons   = 2;
    int direction;
    drawImage(0, 26, BLUEBAR_IMAGE);
    drawImage(0, 26, BLUEBAR_ICONS[1]);
    drawStatusBar();
    tft.setCursor(30, 45);
    tft.setTextSize(1);
    changeFont(1);
    tft.setTextColor(0xffff);
    String boxString[textboxes] = {contact.name, contact.phone};
    tft.print("Edit Contact");
    drawImage(0, 51, BACKGROUND_IMAGE_CUTTED);
    InputField("Name", contact.name, 70, true, false, false);
    InputField("Phone Number", contact.phone, 120, true, false, false);

    button("SAVE", 10, 285, 100, 28);
    button("CANCEL", 120, 285, 100, 28);
    bool save   = false;
    bool cancel = false;
    bool exit   = false;
    while (!exit) {
        switch (pos) {
        case 0:
            boxString[pos] = InputField("Name", boxString[pos], 70, false, false, true, &direction);
            break;
        case 1:
            boxString[pos] = InputField("Phone Number", boxString[pos], 120, false, false, true, &direction, true);
            break;
        case 2:
            save = button("SAVE", 10, 285, 100, 28, true, &direction);
            if (save) {
                if (boxString[1].isEmpty())
                    break;
                if (boxString[0].isEmpty()) {
                    boxString[0] = boxString[1];
                }

                sendATCommand("AT+CPBS=\"SM\"");
                String request = "AT+CPBW=" +
                                 String(contact.index) + ",\"" +
                                 boxString[1] + "\"," +
                                 String(boxString[1].indexOf("+") == 0 ? 145 : 129) +
                                 ",\"" + boxString[0] + "\"";
                String result = sendATCommand(request);
                populateContacts();
                Serial.println(result);
                return;
            }

            // boxString[pos] = textbox("E-mail", boxString[pos], 170, false, false, true, &direction);
            break;
        case 3:
            cancel = button("CANCEL", 120, 285, 100, 28, true, &direction);
            if (cancel)
                return;
            break;
        default:
            pos = 0;
            break;
        }
        Serial.println("DIRECTION:" + String(direction));
        Serial.println("POS:" + String(pos));
        switch (direction) {
        case DOWN:
            if (pos < textboxes)
                pos++;
            else if (pos < textboxes + buttons)
                ;
            else
                pos = 0;
            break;
        case UP:
            if (pos > textboxes)
                pos = textboxes - 1;

            else if (pos > 0)
                pos--;

            break;
        default:
            break;
        case RIGHT:
            if (pos < textboxes + buttons - 1)
                pos++;
            break;
        case LEFT:
            if (pos > textboxes)
                pos--;
            break;
        }
    }
}