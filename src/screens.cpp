#include "screens.h"
void makeCall(Contact contact);
void incomingCall(Contact contact);
void inbox();
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
    inbox();
    break;
  case 1:

    break;
  default:
    break;
  }
  String exampleMessage = "SAMPLE TEXT TO TEST MESSAGE | EmAIL | SMS | OR SoMETHING ~!@#$ \nKILL YOURSELF\n \n \n - Unknown";
  while (buttonsHelding() == -1)
    ;
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
    contact.number = "090X848X146";

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
          break;
        }
        break;
      }
      break;
    }
  }

#endif
}

bool settings()
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
  bool exit = false;
  while (!exit)
  {
    switch (a)
    {
    case -1:
      return false;
      break;
    case 0:
      pic = gallery();
      if (pic == -1)
      {
        exit = true;
        return false;
      }
      break;
    default:
      sysError("DOESN'T_EXIST");
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
        exit = true;
        break;
      default:
        return false;
        break;
      }
    }
  }

  return false;
}
void MainMenu()
{

  drawFromSd(0x5ADC01, 0, 26, 240, 294);
  drawFromSd(0x5D0341, 51, 71, 49, 49);

  int choice = 0;
  bool exit = false;
  while (!exit)
    switch (buttonsHelding())
    {
    case BACK:
    {
      exit = true;
      break;
    }
    case SELECT:
    {
      // middle
      switch (choice)
      {
      case 0:
        messages();
        break;
      case 1:
        contactss();
        break;
      case 2:
        e();
        break;
      case 3:
        exit = settings();
        break;
      }
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

  MainScreen();
}

int gallery()
{

  // drawFromSd((uint32_t)(0xD) + ((uint32_t)(0x22740) * ima), 0, 26, 240, 294);
  return listMenu(wallnames, ArraySize(wallnames), true, 2, "Change wallpaper");
}

void MainScreen()
{
  Serial.println("MAINSCREEN");

  drawFromSd((uint32_t)(0xD) + ((uint32_t)(0x22740) * ima), 0, 26, 240, 294);
  // bool exit = false;
  while (buttonsHelding() != UP)
    ;
  MainMenu();
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

  writeCustomFont(55, 185, contact.number, 1);
  drawFromSd(0x662B5B, 73, 90, 13, 14, true, 0xD6BA);
  for (;;)
    ;
}
void makeCall(Contact contact)
{
  bool calling = true;
  tft.fillScreen(0);
  drawStatusBar();

  changeFont(1);
  tft.setTextColor(0xffff);

  tft.setTextSize(1);
  tft.setCursor(20, 140 + 60);
  tft.print(contact.name);

  tft.setTextSize(2);
  tft.setCursor(0, 180 + 60);
  // tft.print(contact.number);
  writeCustomFont(5, 240, contact.number);

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
    ;
  tft.fillScreen(0);
  drawStatusBar();
}

void contactss()
{

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
}

void inbox()
{
  mOption example[] = {
    {"01/01 Shining Finger",mailimg[0]},
    {"01/01 Shining Finger",mailimg[0]},
    {"01/01 Shining Finger",mailimg[0]},
    {"01/01 Shining Finger",mailimg[0]},
    {"01/01 Shining Finger",mailimg[1]},
    {"01/01 Mayuri",mailimg[1]},
    {"01/01 John Titor",mailimg[1]},
    {"01/01 John Titor",mailimg[0]},
    {"01/01 Part-Time Warrior",mailimg[1]},
    {"01/01 Assistant",mailimg[2]},
    {"01/01 Nergon",mailimg[0]},
    {"01/01 John Titor",mailimg[0]},
    {"01/01 John Titor",mailimg[0]},
    
  };

  listMenu(example, ArraySize(example), false, 0, "MAIL");
}

void fileBrowser()
{
  // TODO
}

void downloadFile(String url, String path)
{
  // TODO
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

void messageActivity()
{
  // TODO
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
