#include "funct.h"
// TaskHandle_t TaskHandleATCommand;
String sendATCommand(String command, uint32_t timeout, bool background)
{
  bool _simIsBusy = simIsBusy;
  while(_simIsBusy){
    delay(100);
    _simIsBusy = simIsBusy;
  }
  simIsBusy = true;
  Serial1.println(command); // Send the AT command

  String response = "";
  uint32_t startTime = millis();

  // Wait for response or timeout
  while (millis() - startTime < timeout)
  {
    while (Serial1.available())
    {

      char c = Serial1.read(); // Read a single character
      response += c;           // Append it to the response
    }
  }
  if (!isCalling)
  {
    bool _isCalling = response.indexOf("RING\r") != -1;
    if (_isCalling)
    {
      int indexClip = response.indexOf("+CLIP:");
      int endindex = response.indexOf("\r", indexClip);
      int firIndex = response.indexOf("\"", indexClip);
      currentNumber = response.substring(firIndex, response.indexOf("\"", firIndex + 1));
      currentNumber.replace("\"", "");
      Serial.println(currentNumber);
    }
    isCalling = _isCalling;
  }
  else
  {
    if (response.indexOf("NO CARRIER") != -1)
    {
      Serial.println("Call Ended.");
      isCalling = false;
      currentNumber = "";
    }
  }
  if (response.indexOf("ERROR") != -1)
  {
    Serial.println("AT COMMAND FAILED :" + command);
  }
  simIsBusy = false;
  return response;
}

String getATvalue(String command, bool background = false)
{

  String response = sendATCommand(command, 1000, background);
  Serial.println(response);
  String result = "";

  if (response.indexOf("ERROR") != -1)
  {
    return "ERROR";
  }

  int startIdx = response.indexOf(":");
  if (startIdx != -1)
  {

    int endIdx = response.indexOf("\r", startIdx);

    result = response.substring(startIdx + 1, endIdx);
    result.trim();
    Serial.println(result);
  }
  else
  {
    result = response;
    result.trim();
  }

  return result;
}
int getChargeLevel()
{
  int charge = chrg.getBatteryLevel() / 25;

  int toIcon = (charge / 25) - 1;
#ifdef DEVMODE
  // toIcon = charge_d;
#endif
  if (toIcon > 3)
    toIcon = 3;
  if (toIcon < 0)
    toIcon = 0;
  return toIcon;
}
int getSignalLevel()
{

  int signal = -1; // temp placeholder to actual getSignalLevel
  String a = getATvalue("AT+CREG?", true);
  Serial.println("GETSIGNALLEVEL_CREG:" + a);
  if (a.charAt(2) == '1' || a.charAt(2) == '5')
  {
    String b = getATvalue("AT+CSQ");
    if (b != "ERROR")
    {
      char buf[5];
      b.substring(0, b.indexOf(',')).toCharArray(buf, 5);
      int strength = atoi(buf);
      if (strength != 99)
        if (signal > 3 || signal < 0)
          signal = strength / 8;
    }
  }

#ifdef DEVMODE
  // signal = signallevel_d;
#endif
  return signal;
}

