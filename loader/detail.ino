
const uint8_t starData[] = {
	15, 15,
	1, 0,
	0,
	0,
	1,
	
	0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x77, 0xA0, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x7A, 0xF0, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x07, 0xAF, 0x77, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x0A, 0xF7, 0x7A, 0x00, 0x00, 0x00,
	0x7A, 0xA7, 0xAF, 0x77, 0xAA, 0x7F, 0x99, 0xA0,
	0x0A, 0x7A, 0xF7, 0x7A, 0xA7, 0xF9, 0x9A, 0x00,
	0x00, 0xAF, 0x77, 0xAA, 0x7F, 0x99, 0xA0, 0x00,
	0x00, 0x07, 0x7A, 0xA7, 0xF9, 0x9A, 0x00, 0x00,
	0x00, 0x00, 0xAA, 0x7F, 0x99, 0xA0, 0x00, 0x00,
	0x00, 0x00, 0xA7, 0xF9, 0x9A, 0xF0, 0x00, 0x00,
	0x00, 0x0A, 0x7F, 0x90, 0xAF, 0x99, 0x00, 0x00,
	0x00, 0x07, 0xF9, 0x00, 0x09, 0x9F, 0x00, 0x00,
	0x00, 0x7F, 0x90, 0x00, 0x00, 0xFA, 0x90, 0x00,
	0x00, 0xF9, 0x00, 0x00, 0x00, 0x09, 0xA0, 0x00,
};
Image star(starData);


bool titleScreenImageExists;
bool detailGameIsFav;

const char* msg_a_start;
uint8_t msg_a_w;
uint8_t msg_a_h;
uint8_t msg_a_x;
uint8_t msg_a_y;
void loadDetailedView() {
	detailGameIsFav = isGameFavorite();
	strcpy(nameBuffer, getCurrentGameFolder());
	strcpy(nameBuffer + strlen(nameBuffer), "/TITLESCREEN.BMP");
	titleScreenImageExists = SD.exists(nameBuffer);
	
	if (titleScreenImageExists) {
		gb.display.setColor(WHITE, BLACK);
		gb.display.setCursor(0, 0);
		gb.display.println(lang_loading);
		gb.updateDisplay();
		gb.display.init(nameBuffer);
		if ((gb.display.width() == 80 && gb.display.height() == 64) || (gb.display.width() == 160 && gb.display.height() == 128)) {
			gb.display.fontSize = gb.display.width() == 80 ? 1 : 2;
		} else {
			titleScreenImageExists = false;
		}
	}
	if (!titleScreenImageExists) {
		gb.display.init(80, 64, ColorMode::rgb565);
		gb.display.fontSize = 1;
	}
}

void loadGame() {
	if (titleScreenImageExists) {
		loadDetailedView(); // easiest way to fetch the first frame
	}
	strcpy(folderName, getCurrentGameFolder());
	getBinPath(nameBuffer);
	if (!titleScreenImageExists) {
		gb.display.clear();
		gb.display.setColor(WHITE, BLACK);
		gb.display.setCursor(0, 24);
		gb.display.println(lang_loading);
		gb.display.print(nameBuffer);
	}
	gb.updateDisplay();
	gb.bootloader.game(nameBuffer);
}

void detailedView() {
	loadDetailedView();
	Image buttonsIcons = Image(Gamebuino_Meta::buttonsIconsData);
	bool MMActiveLastFrame = false;  // True if MM was engaged the previous frame
	
	while (1) {
		while(!gb.update());
		gb.display.fontSize = gb.display.width() == 80 ? 1 : 2;

		uint8_t blockOffset = currentGame / BLOCK_LENGTH;
		uint8_t gameInBlock = currentGame % BLOCK_LENGTH;
		uint8_t b = getBlock(blockOffset);
		
		if (titleScreenImageExists) {
			gb.display.nextFrame();
		} else {
			gb.display.clear();
		}
		
		if (detailGameIsFav) {
			gb.display.drawImage(0, 0, star, star.width()*gb.display.fontSize, star.height()*gb.display.fontSize);
		}
		
		if (!titleScreenImageExists) {
			// center bar
			gb.display.setColor(BROWN);
			gb.display.fillRect(0, 15*gb.display.fontSize, 80*gb.display.fontSize, 9*gb.display.fontSize);
			
			// game name
			gb.display.setColor(WHITE);
			gb.display.setCursor(2*gb.display.fontSize, 17*gb.display.fontSize);
			gb.display.println(getCurrentGameFolder() + 1);
		}
		
		//blinking border
		gb.display.setColor((gb.frameCount % 8) >= 4 ? BROWN : BLACK);
		gb.display.drawRect(0, 0, gb.display.width(), gb.display.height());
		
		//blinking A button icon
		buttonsIcons.setFrame((gb.frameCount%8) >= 4);
		uint8_t scale = gb.display.width() == 80 ? 1 : 2;
		uint8_t w = buttonsIcons.width() * scale;
		uint8_t h = buttonsIcons.height() * scale;
		uint8_t x = gb.display.width() - w;
		uint8_t y = gb.display.height() - h;
		gb.display.drawImage(x, y, buttonsIcons, w, h);

		if (gb.metaMode.isActive() != MMActiveLastFrame) {
			loadDetailedView();
			MMActiveLastFrame = gb.metaMode.isActive();
		}
		
		if (gb.buttons.released(BUTTON_A)) {
			loadGame();
		}
		
		if (gb.buttons.released(BUTTON_B)) {
			return;
		}
		
		if (gb.buttons.released(BUTTON_DOWN)) {
			galleryView(1);
			loadDetailedView();
			continue;
		}
		
		if (gb.buttons.released(BUTTON_UP)) {
			galleryView(-1);
			loadDetailedView();
			continue;
		}
		
		if (gb.buttons.released(BUTTON_MENU)) {
			if (detailGameIsFav) {
				unfavoriteGame();
				gb.gui.popup(gb.language.get(lang_fav_removed), 50);
			} else if (favoriteGame()) {
				gb.gui.popup(gb.language.get(lang_fav_added), 50);
			}
			loadDetailedView();
			continue;
		}

		SPI.beginTransaction(SPISettings(24000000, MSBFIRST, SPI_MODE0));
		gb.tft.commandMode();
		SPI.transfer(gb.metaMode.isActive() ? 0x21 : 0x20);
		gb.tft.idleMode();
		SPI.endTransaction();
	}
}
