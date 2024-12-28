#include "screens.h"
void callActivity(Contact contact);
void incomingCall(Contact contact);
void inbox();
void numberInput(char first);
bool messageActivity(Message message);
void messageActivityOut(Contact contact, String subject, String content, bool sms);
bool confirmation(String reason)
{
  //"ARE YOU SURE YOU WANT TO DO THIS?"
  return true;
}

void messages()
{

  drawFromSd(0x613D45, 0, 26, 240, 294);
  drawFromSd(0x5DECA5, 0, 26, 240, 42);
  drawFromSd(0x5E8A25, 0, 68, 240, 128);
  String lol[] = {"Inbox", "Outbox"};
  int ch = choiceMenu(lol, 2, false);
  switch (ch)
  {
  case 0:
    inbox(false);
    break;
  case 1:
    inbox(true);
    break;
  default:
    break;
  }
  currentScreen = SCREENS::MAINMENU;
}

void knipka()
{

  blemouse.begin();
  changeFont(0);
  tft.fillScreen(0x0);
  drawStatusBar();
  tft.setTextColor(0xffff);
  bool isConnected = !blemouse.isConnected();

  while (buttonsHelding() != DOWN)
  {

    if (isConnected != blemouse.isConnected())
    {
      tft.fillScreen(0x0);
      tft.setCursor(0, 50);
      tft.setTextSize(5);
      tft.print("Knipka\n");
      tft.setTextSize(2);
      tft.println(String("CONNECTED?" + String(blemouse.isConnected())));
      isConnected = blemouse.isConnected();
    }
    if (buttonsHelding() == BACK && blemouse.isConnected())
    {

      blemouse.move(0, 0, -1);
    }
  };
  blemouse.end();
}

void e()
{

#ifdef DEVMODE
  int choice = -2;
  while (choice != -1)
  {
    String debug[] = {
        "Emulate incoming call",
        "Emulate outgoing call",
        "Emulate recieving message",
        "Set battery level",
        "Set signal level",
        "BLUETOOTH",
        "WI-FI",
    };
    choice = listMenu(debug, ArraySize(debug), false, 2, "DEV MODE");
    String levels[] = {"0", "1", "2", "3"};
    String btMenu[] = {"BT Mouse", "BT Keyboard"};
    String btMouse[] = {"SCROLLDOWN"};
    Contact contact;
    contact.index = 0;
    contact.name = "Daru";
    contact.phone = "090X848X146";

    switch (choice)
    {

    case 0:
      incomingCall(contact);
      break;
    case 1:

      callActivity(contact);
      break;
    case 3:
      charge_d = choiceMenu(levels, ArraySize(levels), true);
      drawStatusBar();
      break;
    case 4:
      signallevel_d = choiceMenu(levels, ArraySize(levels), true);
      drawStatusBar();
      break;
    case 5:
      int btChoice = listMenu(btMenu, ArraySize(btMenu), false, 2, "BLUETOOTH");
      switch (btChoice)
      {
      case -1:
        break;
      case 0:
        int btMChoice = listMenu(btMouse, ArraySize(btMouse), false, 2, "BT Mouse");
        switch (btMChoice)
        {
        case 0:
          knipka();
          sbchanged = true;
          break;
        }
        break;
      }
      break;
    }
    idle();
  }

#endif
  currentScreen = SCREENS::MAINMENU;
}

