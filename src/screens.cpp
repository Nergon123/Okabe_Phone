#include "screens.h"
void makeCall(Contact contact);
void incomingCall(Contact contact);
void inbox();
void numberInput(char first);
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
    String debug[] = {"Emulate incoming call", "Emulate outgoing call", "Emulate recieving message", "Set battery level", "Set signal level", "BLUETOOTH", "WI-FI"};
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

      makeCall(contact);
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
  for (;;)
    idle();
}

void makeCall(Contact contact)
{
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
  while (calling)
  {
    for (int i = 7; i >= 0; i--)
    {
      spinAnim(55, 60, 12, 6, i);
      delay(40);
      if (buttonsHelding() == DOWN)
      {
        hang = true;
        calling = false;
        break;
      }
      else if (buttonsHelding() == UP)
      {
        calling = false;
        break;
        // proceed to emulating call
      }
    }
    idle();
  }
  if (hang)
  {
    // if hang up
    return;
  }

  tft.fillScreen(0);
  drawStatusBar();
  drawFromSd(0x65D147, 40, 143, 160, 34);
  while (true /*"IF CALL IN PROGRESS" STATEMENT HERE*/)
    idle();
  tft.fillScreen(0);
  drawStatusBar();
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

  int selectedContactIndex = listMenu(contactNames, contactCount, false, 1, "Address Book");

  if (selectedContactIndex != -1)
  {
    int contextMenuSelection = choiceMenu(contmenu, 5, true);

    switch (contextMenuSelection)
    {
    case 0:
      makeCall(contacts[selectedContactIndex]);
      break;
    case 1:
      // OUTGOING
      break;
    }
  }
  currentScreen = SCREENS::MAINMENU;
}

void inbox(bool outbox)
{
  const char *title;
  if (outbox)
    title = "Outbox";
  else
    title = "Inbox";
  SDImage mailimg[4] = {
      SDImage(0x662DB1, 18, 21, 0, true),
      SDImage(0x662DB1 + (18 * 21 * 2), 18, 21, 0, true),
      SDImage(0x662DB1 + (18 * 21 * 2 * 2), 18, 21, 0, true),
      SDImage(0x662DB1 + (18 * 21 * 2 * 3), 18, 21, 0, true)};
  mOption example[] = {
      {"01/01 Shining Finger", mailimg[0]},
      {"01/01 Shining Finger", mailimg[0]},
      {"01/01 Shining Finger", mailimg[0]},
      {"01/01 Shining Finger", mailimg[0]},
      {"01/01 Shining Finger", mailimg[1]},
      {"01/01 Mayuri", mailimg[1]},
      {"01/01 John Titor", mailimg[1]},
      {"01/01 John Titor", mailimg[0]},
      {"01/01 Part-Time Warrior", mailimg[1]},
      {"01/01 Assistant", mailimg[2]},
      {"01/01 Nergon", mailimg[0]},
      {"01/01 John Titor", mailimg[0]},
      {"01/01 John Titor", mailimg[0]},

  };

  listMenu(example, ArraySize(example), false, 0, title);
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

void messageActivity(Contact contact, String date, String subject, String content, bool outcoming)
{

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
  int height = measureStringHeight(content) * 3;
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

    drawFromSd(0x639365, 0, 0, 240, 269);
    // tft.fillScreen(0xFFFF);
    drawFromSd(0, 0 + y_scr, in_mail[0]);
    tft.setCursor(24, 0 + y_text + y_scr);
    tft.println(date);

    drawFromSd(0, 24 + y_scr, in_mail[2]);
    tft.setCursor(24, 24 + y_text + y_scr);
    tft.println(!contact.name.isEmpty() ? contact.name : !contact.phone.isEmpty() ? contact.phone
                                                     : !contact.email.isEmpty()   ? contact.email
                                                                                  : "UNKNOWN");

    drawFromSd(0, 48 + y_scr, in_mail[3]);
    tft.setCursor(24, 48 + y_text + y_scr);
    tft.println(subject);

    tft.drawLine(0, 72 + y_scr, 240, 72 + y_scr, 0);

    printSplitString(content);
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
        exit = true;
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
}

void editContact()
{
  // TODO
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
char textInput(char input)
{
  const char buttons[12][12] = {
      "0 +E\r",
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
  int sizes[12];

  int pos = 0;
  for (int i = 0; i < 12; i++)
  {
    int b = 0;
    for (; b < 12; b++)
    {
      if (buttons[i][b] == '\r')
        break;
    }
    sizes[i] = b;
    Serial.println(b, DEC);
    b = 0;
  }

  while (true)
  {
    int c = buttonsHelding();
    if (c == input)
    pos++;
    else
    textInput(c);
  }
  return 255;
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
