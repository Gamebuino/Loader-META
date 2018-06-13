#include "language.h"

int16_t demoInactivityCounter = 0;
const int16_t demoSingleFrameDelay = 25*4; // 4 seconds
const int16_t demoStartDelay = 25*20; // 10 seconds
bool demoModeActive;

bool testDemoMode() {
  //quite if demo is disabled
	if (!demoModeActive) {
		return false;
	}
  //hold MENU to trigger demo mode
  if(gb.buttons.held(BUTTON_MENU, 25)){
    return true;
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
	if (demoInactivityCounter > demoStartDelay) {
		return true;
	}
	return false;
}

bool exitDemoMode() {
	if(gb.buttons.released(BUTTON_A) | gb.buttons.released(BUTTON_B)) {
			return true;
	}
	return false;
}
void waitButtonRelease() {
	while(1) {
		while(!gb.update());
		bool release = true;
		for (uint8_t i = 0; i < Gamebuino_Meta::NUM_BTN; i++) {
			if (gb.buttons.states[i]) {
				release = false;
				break;
			}
		}
		if (release) {
			break;
		}
	}
}

const uint8_t DEMO_PATTERN_COLORS = 6;
const uint8_t demo_color_pattern[DEMO_PATTERN_COLORS][3] = {{0, 0, 0}, {255, 127, 0},  {100, 200, 0}, {255, 127, 0}, {0, 0, 0}, {255, 255, 200},};
const uint8_t demo_fade_speed = 40;
const uint8_t demo_led_spacing = 10;

void demoLEDHandler() {
	for (uint8_t i = 0; i < 4; ++i) {
		uint8_t current_pattern_color = (gb.frameCount + i * demo_led_spacing) / demo_fade_speed % DEMO_PATTERN_COLORS;
		uint8_t target_pattern_color = current_pattern_color == DEMO_PATTERN_COLORS - 1 ? 0 : current_pattern_color + 1;
		uint8_t progress = (gb.frameCount + i * demo_led_spacing) % demo_fade_speed;  // From 0 to 100

		uint8_t r = map(progress, 0, demo_fade_speed, demo_color_pattern[current_pattern_color][0], demo_color_pattern[target_pattern_color][0]);
		uint8_t g = map(progress, 0, demo_fade_speed, demo_color_pattern[current_pattern_color][1], demo_color_pattern[target_pattern_color][1]);
		uint8_t b = map(progress, 0, demo_fade_speed, demo_color_pattern[current_pattern_color][2], demo_color_pattern[target_pattern_color][2]);
		Color c = gb.createColor(r, g, b);
		gb.lights.drawPixel(0, i, c);
		gb.lights.drawPixel(1, i, c);
	}
}

bool demoLoadNextBitmap(File& root) {
	root.getName(nameBuffer, NAMEBUFFER_LENGTH);
	uint8_t i = strlen(nameBuffer);
	nameBuffer[i] = '/';
	i++;
	nameBuffer[i] = '\0';
	while (!strstr(nameBuffer, ".BMP") && !strstr(nameBuffer, ".bmp")) {
		File entry = root.openNextFile();
		if (!entry) {
			entry.close();
			root.rewindDirectory();
			return false;
		}
		entry.getName(nameBuffer + i, NAMEBUFFER_LENGTH - i);
		entry.close();
	}
	SerialUSB.println(nameBuffer);
	gb.display.init(nameBuffer);
	demoInactivityCounter = demoSingleFrameDelay;
	return true;
}

void demoLoadNextLangBitmap(File& root, File& lang, bool& alt) {
	alt = !alt;
	File* load = alt ? &root : &lang;
	if(!demoLoadNextBitmap(*load)) {
		demoLoadNextBitmap(*load);
	}
}

void demoMode() {
	File root = SD.open("SAMPLES");
	File lang;
	const char* lang_dir = gb.language.get(lang_folder_names);
	if (SD.exists(lang_dir)) {
		lang = SD.open(lang_dir);
	} else {
		lang = SD.open("EN");
	}
	bool alt = true;
	Image buttonsIcons(Gamebuino_Meta::buttonsIconsData);
	if (!demoLoadNextBitmap(root)) {
		demoInactivityCounter = 0;
		return;
	}
	gb.gui.popup(gb.language.get(lang_settings_demomode), 50);

	while(1) {
		while(!gb.update());
		demoLEDHandler();
		gb.display.nextFrame();
		
		// draw blinking A button
		buttonsIcons.setFrame((gb.frameCount%8) >= 4);
		uint8_t scale = gb.display.width() == 80 ? 1 : 2;
		uint8_t w = buttonsIcons.width() * scale;
		uint8_t h = buttonsIcons.height() * scale;
		uint8_t x = gb.display.width() - w;
		uint8_t y = gb.display.height() - h;
		gb.display.drawImage(x, y, buttonsIcons, w, h);
		
		if (gb.display.frames > 1) { // animation
			if (gb.display.frame >= (gb.display.frames - 1)) {
				demoLoadNextLangBitmap(root, lang, alt);
			}
		} else { // static screenshot
			demoInactivityCounter--;
			if (demoInactivityCounter <= 0) {
				demoLoadNextLangBitmap(root, lang, alt);
			}
		}
		if (exitDemoMode()) {
			break;
		}
	}
	gb.lights.clear();
	gb.display.init(80, 64, ColorMode::rgb565);
	demoInactivityCounter = 0;
	waitButtonRelease();
}
