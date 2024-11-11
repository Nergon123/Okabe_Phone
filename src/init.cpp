#include "init.h"

// Define global variables
int contactCount = 0;
IP5306 chrg;
TFT_eSPI tft = TFT_eSPI();
uint32_t ima = 0;
Preferences preferences;
Contact contacts[MAX_CONTACTS];
#ifdef DEVMODE
int charge_d = 3;
int signallevel_d = 3;
#endif
bool havenewmessages = false;
BleMouse blemouse("OkabePhone", "DEVELOPER", chrg.getBatteryLevel());

Contact examplecontact;
SDImage mailimg[4] = {
    SDImage(0x662DB1, 18, 21, 0, true),
    SDImage(0x662DB1+ (18*21*2), 18, 21, 0, true),
    SDImage(0x662DB1 + (18*21*2*2),18, 21, 0, true),
    SDImage(0x662DB1+ (18*21*2*3), 18, 21, 0, true)
};

void setup()
{

  tft.init();
  tft.fillScreen(0x0000);
  // WIRE.begin();
  pinMode(38, INPUT_PULLUP);
  pinMode(0, INPUT_PULLUP);
  pinMode(39, INPUT_PULLUP);
  pinMode(37, INPUT_PULLUP);
  chrg.begin(21, 22);

  analogWrite(TFT_BL, 50);
  /*if (chrg.isChargerConnected() == 1)
  {
    offlineCharging();
    tft.setTextFont(1);
  } */
  tft.setCursor(0, 0);
  tft.setTextSize(3);
  tft.println("NerBoot");
  tft.setTextSize(1);
  tft.println("NerBoot v.0.0.4 ALPHA\n\nBootloader written by NERGON\n\nResources located in sdcard\nfolder FIRMWARE\n");
  SPI.begin(14, 2, 15, 13);
#ifdef DEVMODE
  tft.println("\n       !!! DEVMODE ENABLED !!!\n\n       THIS MEANS THAT THIS \n       BUILD NOT FOR PRODUCTION\n");
#endif
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
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
  ima = preferences.getUInt("ima", 0); // Load ima with a default value of 0
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
  // Serial.println("test");

  // tft.pushImage(0,0,240,80,(uint16_t)image);
  // while(lol2.available())
  // Serial.print(lol2.read());
}

void loop(void)
{
  drawStatusBar();
  MainScreen();
}