void settings()
{

  drawFromSd(0x613D45, 0, 26, 240, 294);
  drawFromSd(0x5E3B65, 0, 26, 240, 42);
  String lol[] = {
      "Change Wallpaper",
      "Phone ringtone",
      "Mail ringtone",
  };
  int a = choiceMenu(lol, 3, false);
  int pic = -1;
  int picch = -2;

  while (true)
  {
    switch (a)
    {
    case -1:
      currentScreen = SCREENS::MAINMENU;
      return;
      break;
    case 0:
      pic = gallery();
      if (pic == -1)
      {
        currentScreen = SCREENS::MAINMENU;
        return;
      }
      break;
    default:
      currentScreen = SCREENS::MAINMENU;
      return;
      break;
    }

    if (pic != -1)
    {
      String galch[] = {"Preview", "Confirm"};
      picch = choiceMenu(galch, 2, true);
      switch (picch)
      {
      case 0:
        drawFromSd((uint32_t)(0xD) + ((uint32_t)(0x22740) * pic), 0, 26, 240, 294);
        while (buttonsHelding() != BACK)
          ;
        break;
      case 1:
        preferences.putUInt("ima", pic);
        ima = pic;
        currentScreen = SCREENS::MAINSCREEN;
        return;
        break;
      default:

        break;
      }
    }
    idle();
  }
  currentScreen = SCREENS::MAINMENU;
}
void MainMenu()
{

  drawFromSd(0x5ADC01, 0, 26, 240, 294);
  drawFromSd(0x5D0341, 51, 71, 49, 49);

  int choice = 0;
  bool exit = false;
  while (true)
  {
    switch (buttonsHelding())
    {
    case BACK:
    {
      currentScreen = SCREENS::MAINSCREEN;
      return;
      break;
    }
    case SELECT:
    {
      // middle
      switch (choice)
      {
      case 0:

        currentScreen = SCREENS::MESSAGES;
        return;
        break;
      case 1:
        currentScreen = SCREENS::CONTACTS;
        return;
        break;
      case 2:
        currentScreen = SCREENS::E;
        return;
        break;
      case 3:
        currentScreen = SCREENS::SETTINGS;
        return;
        break;
      }
      drawFromSd(0x5ADC01, 0, 26, 240, 294);
      rendermenu(choice, false);
      break;
    }

    case UP:
    {
      choice--;
      if (choice > 3)
        choice = 0;
      if (choice < 0)
        choice = 3;

      rendermenu(choice, false);
      // left
      break;
    }

    case DOWN:
    {
      choice++;
      if (choice > 3)
        choice = 0;
      if (choice < 0)
        choice = 3;

      rendermenu(choice, true);
      break;
    }
    }
    idle();
  }
}

int gallery()
{

  const String wallnames[] = {
      "Wallpaper 1",
      "Wallpaper 2",
      "Wallpaper 3",
      "IBN5100",
      "Red jelly",
      "The head of doll",
      "Mayuri jelly",
      "Fatty Gero Froggy",
      "Burning Gero Froggy",
      "Upa",
      "Metal Upa",
      "Seira",
      "Seira After awaking",
      "Gero Froggy",
      "Cat Gero Froggy",
      "Cow Gero Froggy",
      "FES",
      "Gero Froggies",
      "Calico Gero Froggies",
      "Gold Upa",
      "FES2",
      "Erin 1",
      "Erin 2",
      "Orgel Sisters",
      "Mayuri",
      "Kurisu",
      "Moeka",
      "Luka",
      "Faris",
      "Suzuha",
      "UNCONFIRMED",
      "Popping steiner",
      "Wallpaper 4",
      "NukariyaIce",
      "MayQueen",
      "Upa ♪",
      "Wallpaper 5",
      "Rabikuro",
      "Wallpaper 6",
      "Space Froggies",
      "Wallpaper 7",
      "Nae"};
  // drawFromSd((uint32_t)(0xD) + ((uint32_t)(0x22740) * ima), 0, 26, 240, 294);
  return listMenu(wallnames, ArraySize(wallnames), true, 2, "Change wallpaper");
}

void MainScreen()
{
  Serial.println("MAINSCREEN");
  changeFont(0);
  drawFromSd((uint32_t)(0xD) + ((uint32_t)(0x22740) * ima), 0, 26, 240, 294);
  // bool exit = false;
  int c = -1;
  while (c != UP)
  {
    int c = buttonsHelding();
    idle();
    if ((c >= '0' || c == '*' || c == '#') && c <= '9')
    {
      numberInput(c);
      drawFromSd((uint32_t)(0xD) + ((uint32_t)(0x22740) * ima), 0, 26, 240, 294);
    }
    if (c == UP)
      break;
  }

  currentScreen = SCREENS::MAINMENU;
}

