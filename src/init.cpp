#include "init.h"

// Define global variables
IP5306 chrg;
TFT_eSPI tft = TFT_eSPI();
Preferences preferences;
BleMouse blemouse("OkabePhone", "DEVELOPER", chrg.getBatteryLevel());

#ifdef DEVMODE
int charge_d = 3;
int signallevel_d = 3;
#endif
int currentScreen = SCREENS::MAINSCREEN;
int millDelay = 0;
uint contactCount = 0;
uint32_t ima = 0;
bool isCalling = false;
bool ongoingCall = false;

bool sbchanged = true;
bool havenewmessages = false;
bool isAbleToCall = false;
bool simIsBusy = false;
bool isAnswered = false;

String currentNumber = "";
Contact contacts[MAX_CONTACTS];
Contact examplecontact = {0, "+1234567890", "NERGON", "enderman3025@gmail.com"};
TaskHandle_t TaskHCommand;
void TaskIdleHandler(void *parameter);
void initSim();

void setup()
{

  tft.init();
  tft.fillScreen(0x0000);
  analogWrite(TFT_BL, 20);

  pinMode(38, INPUT_PULLUP);
  pinMode(0, INPUT_PULLUP);
  pinMode(39, INPUT_PULLUP);
  pinMode(37, INPUT_PULLUP);

  chrg.begin(21, 22);

  /*if (chrg.isChargerConnected() == 1)
  {
    offlineCharging();
    tft.setTextFont(1);
  } */

  tft.setCursor(15, 3);
  tft.setTextSize(4);
  tft.setTextColor(tft.color24to16(0x666666));
  tft.println("NerBoot");
  tft.setCursor(12, 0);
  tft.setTextColor(0xFFFF);
  tft.println("NerBoot");
  tft.fillRect(0, 0, 10, 10, TFT_RED);
  tft.fillRect(0, 10, 10, 10, TFT_GREEN);
  tft.fillRect(0, 20, 10, 10, TFT_BLUE);
  tft.setTextSize(1);
  changeFont(0);
  tft.println("\n\nNerBoot v.0.0.4 ALPHA\n\nBootloader written by NERGON\n\nResources located in sdcard\nfolder FIRMWARE\n");
  SPI.begin(14, 2, 15, 13);

#ifdef DEVMODE
  tft.println("\n       !!! DEVMODE ENABLED !!!\n\n       THIS MEANS THAT THIS \n       BUILD NOT FOR PRODUCTION\n");
#endif

  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
  if (sendATCommand("AT").indexOf("OK") != -1)
  {
    initSim();
  }
  while (true)
  {
    if (Serial.available())
    {
      String req = Serial.readString();
      String ans = sendATCommand(req);
      tft.fillScreen(0);
      tft.setCursor(0, 0);
      tft.print(ans);
      Serial.println(ans);
    }
  }

  xTaskCreatePinnedToCore(
      TaskIdleHandler,
      "Core0Checker",
      10000,
      NULL,
      1,
      &TaskHCommand,
      0);

  Serial.print("Initializing SD card...");
  tft.println("\nInitializing SD card...");

  if (!SD.begin(chipSelect, SPI, 80000000))
  {
    Serial.println("SD Initialization failed!");
    tft.setTextColor(0xf800);
    tft.println("\nSD Initialization failed!");
    delay(1000);
    recovery("Something went wrong with your sd card\n(Possibly its just not there)\nAnyway download IMAGES.SG or something\nI think you can use some tools here");
    sysError("SD_CARD_INIT_FAIL");
  }
  else
  {
    Serial.println("SD Initialization done.");
    tft.println("\nSD Initialization done.\n");
  }

  if (!SD.exists("/FIRMWARE/IMAGES.SG"))
    recovery("No /FIRMARE/IMAGE.SG found\nhere some tools to help you!");

  File lol2 = SD.open("/FIRMWARE/IMAGES.SG");

  while (lol2.position() != 13)
  {
    tft.print((char)lol2.read());
  }
  lol2.close();

  preferences.begin("settings", false);
  ima = preferences.getUInt("ima", 0);
  contactCount = preferences.getUInt("contactCount", 0);

  if (!SD.exists("/DATA/MESSAGES.JSON"))
  {
    if (!SD.exists("/DATA"))
    {
      SD.mkdir("/DATA");
    }
    File file = SD.open("/DATA/MESSAGES.JSON", FILE_WRITE);
    file.print("{}");
    file.close();
  }

  while (digitalRead(37) == LOW)
    ;
  drawStatusBar();
}
void TaskIdleHandler(void *parameter)
{
  while (true)
  {
    while (!simIsBusy)
    {
      if (getSignalLevel() != _signal || getChargeLevel() != charge)
      {
        _signal = getSignalLevel();
        charge = getChargeLevel();
        sbchanged = true;
      }
      delay(2000);
    }
  }
}
void loop()
{
  screens();
}
void idle()
{
  checkVoiceCall();
  delay(50);
  if (millis() - millDelay > DBC_MS)
  {
    millDelay = millis();
    drawStatusBar();
  }
}
void screens()
{
  switch (currentScreen)
  {
  case SCREENS::MAINSCREEN:
    MainScreen();
    break;
  case SCREENS::MAINMENU:
    MainMenu();
    break;
  case SCREENS::MESSAGES:
    messages();
    break;
  case SCREENS::CONTACTS:
    contactss();
    break;
  case SCREENS::SETTINGS:
    settings();
    break;
  case SCREENS::E:
    e();
    break;
  }
}
void initSim()
{
  sendATCommand("AT+CLIP=1");
  sendATCommand("AT+CMGF=1");
}