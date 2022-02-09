#include <Wire.h>

#define USE_TIMER_1 true
#include <TimerInterrupt.h>

#include "sbat.h"
#include "menu.h"

// Push buttons
static struct {
	const int pin;
	unsigned long last_change;
	bool is_pressed;
	int menu_input;
} buttons[] = {
	{4, 0, false, MI_ESC},
	{5, 0, false, MI_UP},
	{6, 0, false, MI_DOWN},
	{7, 0, false, MI_OK},
};

#define BUTTON_DEBOUNCE_DELAY 10 // ms 

#define FONT_X 6
#define FONT_Y 8
#define FONT_LINE_SPACING 1

Adafruit_SH1106G display = Adafruit_SH1106G(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

void menu_print(struct menu_display_driver *drv, int x, int y, const char *str, int style) {
	bool inverted      = (style & MS_INVERTED) == MS_INVERTED;
	bool line_inverted = (style & MS_LINE_INVERTED) == MS_LINE_INVERTED;

	display.setTextSize(1);

	x *= FONT_X;
	y *= FONT_Y + FONT_LINE_SPACING;

	if (inverted || line_inverted) {
		display.setTextColor(SH110X_BLACK);
		if (line_inverted) {
			display.fillRect(0, y, OLED_WIDTH, FONT_Y + 1, SH110X_WHITE);
		} else {
			display.fillRect(x, y, x + strlen(str) * FONT_X - 1, FONT_Y + 1, SH110X_WHITE);
		}
	} else {
		display.setTextColor(SH110X_WHITE);
	}

	display.setCursor(x, y + 1); // Offset by one pixel to account for inverted padding.
	display.print(str);
}

void menu_clear(struct menu_display_driver *drv) {
	display.clearDisplay();
}

void menu_clear_line(struct menu_display_driver *drv, int line_idx) {
}

void menu_display(struct menu_display_driver *drv) {
	display.display();
}

struct menu_display_driver menu_drv = {
	21,
	7,
	menu_print,
	menu_clear_line,
	menu_clear,
	menu_display,
};

MENU(calibration, "CALIBRATION",
	FIELD("Volts Max1", 1470, 100, 1699, 2, "V"),
	FIELD("Volts Max5", 1470, 100, 1699, 2, "V"),
	FIELD("Volts Max6", 1470, 100, 1699, 2, "V"),
	FIELD("Volts Max7", 1470, 100, 1699, 2, "V"),
	EXIT()
);

// Label, Value, Min, Max, Precision, Unit
MENU(settings, "SETTINGS",
	SUBMENU(calibration),
	FIELD("Capacity", 200, 0, 999, 0, "Ah"),
	FIELD("Alarm2", 35, 0, 99, 0, "%"),
	FIELD("Alarm3", 35, 0, 99, 0, "%"),
	EXIT()
);

MENUROOT(menu, &menu_drv, settings);

void read_buttons() {
	unsigned long now;
	int i = 0;

	now = millis();

	for (i = 0; i < ARRAY_SIZE(buttons); ++i) {
		int pressed = !digitalRead(buttons[i].pin);
		if (!pressed) {
			buttons[i].is_pressed = false;
			continue;
		}
		if (buttons[i].is_pressed) {
			continue;
		}
		if (now - buttons[i].last_change > BUTTON_DEBOUNCE_DELAY) {
			buttons[i].is_pressed = true;
			buttons[i].last_change = now;
			menu_input(&menu, buttons[i].menu_input);
		}
	}
}

void setup() {
	int i = 0;

	Serial.begin(115200);

	for (i = 0; i < ARRAY_SIZE(buttons); ++i) {
		pinMode(buttons[i].pin, INPUT);
	}

	display.begin(OLED_ADDR, true);
	display.clearDisplay();
}

#define MENU_UPDATE_INTERVAL 100 // ms

void loop() {
	static unsigned long menu_last_update = 0;
	//show_stats(&display);

	read_buttons();

	if (millis() - menu_last_update > MENU_UPDATE_INTERVAL) {
		menu_update(&menu);
	}

	//delay(10);
}