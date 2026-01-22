#include "Tasks.h"

void suspendCore(bool suspend) { (void)suspend; };

// Function to handle the idle task
void TaskIdleHandler(void *) {
    uint32_t oldtime = hw->millis();
    time(&systemTime);
    if (sendATCommand("AT").indexOf("OK") != -1) {
        
        ESP_LOGI("BOOT/SIM", "%s", "Setting up sim card please wait...");
        initSim();

        while (!_checkSim() && NI_delay(oldtime, 10000)) {
            hw->delay(1000);
        } // check if sim card is usable for 10 whole seconds...
        populateContacts();
        ESP_LOGI("BOOT/SIM", "%s", "Done!");
    }
    else {
        ESP_LOGI("BOOT/SIM", "%s", "SIM card not responding");
        simIsUsable = false;
    }

    while (true) {
        time(&systemTime);
        while (!simIsBusy && simIsUsable) {
            backgroundBusy = true;
            if (getSignalLevel() != _signal || getChargeLevel() != charge) {
                if (ongoingCall) { stateCall = GetState(); }
                _signal = getSignalLevel();

                charge = getChargeLevel();

                simIsUsable = _checkSim();
                ESP_LOGI("SIGNAL", "Signal: %d, Charge: %d, SIM USABLE:%d\n", _signal, charge,
                         simIsUsable);
                if (contacts.size() == 0) { populateContacts(); }
                sBarChanged = true;
            }
            hw->delay(DBC_MS);
            backgroundBusy = false;
        }
        if (!simIsUsable) {
            // DBC_MS      = 10000;
            /// simIsUsable = _checkSim();// it can't breath
            charge = getChargeLevel();
            if (_signal != -1) {
                _signal     = -1;
                sBarChanged = true;
            }
        }
        // else { DBC_MS = 3000; }
        backgroundBusy = false;
        hw->delay(DBC_MS);
    }
}

void initBackgroundTasks() {
    LaunchTask(TaskIdleHandler, "Idle Task", nullptr, 4096, 3);
    LaunchTask(setBrightnessTask, "setBrightness", nullptr, 2048, 1);
};

void DeleteTask(TASK &task) {
#ifdef INC_FREERTOS_H
    vTaskDelete(task);
#endif
}

TASK LaunchTask(void (*function)(void *parameters), const char *name, void *parameters,
                int stackSize, int priority, int core) {
#ifdef INC_FREERTOS_H
    TaskHandle_t t;
    xTaskCreatePinnedToCore(function, name, stackSize, parameters, priority, &t, core);
    return t;
#elif defined(PC)
    return std::thread(); // TODO fix
    std::thread t(function, parameters);
    return t;
    (void)name;
    (void)priority;
    (void)core;
    (void)stackSize;
#endif
}