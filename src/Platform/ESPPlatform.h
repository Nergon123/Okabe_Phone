#pragma once

#ifdef PC
#include <stdio.h>

class ESPClass {
public:
    void restart() {
        std::exit(0);
    }
    const char* getChipModel() { return "PC"; }
    unsigned getChipRevision() { return 1; }
};

extern ESPClass ESP;

#else
#include <esp_log.h>
#include <esp32/rom/ets_sys.h>
#endif