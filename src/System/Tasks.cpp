#include "Tasks.h"

// Function to suspend/resume the freeRTOS task on core 0
// @param suspend: true to suspend the task, false to resume it
void suspendCore(bool suspend) {
    if (TaskHCommand) {
        if (suspend) {
            vTaskSuspend(TaskHCommand);
            simIsBusy = false;
        }
        else { vTaskResume(TaskHCommand); }
    }
}

// Function to handle the idle task
void TaskIdleHandler(void *parameter) {

    uint32_t oldtime = millis();

    if (sendATCommand("AT").indexOf("OK") != -1) {
        ESP_LOGI("BOOT/SIM", "%s", "Setting up sim card please wait...");
        initSim();

        while (!_checkSim() &&
               millis() - oldtime < 10000); // check if sim card is usable for 10 whole seconds...
        populateContacts();
        ESP_LOGI("BOOT/SIM", "%s", "Done!");
    }
    else {
        ESP_LOGI("BOOT/SIM", "%s", "SIM card not responding");
        simIsUsable = false;
    }

    while (true) {

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
            vTaskDelay(pdMS_TO_TICKS(DBC_MS));
            backgroundBusy = false;
        }
        if (!simIsUsable) {
            DBC_MS      = 10000;
            simIsUsable = _checkSim();
            charge      = getChargeLevel();
            if (_signal != -1) {
                _signal     = -1;
                sBarChanged = true;
            }
        }
        else { DBC_MS = 3000; }
        backgroundBusy = false;
        vTaskDelay(pdMS_TO_TICKS(DBC_MS));
    }
}

// Function to initialize the tasks
void initTasks() {
    xTaskCreatePinnedToCore(TaskIdleHandler, "Core0Checker", 10000, NULL, 1, &TaskHCommand, 0);
}