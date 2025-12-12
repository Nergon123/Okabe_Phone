#include "Tasks.h"

void suspendCore(bool suspend) { (void)suspend; };

// #ifndef PC
// TaskHandle_t TaskHCommand;
// // Function to suspend/resume the freeRTOS task on core 0
// // @param suspend: true to suspend the task, false to resume it
// void suspendCore(bool suspend) {
//     if (TaskHCommand) {
//         if (suspend) {
//             vTaskSuspend(TaskHCommand);
//             simIsBusy = false;
//         }
//         else { vTaskResume(TaskHCommand); }
//     }
// }
//
// Function to handle the idle task
void TaskIdleHandler(void *parameter) {

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
    LaunchTask(setBrightnessTask, "setBrightness", nullptr, 1024, 1);
};

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