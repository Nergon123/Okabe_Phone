#include "OTA.h"
#include <UI/UIElements.h>
#include <System/LanguageSystem.h>
void OTAactivity() {
    // Both current partition tables provide a single application slot.
    InfoWindow(getTranslation(TextKey::IW_NOT_SUPPORTED));
}