void populateContacts()
{
    String response = sendATCommand("AT+CPBR=1,100"); // Query contacts from index 1 to 100
    Serial.println(response);
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

        // Extract index
        String indexStr = entry.substring(0, commaIndex);
        int contactIndex = indexStr.toInt();
        entry = entry.substring(commaIndex + 1);

        commaIndex = entry.indexOf(',');
        if (commaIndex == -1)
            break;

        // Extract phone number
        String number = entry.substring(0, commaIndex);
        number.replace("\"", "");
        entry = entry.substring(commaIndex + 1);

        // Extract name
        String name = entry;
        name.replace("\"", "");
        name.replace("145,", "");

        // Populate the contact structure
        contacts[contactCount].index = contactIndex; // Use the index from +CPBR
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

int buttonsHelding()
{

  /*
   * SIDE BUTTON 10
   *  7 8 9
   * |M|I|B|
   *     ^=2
   * <=4 0=1 >=5
   * A=6 V=3 D=BACK
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

#ifndef DEVMODE
  if (checkButton(0))
    return BACK;
  if (checkButton(37))
    return SELECT;
  if (checkButton(38))
    return UP;
  if (checkButton(39))
    return DOWN;
#endif

  if (Serial.available())
  {
    char input = Serial.read();
    switch (input)
    {
    case 'a':
      Serial.println("LEFT");
      return LEFT;
      break;
    case 'd':
      Serial.println("RIGHT");
      return RIGHT;
      break;
    case 'w':
      Serial.println("UP");
      return UP;
      break;
    case 's':
      Serial.println("DOWN");
      return DOWN;
      break;
    case ' ':
      Serial.println("SELECT");
      return SELECT;
      break;
    case 'e':
      Serial.println("ANSWER");
      return ANSWER;
      break;
    case 'q':
      Serial.println("BACK");
      return BACK;
      break;
    case '`':
      Serial.println("SOFTWARE RESET");
      ESP.restart();
      return -1;
      break;
    case 'm':
      Serial.println("MESSAGES");
      drawStatusBar();
      messages();
      MainScreen();
      return -1;
      break;
    case 'b':
      Serial.println("CONTACTS");
      drawStatusBar();
      contactss();
      MainScreen();
      return -1;
      break;
    case '*':
      Serial.println(input);
      return input;
      break;
    case '#':
      Serial.println(input);
      return input;
      break;
    default:
      if (input >= '0' && input <= '9')
      {
        Serial.println(input);
        return input;
      }
      break;
    }
  }

  return -1;
}

int measureStringHeight(const String &text, int displayWidth, int addLines)
{
  // STOLEN FROM CHATGPT
  int lines = 1;
  int lineWidth = 0;
  int spaceWidth = tft.textWidth(" ");
  int lineHeight = tft.fontHeight();

  String word = "";

  for (int i = 0; i < text.length(); i++)
  {
    char c = text[i];

    if (c == '\r' && i + 1 < text.length() && text[i + 1] == '\n')
    {
      lines++;
      lineWidth = 0;
      i++;
      continue;
    }

    if (c == '\n' || c == '\r')
    {
      lines++;
      lineWidth = 0;
      continue;
    }

    if (c == ' ' || i == text.length() - 1)
    {
      if (i == text.length() - 1 && c != ' ')
      {
        word += c;
      }

      int wordWidth = tft.textWidth(word);

      if (lineWidth + wordWidth > displayWidth)
      {
        lines++;
        lineWidth = wordWidth + spaceWidth;
      }
      else
      {
        lineWidth += wordWidth + spaceWidth;
      }

      word = "";
    }
    else
    {
      word += c;
    }
  }

  return lines * lineHeight;
}

void saveContactsToJson(const Contact contacts[], size_t contactCount, const char *fileName)
{
  File file = SD.open(fileName, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing.");
    return;
  }
  StaticJsonDocument<1024> doc;
  JsonArray contactArray = doc.to<JsonArray>();
  for (size_t i = 0; i < contactCount; i++)
  {
    JsonObject contact = contactArray.createNestedObject();
    contact["name"] = contacts[i].name;
    contact["phone"] = contacts[i].phone;
    contact["email"] = contacts[i].email;
  }

  // Serialize JSON to the file
  if (serializeJson(doc, file) == 0)
  {
    Serial.println("Failed to write JSON to file.");
  }
  else
  {
    Serial.println("Contacts saved successfully.");
  }

  // Close the file
  file.close();
}

void saveMessagesToJson(const Message messages[], size_t messageCount, const char *fileName)
{
  File file = SD.open(fileName, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing.");
    return;
  }

  StaticJsonDocument<2048> doc;
  JsonArray messageArray = doc.to<JsonArray>();

  for (size_t i = 0; i < messageCount; i++)
  {
    JsonObject messageObj = messageArray.createNestedObject();
    messageObj["index"] = messages[i].index;
    messageObj["status"] = messages[i].status;
    messageObj["isOutgoing"] = messages[i].isOutgoing;
    messageObj["contact"]["name"] = messages[i].contact.name;
    messageObj["contact"]["phone"] = messages[i].contact.phone;
    messageObj["contact"]["email"] = messages[i].contact.email;
    messageObj["subject"] = messages[i].subject;
    messageObj["content"] = messages[i].content;
    messageObj["date"] = messages[i].date;
  }

  if (serializeJson(doc, file) == 0)
  {
    Serial.println("Failed to write JSON to file.");
  }
  else
  {
    Serial.println("Messages saved successfully.");
  }

  // Close the file
  file.close();
}

void checkVoiceCall()
{
  if (isCalling && !ongoingCall)
  {

    Contact calling;
    calling.phone = currentNumber;
    for (int i = 0; i < contactCount; i++)
    {
      if (contacts[i].phone.indexOf(currentNumber) != -1)
      {
        calling = contacts[i];
        break;
      }
    }
    ongoingCall = true;
    incomingCall(calling);
  }
}