#include "../Hardware.h"
#include <SDL2/SDL.h>
class Linux : iHW {
  public:
    void init() override {

    };
    ulong micros() override {
        auto now = std::chrono::system_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
        return duration.count();
    }
    ulong millis() override { return micros() / 1000; };

    void setCPUSpeed(CPU_SPEED speed) override {

    };
    CPU_SPEED getCPUSpeed() override {

    };
    const char* getDeviceName() override {

    };
    void shutdown() override {

    };
    void reboot() override {

    };

    void setScreenBrightness(int8_t value) override {

    };
    char getCharInput() override {
        char input = 0;

        SDL_Delay(1);
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                SDL_Quit();
                std::exit(0);
                break;
            case SDL_KEYDOWN:
                input = ev.key.keysym.sym;
                if (input == 'u') { input = '*'; }
                if (input == ']') { SDL_Quit(); }
                // printf("Key pressed: %c\n", input);

                return input;
                break;
                break;

            default: break;
            }
        }

        return 0;
    };
    int getKeyInput() override {

    };
    int getBatteryCharge() override {

    };
    bool isCharging() override {

    };
};