#ifndef _BATMON_H_
#define _BATMON_H_

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

void show_stats(Adafruit_SH1106G *display);

#endif