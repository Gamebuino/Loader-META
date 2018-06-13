#include "language.h"

int16_t demoInactivityCounter = 0;
const int16_t demoSingleFrameDelay = 25 * 4; // 4 seconds
const int16_t demoStartDelay = 25 * 20; // 10 seconds
bool demoModeActive;

#define DEMO_MODE_PATH "/Demo/Demo.bin"

bool testDemoMode() {
  //quite if demo is disabled
  if (!demoModeActive) {
    return false;
  }
  //check if no button is pressed
  for (uint8_t i = 0; i < Gamebuino_Meta::NUM_BTN; i++) {
    if (gb.buttons.states[i]) {
      demoInactivityCounter = 0;
      return false;
    }
  }
  //if inactive for a while go in demo mode
  demoInactivityCounter++;
  if (demoInactivityCounter == (demoStartDelay - 50)) {
    gb.gui.popup(gb.language.get(lang_settings_demomode), 50);
  }
  if (demoInactivityCounter > demoStartDelay) {
    return true;
  }
  return false;
}


void demoMode() {
  if (SD.exists(DEMO_MODE_PATH)) {
    gb.bootloader.game(DEMO_MODE_PATH);
  }
}
