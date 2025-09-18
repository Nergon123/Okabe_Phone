#include "Time.h"

void SetUpTime() {

    struct tm tm_time = {};

    preferences.begin("TimePhone");

    systemTime         = preferences.getLong("TIME", mktime(&tm_time));
    struct timeval now = {.tv_sec = systemTime};
    settimeofday(&now, NULL);
    preferences.end();

    systemTimeInfo = *gmtime(&systemTime);
}

// Function to save the time
// This function saves the current time in the preferences storage
// and updates the system time
// @param time The time to be saved
void SaveTime(time_t time) {
    preferences.begin("TimePhone");
    preferences.putLong("TIME", time);

    preferences.end();

    time_t         t    = time;
    struct timeval data = {t, 0};
    systemTimeInfo      = *gmtime(&time);
    // Serial.printf("\nSTI %d:%d, TIME: %d:%d\n", systemTimeInfo.tm_hour, systemTimeInfo.tm_min,
    // tm_time.tm_hour, tm_time.tm_min);
    settimeofday(&data, NULL);
}
