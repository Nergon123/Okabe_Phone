#include "screens.h"
void messages()
{
  File lol2 = SD.open("/FIRMWARE/IMAGES.SG", FILE_READ);
  drawFromSd(0x613D45, 0, 26, 240, 294, lol2);
  drawFromSd(0x5DECA5, 0, 26, 240, 42, lol2);
  drawFromSd(0x5E8A25, 0, 68, 240, 128, lol2);
  String lol[] = {"Inbox", "Outbox"};
  int ch = choiceMenu(lol, 2, false);
  String exampleMessage = "SAMPLE TEXT TO TEST MESSAGE | EmAIL | SMS | OR SoMETHING ~!@#$ \nKILL YOURSELF\n \n \n - Unknown";
  while (digitalRead(0) == HIGH)
    ;
  lol2.close();
}
void knipka(){

          blemouse.begin();
          changeFont(0);
          tft.fillScreen(0x0);
          drawStatusBar();
          tft.setTextColor(0xffff);
            bool isConnected = true;
  
          while (digitalRead(37) == HIGH)
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
            if (digitalRead(0) == LOW && blemouse.isConnected())
            {
              while (digitalRead(0) == LOW)
                ;
              blemouse.move(0, 0, -1);
            }
          };
          blemouse.end();
}
void e()
{
#ifdef DEVMODE
  File lol2 = SD.open("/FIRMWARE/IMAGES.SG", FILE_READ);
  tft.fillScreen(0x0000);
  drawStatusBar();
  int choice = -1;
  while (choice != -2)
  {
    String debug[] = {"Emulate incoming call", "Emulate outgoing call", "Emulate recieving message", "Set battery level", "Set signal level", "BLUETOOTH", "WI-FI"};
    choice = listMenu(debug, ArraySize(debug), false, 2, "DEV MODE");
    String levels[] = {"0", "1", "2", "3"};
    String btMenu[] = {"BT Mouse", "BT Keyboard"};
    String btMouse[] = {"SCROLLDOWN"};
    switch (choice)
    {
    case 0:
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
  lol2.close();
#endif
}


bool settings()
{
  File lol2 = SD.open("/FIRMWARE/IMAGES.SG", FILE_READ);
  drawFromSd(0x613D45, 0, 26, 240, 294, lol2);
  drawFromSd(0x5E3B65, 0, 26, 240, 42, lol2);

  while (digitalRead(37) == LOW)
    ;
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
      blueScreen("DOESN'T_EXIST");
      break;
    }

    if (pic != -1)
    {
      String galch[] = {"Preview", "Confirm"};
      picch = choiceMenu(galch, 2, true);
      switch (picch)
      {
      case 0:
        drawFromSd((uint32_t)(0xD) + ((uint32_t)(0x22740) * pic), 0, 26, 240, 294, lol2);
        while (digitalRead(0) == HIGH)
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
  lol2.close();
  return false;
}
void MainMenu()
{
  File lol2 = SD.open("/FIRMWARE/IMAGES.SG", FILE_READ);
  drawFromSd(0x5ADC01, 0, 26, 240, 294, lol2);
  drawFromSd(0x5D0341, 51, 71, 49, 49, lol2);
  lol2.close();
  while (digitalRead(38) == LOW)
    ;
  int choice = 0;
  bool exit = false;
  while (!exit)
  {
    if (digitalRead(0) == LOW)
    {
      exit = true;
      break;
    }
    if (digitalRead(37) == LOW)
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
    }

    if (digitalRead(38) == LOW)
    {
      choice--;
      if (choice > 3)
        choice = 0;
      if (choice < 0)
        choice = 3;

      rendermenu(choice, false);
      // left
      while (digitalRead(38) == LOW)
        ;
    }

    if (digitalRead(39) == LOW)
    {
      choice++;
      if (choice > 3)
        choice = 0;
      if (choice < 0)
        choice = 3;

      rendermenu(choice, true);
      // right
      while (digitalRead(39) == LOW)
        ;
    }
  }

  MainScreen();
}

int gallery()
{
  File lol2 = SD.open("/FIRMWARE/IMAGES.SG", FILE_READ);
  // drawFromSd((uint32_t)(0xD) + ((uint32_t)(0x22740) * ima), 0, 26, 240, 294, lol2);
  if (!lol2.available())
    blueScreen("SD_CARD_NOT_AVAILABLE");
  while (digitalRead(37) == LOW)
    ;
  lol2.close();
  return listMenu(wallnames, ArraySize(wallnames), true, 2, "Change wallpaper");
}

void MainScreen()
{
  Serial.println("MAINSCREEN");
  File lol2 = SD.open("/FIRMWARE/IMAGES.SG", FILE_READ);
  drawFromSd((uint32_t)(0xD) + ((uint32_t)(0x22740) * ima), 0, 26, 240, 294, lol2);
  // bool exit = false;
  while (true)
  {
    if (digitalRead(38) == LOW)
    {
      while (digitalRead(38) == LOW)
        ;
      MainMenu();
    }
  }
  lol2.close();
}

void offlineCharging()
{
  tft.fillRect(35, 100, 160, 80, 0xffff);
  tft.fillRect(195, 120, 10, 40, 0xFFFF);
  tft.setCursor(70, 310);
  tft.setTextSize(1);
  changeFont(1);
  tft.print("vvLAUNCHvv");

  while (digitalRead(37) == HIGH)
  {
    int icc = chrg.isChargerConnected();
    int bp = chrg.getBatteryLevel();
    tft.fillRect(40, 105, 150, 70, 0x0000);
    if (chrg.isChargerConnected() && bp < 25)
      delay(500);

    tft.fillRect(45, 110, 43, 60, 0xffff);
    if (digitalRead(37) == LOW)
      break;

    if (chrg.isChargerConnected() && bp < 75)
      delay(500);
    tft.fillRect(45 + 48, 110, 43, 60, 0xffff);

    if (digitalRead(37) == LOW)
      break;
    if (chrg.isChargerConnected() && bp < 100)
      delay(500);

    tft.fillRect(45 + 96, 110, 43, 60, 0xffff);
    if (digitalRead(37) == LOW)
      break;
    delay(500);

    if (chrg.getBatteryLevel() == 100)
    {
      while (digitalRead(37) == HIGH)
        ;
    }
  }
  tft.fillScreen(0x0000);
}

void makeCall(Contact contact)
{
  // Ensure the contact number is valid
  if (contact.number.isEmpty())
  {
    Serial.println("Invalid contact number.");
    return;
  }

  // Send AT command to initiate a call
  Serial1.print("ATD");
  Serial1.print(contact.number);
  Serial1.println(";"); // End the command with a semicolon to start the call
  tft.fillScreen(0x0000);
  drawStatusBar();
  tft.setCursor(40, 120);
  tft.print("Calling...");
  // Optional: Wait and check for response
  unsigned long startTime = millis();
  while (millis() - startTime < 10000)
  { // Wait up to 10 seconds
    if (Serial1.available())
    {
      String response = Serial1.readStringUntil('\n');
      if (response.indexOf("OK") != -1)
      {
        Serial.println("Call initiated successfully.");
        tft.fillScreen(0x0000);
        drawStatusBar();

        tft.setCursor(40, 120);
        tft.print("VOICE ONLY");
        Serial.println(sendATCommand("AT+CLCC"));

        return;
      }
      else if (response.indexOf("ERROR") != -1)
      {
        Serial.println("Failed to initiate call.");
        return;
      }
    }
  }

  Serial.println("No response from SIM800L.");
}
void contactss()
{
  // Initialize contacts array
  populateContacts();

  // Convert contact names to a string array for display
  String contactNames[contactCount];
  for (int i = 0; i < contactCount; ++i)
  {
    contactNames[i] = contacts[i].name;
  }

  // Introduce a delay and draw the status bar
  delay(300);
  drawStatusBar();

  // Use listMenu to display the contacts list
  int selectedContactIndex = listMenu(contactNames, contactCount, false, 1, "Address Book");

  // If a contact was selected, display the context menu
  if (selectedContactIndex != -1)
  {
    int contextMenuSelection = choiceMenu(contmenu, 5, true);

    // Implement what happens after a context menu option is selected
    switch (contextMenuSelection)
    {
    case 0: // Call
      makeCall(contacts[selectedContactIndex]);
      break;
    case 1: // Outgoing
      // Implement outgoing call functionality
      break;
      // Add other cases as needed
    }
  }
}
