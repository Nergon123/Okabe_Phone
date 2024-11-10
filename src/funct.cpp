#include "funct.h"
String sendATCommand(String command)
{
  Serial1.println(command);
  delay(100); // Wait for response

  String response = "";
  while (Serial1.available())
  {
    response += Serial1.readString();
  }
  return response;
}

int getChargeLevel()
{
  int charge = chrg.getBatteryLevel() / 25;

  int toIcon = (charge / 25) - 1;
#ifdef DEVMODE
  toIcon = charge_d;
#endif
  if (toIcon > 3)
    toIcon = 3;
  if (toIcon < 0)
    toIcon = 0;
  return toIcon;
}
int getSignalLevel()
{
  int signal = 1; // temp placeholder to actual getSignalLevel
#ifdef DEVMODE
  signal = signallevel_d;
#endif
  return signal;
}

void populateContacts()
{
  String response = sendATCommand("AT+CPBR=1,100"); // Query contacts from index 1 to 100

  // Process the response
  int startIndex = 0;
  int endIndex = 0;
  contactCount = 0;

  while ((startIndex = response.indexOf("+CPBR: ", endIndex)) != -1)
  {
    startIndex += 7; // Skip "+CPBR: "
    endIndex = response.indexOf('\n', startIndex);
    String entry = response.substring(startIndex, endIndex);

    // Split the entry into components
    int commaIndex = entry.indexOf(',');
    if (commaIndex == -1)
      break;

    String indexStr = entry.substring(0, commaIndex);
    entry = entry.substring(commaIndex + 1);

    commaIndex = entry.indexOf(',');
    if (commaIndex == -1)
      break;

    String number = entry.substring(0, commaIndex);
    number.replace("\"", "");
    entry = entry.substring(commaIndex + 1);

    String name = entry;
    name.replace("\"", "");
    name.replace("145,", "");

    // Populate the contact structure
    contacts[contactCount].index = contactCount; // Use the count as index
    contacts[contactCount].number = number;
    contacts[contactCount].name = name;
    contactCount++;

    if (contactCount >= MAX_CONTACTS)
      break; // Prevent overflow
  }
}


bool checkButton(int pin) {
  if (digitalRead(pin) == LOW) {
    delay(50); 
    while (digitalRead(pin) == LOW);  
    return true;
  }
  return false;
}

int buttonsHelding() {
    /*
   *
   * |M|I|B|C|
   *     ^=2
   * <   0=1 >
   * A   V=3 D=0
   * 1   2   3
   * 4   5   6
   * 7   8   9
   * *   0   #
   *
   * M-MESSAGES
   * I-INFORMATION(????)
   * B-CONTACTS
   * C-CAMERA(???)
   * A-ANSWER
   * D-DECLINE
   * 0-SELECT
   *
   * because we don't have BACK button Button D also acts as BACK
   *
   *
   */
  // BACK_BUTTON_PIN = 0;
  // UP_BUTTON_PIN = 38;
  // SELECT_BUTTON_PIN = 37;
  // DOWN_BUTTON_PIN = 39;

  if (checkButton(0)) return BACK; 
  if (checkButton(37)) return SELECT; 
  if (checkButton(38)) return UP;    
  if (checkButton(39)) return DOWN;  

  return -1;
}
