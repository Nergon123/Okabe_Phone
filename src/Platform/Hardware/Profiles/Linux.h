#include "Platform/Hardware/Hardware.h"
#ifdef PC
#include <Platform/FileSystem/FileSystem.h>
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
#include <zlib.h>
extern "C" {
#include <curl/curl.h>
}
struct WriteContext {
    FILE* file;
};
class DEV_LINUX : public iHW {
  public:
    void init() override {};
    void initStorage() override {
        IFileSystem* spiffs = new Std2FileSystem("spiffs/", FS_INTERNAL);
        IFileSystem* sdcard = new Std2FileSystem("sd/", FS_EXTERNAL);
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
    void  timeSet(time_t t) override {
        time_t now = time(nullptr);
        timeOffset = t - now;
    }
    time_t    timeGet() override { return time(nullptr) + timeOffset; }
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
    // Helper callback for libcurl to collect response
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        std::string* response = reinterpret_cast<std::string*>(userp);
        response->append(reinterpret_cast<char*>(contents), size * nmemb);
        return size * nmemb;
    }

    HttpAnswer httpSend(HttpMethod method, const NString& url, NString& payload,
                        const std::vector<HttpHeader>& headers, uint16_t timeout) override {
        HttpAnswer answ;

        CURL* curl = curl_easy_init();
        if (!curl) { return answ; }

        std::string responseString;

        // URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Timeout (optional, 5 sec)
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, (long)timeout);

        // Response callback
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

        // Headers
        struct curl_slist* chunk = nullptr;
        for (const auto& h : headers) {
            std::string line = h.name.c_str() + std::string(": ") + h.content.c_str();
            chunk            = curl_slist_append(chunk, line.c_str());
        }
        if (chunk) { curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk); }

        // HTTP Method
        switch (method) {
        case HttpMethod::GET: curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); break;

        case HttpMethod::POST:
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
            break;

        case HttpMethod::PUT:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
            break;

        case HttpMethod::DELETE_:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            if (!payload.isEmpty()) {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
            }
            break;

        case HttpMethod::PATCH:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
            break;
        }

        // Perform request
        CURLcode res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &answ.code);
            answ.response = responseString;
        }
        else {
            answ.code     = -1; // indicate error
            answ.response = "Curl error: " + std::string(curl_easy_strerror(res));
        }

        // Cleanup
        if (chunk) { curl_slist_free_all(chunk); }
        curl_easy_cleanup(curl);

        return answ;
    }

    void downloadFile(NString& url, IFile* fileToDownload,
                      std::function<void(size_t, size_t)> progressCallback) {
        if (!fileToDownload) { return; }

        CURL* curl = curl_easy_init();
        if (!curl) {
            fprintf(stderr, "Failed to initialize CURL\n");
            return;
        }

        // Set URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Write downloaded data into IFile
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fileToDownload);

        // Follow redirects
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Enable progress callback
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallbackCurl);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progressCallback);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

        // Perform the download
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "CURL download failed: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }

    uint32_t crc32(uint32_t crc, const uint8_t* buf, size_t len) override {
        return ::crc32(crc, buf, len);
    }

    bool isCharging() override {
        std::string path = getBatteryPath();
        if (path.empty()) { return false; }
        std::string status = readFile(path + "/status");
        std::transform(status.begin(), status.end(), status.begin(), ::tolower);
        return status.find("charging") != std::string::npos && status != "not charging";
    }

  private:
    time_t     timeOffset = 0;
    static int progressCallbackCurl(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
                                    curl_off_t ultotal, curl_off_t ulnow) {
        auto* userCallback = reinterpret_cast<std::function<void(size_t, size_t)>*>(clientp);
        if (userCallback && *userCallback) {
            (*userCallback)(static_cast<size_t>(dlnow), static_cast<size_t>(dltotal));
        }
        (void)ultotal;(void)ulnow;
        return 0; // return non-zero to abort transfer
    }
    static size_t writeCallback(void* ptr, size_t size, size_t nmemb, void* userdata) {
        NFile* file    = reinterpret_cast<NFile*>(userdata);
        size_t written = file->write(ptr, size * nmemb);
        return written;
    }

    struct utsname sys;
    bool           AudioAvailable = false;
    std::string    readFile(const std::string& path) {
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
        auto              opts = std::filesystem::directory_options::skip_permission_denied;
        for (const auto& entry : std::filesystem::directory_iterator(base, opts)) {
            std::string type = readFile(entry.path().string() + "/type");
            if (type == "Battery") {
                batteryPath = entry.path();
                return batteryPath;
            }
        }
        return "";
    }

    RenderTarget* GetScreen() override { return setupSDL2RenderTarget(); }

  private:
};
#endif // PC