void offlineCharging()
{
  tft.fillRect(35, 100, 160, 80, 0xffff);
  tft.fillRect(195, 120, 10, 40, 0xFFFF);
  tft.setCursor(70, 310);
  bool exit = false;
  while (buttonsHelding() == -1 || !exit)
  {

    int mill = millis();
    if (buttonsHelding() != -1)
      break;
    int icc = chrg.isChargerConnected();
    int bp = chrg.getBatteryLevel();
    tft.fillRect(40, 105, 150, 70, 0x0000);
    if (chrg.isChargerConnected() && bp < 25)
      while (millis() - mill < 500)
        if (buttonsHelding() != -1)
        {
          exit = true;
          break;
        }

    tft.fillRect(45, 110, 43, 60, 0xffff);

    if (chrg.isChargerConnected() && bp < 75)
      while (millis() - mill < 1000)
        if (buttonsHelding() != -1)
        {
          exit = true;
          break;
        }
    tft.fillRect(45 + 48, 110, 43, 60, 0xffff);

    if (chrg.isChargerConnected() && bp < 100)
      while (millis() - mill < 1500)
        if (buttonsHelding() != -1)
        {
          exit = true;
          break;
        }
    tft.fillRect(45 + 96, 110, 43, 60, 0xffff);

    while (chrg.getBatteryLevel() == 100)
      if (buttonsHelding() != -1)
      {
        exit = true;
        break;
      }
  }
  tft.fillScreen(0x0000);
}

