#include "../Drivers/Battery/IP5306.h"
#include "../Hardware.h"
#include <Esp.h>
class DEV_ESP32 : iHW {
  public:
    void  init() override {
        
    };
    ulong micros() override { return ::micros(); };

    void setCPUSpeed(CPU_SPEED speed) override {

    };
    CPU_SPEED getCPUSpeed() override {

    };
    const char* getDeviceName() override {

    };
    void shutdown() override { reboot(); };
    void reboot() override { ESP.restart(); };

    void setScreenBrightness(int8_t value) override { analogWrite(TFT_BL, (value * 255) / 100); };
    char getInput() override { return Serial.read(); };
    int  getKeyInput() override {};
    int  getBatteryCharge() override { return bat.getBatteryLevel(); };
    bool isCharging() override { return bat.isChargerConnected(); };
};