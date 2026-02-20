#include "Time.h"
#include <sys/time.h>

// Function to set up the time
// This function sets the system time to a specific date and time
// and saves it in the preferences storage
void SetUpTime() {
    struct tm tm_time = {};
    preferences.begin("TimePhone");
    hw->timeSet(preferences.getLong("TIME", mktime(&tm_time)));
    preferences.end();
}

// Function to save the time
// This function saves the current time in the preferences storage
// and updates the system time
// @param time The time to be saved
void SaveTime(time_t time) {
    preferences.begin("TimePhone");
    preferences.putLong("TIME", time);
    preferences.end();
    hw->timeSet(time);
}