void incomingCall(Contact contact)
{
  drawFromSd((uint32_t)(0xD) + ((uint32_t)(0x22740) * ima), 0, 26, 240, 294);

  drawFromSd(0x5F7A25, 0, 90, 240, 134);
  changeFont(1);
  tft.setTextColor(0);
  tft.setCursor(15, 170);
  tft.print(contact.name);
  tft.setTextColor(0xf800);
  tft.setCursor(90, 140);
  changeFont(4);
  tft.setTextSize(1);
  tft.print("Ca l l ing");
  drawFromSd(0x661AF3, 45, 105, 42, 50, true, 0xD6BA);

  writeCustomFont(55, 185, contact.phone, 1);
  drawFromSd(0x662B5B, 73, 90, 13, 14, true, 0xD6BA);
  int button = buttonsHelding();
  while (isCalling)
  {
    button = buttonsHelding();
    idle();

    switch (button)
    {
    case ANSWER:
      if (sendATCommand("ATA").indexOf("NO CARRIER") == -1)

      {

        callActivity(contact);
      }
      else
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
void makeCall(Contact contact)
{
  isAnswered = true;
  sendATCommand("ATD" + contact.phone + ";");
  callActivity(contact);
}
void callActivity(Contact contact)
{
  ongoingCall = true;
  bool calling = true;
  tft.fillScreen(0);
  sbchanged = true;
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
  delay(50);
  bool hang = false;
  while (!isAnswered)
  {
    for (int i = 7; i >= 0; i--)
    {
      spinAnim(55, 60, 12, 6, i);
      delay(40);
      if (buttonsHelding() == DECLINE)
      {
        hang = true;
        calling = false;
        break;
      }
      else if (buttonsHelding() == UP)
      {
        calling = false;
        break;
      }
    }

    idle();
  }
  if (hang)
  {
    // if hang up
    sendATCommand("ATH");
    ongoingCall = false;

    return;
  }

  tft.fillScreen(0);
  drawStatusBar();
  drawFromSd(0x65D147, 40, 143, 160, 34);
  while (ongoingCall)
  {
    if (buttonsHelding() == DECLINE)
    {
      sendATCommand("ATH");
    }
    idle();
  }
  tft.fillScreen(0);
  drawStatusBar();
  ongoingCall = false;
  isAnswered = false;
}

void contactss()
{
  const String contmenu[] = {
      "Call",
      "Outgoing",
      "Edit",
      "Create",
      "Delete"};

  populateContacts();

  String contactNames[contactCount];
  for (int i = 0; i < contactCount; ++i)
  {
    contactNames[i] = contacts[i].name;
  }

  delay(300);
  drawStatusBar();
  bool exit = false;
  while (!exit)
  {
    populateContacts();

    String contactNames[contactCount];
    for (int i = 0; i < contactCount; ++i)
    {
      contactNames[i] = contacts[i].name;
    }
    int selectedContactIndex = listMenu(contactNames, contactCount, false, 1, "Address Book");

    if (selectedContactIndex != -1)
    {
      int contextMenuSelection = -1;
      if (selectedContactIndex == -2)
      {
        const String choice = "Create";
        contextMenuSelection = choiceMenu({&choice}, 1, true);
        if (contextMenuSelection == 0)
        {
          editContact({lastContactIndex + 1});
        }
        else
          exit = true;
      }
      else
      {
        contextMenuSelection = choiceMenu(contmenu, 5, true);

        switch (contextMenuSelection)
        {

        case 0:
          makeCall(contacts[selectedContactIndex]);
          exit = true;
          break;
        case 1:
          // OUTGOING
          messageActivityOut(contacts[selectedContactIndex], "", "", true);
          break;
        case 2:
          editContact(contacts[selectedContactIndex]);
          break;
        case 3:
          // CREATE
          editContact({lastContactIndex + 1});
          break;
        case 4:
          // DELETE
          sendATCommand("AT+CPBW=" + String(contacts[selectedContactIndex].index));
          break;
        }
      }
    }
    else
      exit = true;
  }
  currentScreen = SCREENS::MAINMENU;
}

void inbox(bool outbox)
{
  const char *title;
  if (outbox)
    title = "Outbox";
  else
    title = "Inbox ";

  int count = 0;
  bool exit = false;
  Message *messages = nullptr;
  while (!exit)
  {
    exit = true;
    parseMessages(messages, count);
    mOption *a = new mOption[count];
    for (int i = 0; i < count; i++)
    {
      a[count - i - 1] = messages[i];
    }

    Serial.println("LISTMENU");
    int choice = -2;
    while (choice != -1)
    {
      choice = listMenu(a, count, false, 0, title);
      if (choice >= 0)
      {
        exit = !messageActivity(messages[count - choice - 1]);
        if (!exit)
          choice = -1;
      }
    }
  }
}

void fileBrowser(File dir)
{
  SDImage folderIcon = SDImage(0x663983, 18, 18, 0, true);
  SDImage fileIcon = SDImage(0x663984 + (18 * 18 * 2), 18, 18, 0, true);
  String currentPath = dir.path();
  File file = dir.openNextFile();
  dir.rewindDirectory();
  mOption *options = new mOption[64];
  int choice = -2;
  int i = 0;
  do
  {
    i = 0;

    dir = SD.open(currentPath);
    dir.rewindDirectory();
    file = dir.openNextFile();
    Serial.println(currentPath);
    while (file)
    {
      if (file.name() != "")
      {

        Serial.println();
        Serial.print(file.name());
        if (file.isDirectory())
        {
          Serial.print("/");
          options[i].icon = folderIcon;
          options[i].label = file.name() + String("/");
        }
        else
        {
          options[i].icon = fileIcon;
          options[i].label = file.name();
        }
        i++;
        file = dir.openNextFile();
      }
    }
    choice = listMenu(options, i, false, 2, "FILE MANAGER");
    if (options[choice].label.endsWith("/"))
    {
      options[choice].label.remove(options[choice].label.lastIndexOf("/"));
      currentPath += options[choice].label;
    }
    dir.close();
  } while (choice != -1);

  file.close();
  dir.close();
}

void downloadFile(const char *url, const char *path)
{
  tft.fillScreen(0);
  tft.setCursor(0, 0);
  HTTPClient http;
  http.setTimeout(20000);
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK)
  {
    WiFiClient *stream = http.getStreamPtr();
    File file = SD.open(path, FILE_WRITE);

    if (file)
    {
      const uint16_t chunksize = 512;
      uint8_t buffer[chunksize];
      uint32_t downloaded = 0;
      ulong startTime = millis();

      Serial.println("Starting file download...");

      while (stream->connected() || stream->available())
      {
        int bytesRead = stream->readBytes(buffer, sizeof(buffer));

        if (bytesRead > 0)
        {
          file.write(buffer, bytesRead);
          downloaded += bytesRead;

          tft.print("Downloaded: ");
          tft.print(downloaded);
          tft.println(" bytes");
          tft.setCursor(0, 0);

          file.flush();
        }
        else if (bytesRead == 0)
        {
          break;
        }
        else
        {
          Serial.println("Read error or disconnected");
          break;
        }
      }

      float elapsedTime = (millis() - startTime) / 1000.0;
      tft.println("Download complete.");
      tft.print("Total bytes downloaded: ");
      tft.println(downloaded);
      tft.print("Elapsed time: ");
      tft.print(elapsedTime);
      tft.println(" seconds");
      tft.print("Download speed: ");
      tft.print(downloaded / elapsedTime);
      tft.println(" bytes/second");

      file.close();
    }
    else
    {
      Serial.println("Failed to open file on SD card.");
    }
  }
  else
  {
    Serial.printf("Download failed, HTTP error code: %d\n", httpCode);
  }

  http.end();
}

void mediaPlayer(String path)
{
  // TODO
}

void ringtoneSelector()
{
  // TODO
}

void mailRingtoneSelector()
{
  // TODO
}

bool messageActivity(Contact contact, String date, String subject, String content, int index, bool outcoming, bool sms)
{
  // returns if deleted
  if (sms)
  {
    String response = sendATCommand("AT+CMGR=" + String(index));
    if (response.indexOf("OK") == -1)
    {
      content = "======\nERROR LOADING FOLLOWING MESSAGE\n======";
    }
    int resIndex = 0;
    for (int i = 0; i < 7; i++)
    {
      resIndex = response.indexOf('\"', ++resIndex);
    }
    int endIndex = response.indexOf('\"', ++resIndex);
    Serial.println("resIndex:" + String(resIndex) + "\nendIndex:" + endIndex);
    date = response.substring(resIndex, endIndex);
    date.replace(',', ' ');
    date = date.substring(0, date.lastIndexOf(':'));
    Serial.println(date);
    content = response.substring(endIndex + 3, response.lastIndexOf("OK") - 2);
  }
  content.trim();
  const String choices[3] = {"Reply", "Return", "Delete"};
  drawStatusBar();
  SDImage in_mail[4] = {
      SDImage(0x663E91, 23, 24, 0, false), // TIME
      SDImage(0x663E91 + (23 * 24 * 2), 23, 24, 0, false),
      SDImage(0x663E91 + (23 * 24 * 2 * 2), 23, 24, 0, false), // "TO"
      SDImage(0x663E91 + (23 * 24 * 2 * 3), 23, 24, 0, false),
  };
  drawFromSd(0x5DAF1F, 0, 26, 240, 25);
  drawFromSd(0x5DDDFF, 0, 26, 25, 25);
  int y_jump = 22;
  int y_scr = 0;
  int y_text = 18;

  tft.setCursor(30, 45);
  tft.setTextSize(1);
  changeFont(1);
  tft.setTextColor(0xffff);
  bool deleted = false;
  tft.print("Recieve Mail");
  tft.setTextColor(0);
  tft.setViewport(0, 51, 240, 269, true);
  bool exit = false;
  while (!exit)
  {
    deleted = false;
    tft.setTextColor(0);
    drawFromSd(0x639365, 0, 0, 240, 269);
    // tft.fillScreen(0xFFFF);
    drawFromSd(0, 0 + y_scr, in_mail[0]);
    tft.setCursor(24, 0 + y_text + y_scr);
    tft.println(date);

    drawFromSd(0, 24 + y_scr, in_mail[1]);
    tft.setCursor(24, 24 + y_text + y_scr);
    tft.println(!contact.name.isEmpty() ? contact.name : !contact.phone.isEmpty() ? contact.phone
                                                     : !contact.email.isEmpty()   ? contact.email
                                                                                  : "UNKNOWN");

    tft.drawLine(0, 48 + y_scr, 240, 48 + y_scr, 0);
    int height = measureStringHeight(content, printSplitString(content));
    int ch = -2;
    int r = -1;
    while (r == -1)
    {
      r = buttonsHelding();
      switch (r)
      {
      case DOWN:
        if (y_scr > -height)
          y_scr -= y_jump;
        break;
      case UP:
        if (y_scr < 0)
          y_scr += y_jump;
        break;
      case BACK:
        ch = choiceMenu(choices, 3, true);
        switch (ch)
        {
        case -1:
          exit = true;
          break;
        case 0:
          messageActivityOut(contact, "", "", true);
          break;

        case 2:
          if (confirmation("ARE YOU SURE YOU WANT TO DELETE MESSAGE"))
          {
            sendATCommand("AT+CMGD=" + String(index), 5000);
            exit = true;
            deleted = true;
          }
          break;
        }
        break;
      default:
        break;
      }
    }
    tft.resetViewport();
    idle();
    tft.setViewport(0, 51, 240, 269, true);
  }
  tft.resetViewport();
  return deleted;
}

bool messageActivity(Message message)
{
  return messageActivity(message.contact, message.date, message.subject, message.content, message.index, false, true);
}

void messageActivityOut(Contact contact, String subject, String content, bool sms)
{
#define TLVP 238
  // TEXT LIMIT VERTICAL VIEWPORT

  int limit = 0;
  if (sms)
  {
    limit = 160;
  }
  else
  {
    limit = 400;
  }
  String messagebuf;

  int text_pos = 0;
  int position = 0;
  drawStatusBar();
  SDImage in_mail[4] = {
      SDImage(0x663E91, 23, 24, 0, false),
      SDImage(0x663E91 + (23 * 24 * 2), 23, 24, 0, false),
      SDImage(0x663E91 + (23 * 24 * 2 * 2), 23, 24, 0, false),
      SDImage(0x663E91 + (23 * 24 * 2 * 3), 23, 24, 0, false),
  };
  drawFromSd(0x5DAF1F, 0, 26, 240, 25);
  drawFromSd(0x5DDDFF, 0, 26, 25, 25);
  int y_jump = 22;
  int y_scr = 0;
  int y_text = 18;
  int min_y = y_scr;
  tft.setCursor(30, 45);
  tft.setTextSize(1);
  changeFont(1);
  tft.setTextColor(0xffff);

  tft.print("Send Mail");
  tft.setTextColor(0);

  tft.setViewport(0, 51, 240, 269, true);
  bool exit = false;
  while (!exit)
  {
    position = 0;
    drawFromSd(0x639365, 0, 0, 240, 269);
    // tft.fillScreen(0xFFFF);
    // position += 24;
    drawFromSd(0, position + y_scr, in_mail[2]);
    tft.setCursor(24, position + y_text + y_scr);
    tft.println(!contact.name.isEmpty() ? contact.name : !contact.phone.isEmpty() ? contact.phone
                                                     : !contact.email.isEmpty()   ? contact.email
                                                                                  : "UNKNOWN");

    if (!subject.isEmpty() || !sms)
    {
      position += 24;
      drawFromSd(0, position + y_scr, in_mail[3]);
      tft.setCursor(24, position + y_text + y_scr);
      tft.println(subject);
    }
    position += 24;
    tft.drawLine(0, position + y_scr, 240, position + y_scr, 0);
    int height = measureStringHeight(messagebuf, 240, printSplitString(String(messagebuf)));
    Serial.println(messagebuf);
    int r = -1;
    String a[] = {"Return", "Send Message", "Delete", "Save To Drafts"};
    int u;
    bool returns = false;
    while (r == -1)
    {

      if (y_scr < min_y)
      {
        min_y = y_scr;
      }
      r = buttonsHelding();
      switch (r)
      {
      case DOWN:
        if (y_scr > -height)
          y_scr -= y_jump;
        break;
      case UP:
        if (y_scr < 0)
          y_scr += y_jump;
        break;
      case BACK:

        u = choiceMenu(a, 4, true);
        tft.setTextColor(0);
        switch (u)
        {
        case 0:
          Serial.println("Return");
          r = -2;
          exit = true;
          break;
        case 1:
          Serial.println("SEND MESSAGE");
          if (sendATCommand("AT+CMGF=1").indexOf("OK") != -1)
          {
            sendATCommand("AT+CMGS=\"" + contact.phone + "\"");
            sendATCommand(messagebuf + char(26));
          }
          tft.resetViewport();
          return;
          break;
        case 2:
          Serial.println("DELETE");

          break;
        case 3:
          Serial.println("Save to Drafts");
          break;

        default:
          break;
        }
        break;
      default:
        if (r >= '0' && r <= '9')
        {
          while (r != BACK && r != UP && r != DOWN)
          {
            char l = textInput(r);
            r = buttonsHelding();
            if (l != 0)
            {

              Serial.println("Y:" + String(tft.getCursorY()));
              drawCutoutFromSd(SDImage(0x639365, 240, 269, 0, false), 0, 240, 120, 20, 0, 240);

              if (messagebuf.length() < limit)
                if (l != '\b')
                {
                  messagebuf += l;
                  
                  tft.print(l);
                }
                else
                {
                  messagebuf.remove(messagebuf.length()-1);
                  r = BACK;
                  // WORKAROUND TODO: DO BETTER
                  //update after week: i actually did this better
                }
              if (tft.getCursorY() > TLVP)
              {
                y_scr -= y_jump;
                if (y_scr < min_y)
                  min_y = y_scr;
                r = BACK;
              }

              if (y_scr != min_y)
              {
                y_scr = min_y;
                r = BACK;
              }
              Serial.println("MINIMUM:" + String(y_scr));
            }
          }
        }
        break;
      }
    }
    tft.resetViewport();
    idle();
    tft.setViewport(0, 51, 240, 269, true);
  }
  tft.resetViewport();
}
void editContact(Contact contact)
{
  int pos = 0;
  int textboxes = 2;
  int buttons = 2;
  int direction;
  drawFromSd(0x5DAF1F, 0, 26, 240, 25);
  drawFromSd(0x5DDDFF + (0x4E2 * 1), 0, 26, 25, 25);
  drawStatusBar();
  tft.setCursor(30, 45);
  tft.setTextSize(1);
  changeFont(1);
  tft.setTextColor(0xffff);
  String boxString[textboxes] = {contact.name, contact.phone};
  tft.print("Edit Contact");
  drawFromSd(0x639365, 0, 51, 240, 269);
  textbox("Name", contact.name, 70, true, false, false);
  textbox("Phone Number", contact.phone, 120, true, false, false);

  button("SAVE", 10, 285, 100, 28);
  button("CANCEL", 120, 285, 100, 28);
  bool save = false;
  bool cancel = false;
  bool exit = false;
  while (!exit)
  {
    switch (pos)
    {
    case 0:
      boxString[pos] = textbox("Name", boxString[pos], 70, false, false, true, &direction);
      break;
    case 1:
      boxString[pos] = textbox("Phone Number", boxString[pos], 120, false, false, true, &direction, true);
      break;
    case 2:
      save = button("SAVE", 10, 285, 100, 28, true, &direction);
      if (save)
      {
        if (boxString[1].isEmpty())
          break;
        if (boxString[0].isEmpty())
        {
          boxString[0] = boxString[1];
        }

        sendATCommand("AT+CPBS=\"SM\"");
        String request = "AT+CPBW=" +
                         String(contact.index) + ",\"" +
                         boxString[1] + "\"," +
                         String(boxString[1].indexOf("+") == 0 ? 145 : 129) +
                         ",\"" + boxString[0] + "\"";
        String result = sendATCommand(request);
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
    switch (direction)
    {
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

void imageViewer()
{
  // TODO
}

void recovery(String message)
{
  tft.setCursor(0, 40);
  tft.fillScreen(0);
  tft.setTextFont(1);
  tft.setTextSize(4);
  tft.setTextColor(0x00FF);
  tft.print("=RECOVERY=\n\n");
  tft.setTextSize(1);
  tft.setTextColor(0xFFFF);
  tft.println(message);
  for (;;)
    ;
}

void numberInput(char first)
{
  tft.fillRect(0, 300, 240, 20, 0);
  sbchanged = true;
  drawStatusBar();
  const uint8_t max_char = 13;
  char input[max_char];
  for (int i = 0; i < max_char; i++)
    input[i] = ' ';
  input[0] = first;
  char c = 255;
  int pos = 1;
  changeFont(0);
  tft.setTextSize(3);
  tft.fillRect(0, 300, 240, 20, 0);
  tft.setCursor(0, 300);
  tft.print(input);

  while (true)
  {
    while (c == 255)
    {
      c = buttonsHelding();
      idle();
    }

    switch (c)
    {
    case ANSWER:
      makeCall({0, input});
      return;
      break;
    case RIGHT:
      pos--;
      input[pos] = ' ';

      tft.fillRect(0, 300, 240, 20, 0);
      tft.setCursor(0, 300);
      tft.print(input);
      break;
    case BACK:
      return;
      break;
    default:
      break;
    }
    if (pos == 0)
      return;
    if ((c >= '0' || c == '*' || c == '#') && c <= '9' && pos < max_char)
    {
      input[pos] = c;
      pos++;
      tft.fillRect(0, 300, 240, 20, 0);
      tft.setCursor(0, 300);
      tft.print(input);
      c = 255;
    }
    idle();
    Serial.println(c, DEC);
    c = 255;
  }
}

void showText(const char *text, int pos)
{

  int h = tft.getViewportHeight();
  int w = tft.getViewportWidth();
  int vx = tft.getViewportX();
  int vy = tft.getViewportY();
  bool viewport = false;
  if (tft.getViewportHeight() < 320)
  {
    tft.resetViewport();
    viewport = true;
  }

  int pfont = currentFont;
  int textColor = tft.textcolor;
  int textSize = tft.textsize;
  changeFont(0);
  tft.setTextSize(2);

  tft.setCursor(0, INPUT_LOCATION_Y);

  for (int i = 0; i < (int)(strchr(text, '\r') - text); i++)
  {

    if (i != pos)
      tft.setTextColor(0xFFFF, 0, true);
    else
      tft.setTextColor(0xFFFF, 0x001F, true);
    if (text[i] == '\n')
      tft.print("NL");
    else if (text[i] == '\b')
      tft.print("<-");
    else
      tft.print(text[i]);
  }

  tft.textcolor = textColor;
  changeFont(pfont);
  tft.setTextSize(textSize);
  if (viewport)
    tft.setViewport(vx, vy, w, h);
}

char textInput(int input, bool onlynumbers, bool nonl)
{
  if (input == -1)
    return 0;
  char buttons[12][12] = {
      "0+@\b \n\r",
      "1,.()\r",
      "2ABCabc\r",
      "3DEFdef\r",
      "4GHIghi\r",
      "5JKLjkl\r",
      "6MNOmno\r",
      "7PQRSpqrs\r",
      "8TUVtuv\r",
      "9WXYZwxyz\r",
      "*\r",
      "#\r"};
  if (nonl)
  {
    buttons[0][5] = '\r';
  }
  if (onlynumbers)
  {
    buttons[0][3] = '\r';
    for (int i = 1; i < 12; i++)
    {
      buttons[i][1] = '\r';
    }
  }
  bool first = true;
  int sizes[12];
  char result;
  int pos = 0;
  int currentIndex =
      input >= '0' && input <= '9'
          ? input - 48
      : input == '*' ? 10
      : input == '#' ? 11
                     : -1;
  Serial.println("INPUT:" + String(input));
  Serial.println("currentIndex" + String(currentIndex));
  if (currentIndex == -1)
  {
    Serial.println("UNKNOWN BUTTON:" + String(input));
    return '\0';
  }

  for (int i = 0; i < 12; i++)
  {
    int b = 0;
    for (; b < 12; b++)
    {
      if (buttons[i][b] == '\r')
        break;
    }
    sizes[i] = b;
    // Serial.println(b, DEC);
    b = 0;
  }
  int mil = millis();
  pos = -1;
  int curx = tft.getCursorX();
  int cury = tft.getCursorY();
  while (millis() - mil < DIB_MS)
  {
    curx = tft.getCursorX();
    cury = tft.getCursorY();
    // Serial.println("POSITION:" + String(pos));
    int c = buttonsHelding();
    if (c == input || first)
      if (pos < (int)(strchr(buttons[currentIndex], '\r') - buttons[currentIndex]))
      {
        mil = millis();
        pos++;
        result = buttons[currentIndex][pos];
        Serial.println("POSITION:" + String(pos));

        showText(buttons[currentIndex], pos);
        tft.setCursor(curx, cury);
      }
      else
      {
        mil = millis();
        pos = 0;
        Serial.println("POSITION:" + String(pos));
        result = buttons[currentIndex][pos];
        showText(buttons[currentIndex], pos);
        tft.setCursor(curx, cury);
      }
    first = false;
  }

  tft.fillRect(0, 300, 240, 30, 0x00);

  tft.setCursor(curx, cury);
  bool viewport = false;
  int h = tft.getViewportHeight();
  int w = tft.getViewportWidth();
  int vx = tft.getViewportX();
  int vy = tft.getViewportY();
  if (tft.getViewportHeight() < 320)
  {
    tft.resetViewport();
    viewport = true;
  }

  int x = 0;
  int y = 240;
  drawCutoutFromSd(SDImage(0x639365, 240, 269, 0, false), x, y, 120, 20, x, y);
  if (viewport)
    tft.setViewport(vx, vy, w, h);
  Serial.println("Result:" + String(result));
  if (result == '\r')
  {
    return '\0';
  }
  return result;
}

void WiFiList()
{
  WiFi.begin();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  while (true)
  {
    int count = WiFi.scanNetworks();
    String names[50];
    uint8_t a;
    uint8_t *l;
    int32_t c, d;
    for (int i = 0; i < count; i++)
    {
      WiFi.getNetworkInfo(i, names[i], a, c, l, d);
    }
    listMenu(names, count, false, 0, "WI-FI");
  }
}
