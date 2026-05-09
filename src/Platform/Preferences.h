#pragma once
#include "FileSystem/VFS.h"
#include <map>
#include <string>

class Preferences {
  private:
    const char*                        PTAG = "Preferences";
    std::map<std::string, std::string> values;
    std::string                        filePath;
    std::string                        sectionName;
    bool                               initialized = false;

    bool saveToFile() {
        auto file = VFS.open(filePath, "w");
        if (!file) { return false; }

        file->printf("[%s]\n", sectionName.c_str());
        for (const auto& pair : values) {
            file->printf("%s=\"%s\"\n", pair.first.c_str(), pair.second.c_str());
        }
        file->close();
        delete file;
        return true;
    }

    bool loadFromFile() {
        auto file = VFS.open(filePath, "r");
        if (!file) {
            ESP_LOGE(PTAG, "Failed to open file %s", filePath.c_str());
            return false;
        }

        char buffer[256];
        bool inCorrectSection = false;
        while (file->readLine(buffer, sizeof(buffer))) {
            NString line(buffer);
            line = line.trim();
            // Check if this is a section header
            if (line.length() >= 3 && line.charAt(0) == '[' && line.charAt(line.length() - 1) == ']') {
                std::string foundSection = line.substring(1, line.length() - 1);
                inCorrectSection         = (foundSection == sectionName);
                continue;
            }

            if (!inCorrectSection) { continue; }

            size_t eqPos = line.indexOf('=');
            if (eqPos == std::string::npos) { continue; }

            std::string key   = line.substring(0, eqPos);
            std::string value = line.substring(eqPos + 1);

            // Remove quotes
            if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.size() - 2);
            }
            ESP_LOGI(PTAG, "Loaded key: '%s' with value: '%s'", key.c_str(), value.c_str());

            values[key] = value;
        }
        file->close();
        delete file;
        return true;
    }

  public:
    bool begin(const char* name, bool readOnly = false) {
        if (name == nullptr) { return false; }

        sectionName = name;

        // Try SD first, then SPIFFS
        filePath = std::string("/sd/") + name + ".cfg";
        if (!VFS.exists(filePath)) {
            filePath = std::string("/spiffs/") + name + ".cfg";
            if (!VFS.exists(filePath)) {
                ESP_LOGW(PTAG, "File %s does not exist, will create new one on save",
                         filePath.c_str());
            }
        }

        initialized = loadFromFile();
        (void)readOnly;
        return initialized;
    }

    void end() {
        values.clear();
        sectionName.clear();
        initialized = false;
    }

    bool putString(const char* key, const char* value) {
        if (!initialized) { return false; }
        values[key] = value;
        return saveToFile();
    }

    bool putInt(const char* key, int32_t value) {
        if (!initialized) { return false; }
        values[key] = std::to_string(value);
        return saveToFile();
    }

    bool putFloat(const char* key, float value) {
        if (!initialized) { return false; }
        values[key] = std::to_string(value);
        return saveToFile();
    }

    bool putLong(const char* key, int64_t value) {
        if (!initialized) { return false; }
        values[key] = std::to_string(value);
        return saveToFile();
    }

    std::string getString(const char* key, const char* defaultValue = "") {
        if (!initialized) { return defaultValue; }
        auto it = values.find(key);
        return (it != values.end()) ? it->second : defaultValue;
    }

    int32_t getInt(const char* key, int32_t defaultValue = 0) {
        if (!initialized) { return defaultValue; }
        auto it = values.find(key);
        return (it != values.end()) ? std::stoi(it->second) : defaultValue;
    }

    float getFloat(const char* key, float defaultValue = 0.0f) {
        if (!initialized) { return defaultValue; }
        auto it = values.find(key);
        return (it != values.end()) ? std::stof(it->second) : defaultValue;
    }

    int64_t getLong(const char* key, int64_t defaultValue = 0L) {
        if (!initialized) { return defaultValue; }
        auto it = values.find(key);
        return (it != values.end()) ? std::stoll(it->second) : defaultValue;
    }

    bool remove(const char* key) {
        if (!initialized) { return false; }
        auto it = values.find(key);
        if (it != values.end()) {
            values.erase(it);
            return saveToFile();
        }
        return false;
    }

    void clear() {
        if (!initialized) { return; }
        values.clear();
        saveToFile();
    }
};

extern Preferences preferences;