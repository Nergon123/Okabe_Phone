#include "../Hardware.h"
#ifdef PC
#include <Platform/Graphics/SDL2RenderTarget.h>
#include <SDL2/SDL.h>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <sys/utsname.h>
#include <thread>

class DEV_LINUX : public iHW {
  public:
    void init() override { };
    void initStorage() override {
        IFileSystem* spiffs = new StdFileSystem("spiffs/", FS_INTERNAL);
        IFileSystem* sdcard = new StdFileSystem("sd/", FS_EXTERNAL);
        sdcard->begin();
        spiffs->begin();
        VFS.mount("/sd", sdcard);
        VFS.mount("/spiffs", spiffs);
    }
    ulong micros() override {
        auto now = std::chrono::system_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
        return duration.count();
    }
    ulong millis() override { return micros() / 1000; };
    void  delay(ulong ms) override { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }
    void  setCPUSpeed(CPU_SPEED speed) override { (void)speed; };

    CPU_SPEED getCPUSpeed() override {
#ifndef EMU
#endif
        return CPU_DEFAULT;
    };
    const char* getDeviceName() override {
        if (uname(&sys) == 0) { return sys.nodename; };
        return "Unknown";
    }
    void shutdown() override {
#ifndef EMU
        system("poweroff");
#endif
    };
    void reboot() override {
#ifndef EMU
        system("reboot");
#endif
    };

    void setScreenBrightness(int8_t value) override {
#ifndef EMU
        char cmd[64];
        snprintf(cmd, sizeof(cmd), "brightnessctl set %d%% > /dev/null 2>&1", value);
        system(cmd);
#else
        (void)value;
#endif
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
    int getKeyInput() override { return 0; };

    int getBatteryCharge() override {
        std::string path = getBatteryPath();
        if (path.empty()) {
            return -1; // no battery
        }
        return std::atoi(readFile(path + "/capacity").c_str());
    }

    bool isCharging() override {
        std::string path = getBatteryPath();
        if (path.empty()) { return false; }
        std::string status = readFile(path + "/status");
        std::transform(status.begin(), status.end(), status.begin(), ::tolower);
        return status.find("charging") != std::string::npos && status != "not charging";
    }

  private:
    struct utsname sys;

    std::string readFile(const std::string& path) {
        std::ifstream file(path);
        std::string   value;
        if (file.is_open()) { std::getline(file, value); }
        if (!value.empty() && value.back() == '\n') { value.pop_back(); }
        return value;
    }
    std::string batteryPath;

    std::string getBatteryPath() {
        if (!batteryPath.empty()) { return batteryPath; }

        const std::string base = "/sys/class/power_supply/";
        for (const auto& entry : std::filesystem::directory_iterator(base)) {
            std::string type = readFile(entry.path().string() + "/type");
            if (type == "Battery") {
                batteryPath = entry.path();
                return batteryPath;
            }
        }
        return "";
    }

    RenderTarget* GetScreen() override { return setupSDL2RenderTarget(240, 320, "Emulator"); }

  private:
};
#endif // PC