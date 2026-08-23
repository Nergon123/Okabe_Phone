#include "Time.h"
#include <sys/time.h>
#include <System/properties.h>
#include <cstdlib>

#define PROPERTY_KEY_TIME "sys.time"

// Function to set up the time
// This function sets the system time to a specific date and time
// and saves it in the preferences storage
void SetUpTime() {
    struct tm tm_time = {};
    hw->timeSet(property_get_long(PROPERTY_KEY_TIME, mktime(&tm_time)));
}

// Function to save the time
// This function saves the current time in the preferences storage
// and updates the system time
// @param time The time to be saved
void SaveTime(time_t time) {
   // property_set_long(PROPERTY_KEY_TIME, time);
   // hw->timeSet(time);
}
