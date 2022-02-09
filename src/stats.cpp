#include "sbat.h"

static void draw_battery(Adafruit_SH1106G *display, int x, int y, int w, int h, int fill_pct) {
	int tank_height = 0;
	int nsegs = 0;
	int i = 0;

	display->fillRect(x + 6, y, w - 12, 4, SH110X_WHITE);
	display->fillRect(x, y+4, w, h - 4, SH110X_WHITE);
	display->fillRect(x + 8, y + 2, w - 16, 2, SH110X_BLACK);
	display->fillRect(x + 2, y + 6, w - 4, h - 4 - 4, SH110X_BLACK);
	display->drawPixel(x, y + 4, SH110X_BLACK);
	display->drawPixel(x + w - 1, y + 4, SH110X_BLACK);
	display->drawPixel(x, y + h - 1, SH110X_BLACK);
	display->drawPixel(x + w - 1, y + h - 1, SH110X_BLACK);

	tank_height = h - 8;
	nsegs = (tank_height - 1) / 6; // 5 rows for segment, one row separator.

	// offset fill_pct so that fill_pct falls half way within the segment (looks more realistic)
	fill_pct += (100 / nsegs) / 2;
	nsegs = (nsegs * fill_pct) / 100;

	for (i; i < nsegs; ++i) {
		display->fillRect(x + 3, y + h - 2 - (i + 1) * 6, w - 6, 5, SH110X_WHITE);
	}
}

void show_stats(Adafruit_SH1106G *display) {
	char buf[32] = "";
	char fbuf[8] = "";
	float tmp = 0.f;
	int fill = 0;
	int i = 0;
	
	for (i = 400; i >= 0 ; --i) {
		fill = i / 4;
	
		display->clearDisplay();
		
		draw_battery(display, 7, 0, 32, 45, fill);
	
		if (fill < 10) {
			// For TextSize(2) each digit is 12px */
			display->setCursor(12, 50);
		} else if (fill < 100) {
			display->setCursor(6, 50);
		} else {
			display->setCursor(0, 50);
		}
		display->setTextSize(2);
		display->setTextColor(SH110X_WHITE);
		display->print(fill); display->print("%");

		int y = 0;
		// Ah
		tmp = (float)i / 2.0f;
		dtostrf(tmp, 0, 1, fbuf);
		display->setTextSize(2);
		display->setCursor(OLED_WIDTH - strlen(fbuf) * 12 - 14, y);
		display->print(fbuf);
		display->setTextSize(1);
		display->setCursor(OLED_WIDTH - 12, 7);
		display->print("Ah");

		// A
		y += 17;
		tmp = -5 + ((float)i / 130.f);
		float a = tmp;
		dtostrf(tmp, 0, 2, fbuf);
		sprintf(buf, "%s A", fbuf);
		display->setTextSize(1);
		display->setCursor(OLED_WIDTH - strlen(buf) * 6, y);
		display->print(buf);

		// V
		y += 10;
		tmp = 14.7;
		float v = tmp;
		dtostrf(tmp, 0, 2, fbuf);
		sprintf(buf, "%s V", fbuf);
		display->setTextSize(1);
		display->setCursor(OLED_WIDTH - strlen(buf) * 6, y);
		display->print(buf);

		// W
		y += 10;
		tmp = v * a;
		dtostrf(tmp, 0, 2, fbuf);
		sprintf(buf, "%s W", fbuf);
		display->setTextSize(1);
		display->setCursor(OLED_WIDTH - strlen(buf) * 6, y);
		display->print(buf);

		// Time
		y += 10;
		long to_empty_m = (long)i * 758;
		int m = (to_empty_m / 60) % 60;
		int h = (to_empty_m / (60 * 60)) % 24;
		int d = (long)to_empty_m / ((long)60 * 60 * 24);
		sprintf(buf, "%dd:%dh:%dm", d, h, m);
		display->setTextSize(1);
		display->setCursor(OLED_WIDTH - strlen(buf) * 6, y);
		display->print(buf);

		// Temp
		y += 10;
		sprintf(buf, "28.9 C");
		display->setTextSize(1);
		display->setCursor(OLED_WIDTH - strlen(buf) * 6, y);
		display->print(buf);

		display->display();
	}
}