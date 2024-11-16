#include "funct.h"
String sendATCommand(String command)
{
  Serial1.println(command);
  delay(100); 

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
  //STOLEN FROM CHATGPT
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
    contacts[contactCount].phone = number;
    contacts[contactCount].name = name;
    contactCount++;

    if (contactCount >= MAX_CONTACTS)
      break; // Prevent overflow
  }
}

bool checkButton(int pin)
{
  if (digitalRead(pin) == LOW)
  {
    delay(50);
    while (digitalRead(pin) == LOW)
      ;
    return true;
  }
  return false;
}


void ButtonTask(void *pvParameters) {

  /*
   * SIDE BUTTON 10
   *  7 8 9
   * |M|I|B|
   *     ^=2
   * <=4 0=1 >=5
   * A=6 V=3 D=BACK
   * 1  2  3
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

int measureStringHeight(const String& text, int displayWidth) {
    //STOLEN FROM CHATGPT
    int lines = 1;               
    int lineWidth = 0;            
    int spaceWidth = tft.textWidth(" ");  
    int lineHeight = tft.fontHeight();   

    String word = "";  

    for (int i = 0; i < text.length(); i++) {
        char c = text[i];

        
        if (c == '\r' && i + 1 < text.length() && text[i + 1] == '\n') {
            lines++;         
            lineWidth = 0;    
            i++;             
            continue;
        }


        if (c == '\n' || c == '\r') {
            lines++;          
            lineWidth = 0;    
            continue;
        }

       
        if (c == ' ' || i == text.length() - 1) {
            if (i == text.length() - 1 && c != ' ') {
                word += c; 
            }

            int wordWidth = tft.textWidth(word);

           
            if (lineWidth + wordWidth > displayWidth) {
                lines++;          
                lineWidth = wordWidth + spaceWidth; 
            } else {
                lineWidth += wordWidth + spaceWidth; 
            }

            word = "";  
        } else {
            word += c;  
        }
    }

    
    return lines * lineHeight;
}

void saveContactsToJson(const Contact contacts[], size_t contactCount, const char* fileName) {
    if (!SD.begin()) {
        Serial.println("SD card initialization failed!");
        return;
    }


    File file = SD.open(fileName, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing.");
        return;
    }
    StaticJsonDocument<1024> doc;
    JsonArray contactArray = doc.to<JsonArray>();
    for (size_t i = 0; i < contactCount; i++) {
        JsonObject contact = contactArray.createNestedObject();
        contact["name"] = contacts[i].name;
        contact["phone"] = contacts[i].phone;
        contact["email"] = contacts[i].email;
    }

    // Serialize JSON to the file
    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write JSON to file.");
    } else {
        Serial.println("Contacts saved successfully.");
    }

    // Close the file
    file.close();
}