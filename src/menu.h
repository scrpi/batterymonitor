#ifndef _MENU_H_
#define _MENU_H_

#include <Arduino.h>

#define DECLARE(x) DECLARE_##x
#define DEFINE(x) DEFINE_##x,
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define FOR_EACH_1(item, x, ...) item(x)
#define FOR_EACH_2(item, x, ...)\
	item(x)\
	FOR_EACH_1(item,  __VA_ARGS__)
#define FOR_EACH_3(item, x, ...)\
	item(x)\
	FOR_EACH_2(item, __VA_ARGS__)
#define FOR_EACH_4(item, x, ...)\
	item(x)\
	FOR_EACH_3(item,  __VA_ARGS__)
#define FOR_EACH_5(item, x, ...)\
	item(x)\
	FOR_EACH_4(item,  __VA_ARGS__)
#define FOR_EACH_6(item, x, ...)\
	item(x)\
	FOR_EACH_5(item,  __VA_ARGS__)
#define FOR_EACH_7(item, x, ...)\
	item(x)\
	FOR_EACH_6(item,  __VA_ARGS__)
#define FOR_EACH_8(item, x, ...)\
	item(x)\
	FOR_EACH_7(item,  __VA_ARGS__)
#define FOR_EACH_9(item, x, ...)\
	item(x)\
	FOR_EACH_8(item,  __VA_ARGS__)
#define FOR_EACH_10(item, x, ...)\
	item(x)\
	FOR_EACH_9(item,  __VA_ARGS__)
#define FOR_EACH_11(item, x, ...)\
	item(x)\
	FOR_EACH_10(item,  __VA_ARGS__)
#define FOR_EACH_12(item, x, ...)\
	item(x)\
	FOR_EACH_11(item,  __VA_ARGS__)
#define FOR_EACH_13(item, x, ...)\
	item(x)\
	FOR_EACH_12(item,  __VA_ARGS__)
#define FOR_EACH_14(item, x, ...)\
	item(x)\
	FOR_EACH_13(item,  __VA_ARGS__)
#define FOR_EACH_15(item, x, ...)\
	item(x)\
	FOR_EACH_14(item,  __VA_ARGS__)
#define FOR_EACH_16(item, x, ...)\
	item(x)\
	FOR_EACH_15(item,  __VA_ARGS__)

#define CAT(arg1, arg2) arg1##arg2
#define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
#define FOR_EACH_NARG_(...) FOR_EACH_ARG_N(__VA_ARGS__)
#define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N
#define FOR_EACH_RSEQ_N() 16,15,14,13,12,11,10,9,8, 7, 6, 5, 4, 3, 2, 1, 0
#define FOR_EACH_(N, item, x, ...) CAT(FOR_EACH_, N)(item, x, __VA_ARGS__)
#define FOR_EACH(item, x, ...) FOR_EACH_(FOR_EACH_NARG(x, __VA_ARGS__), item, x, __VA_ARGS__)

#define MENUROOT(id, driver, node) struct menu id = { driver, &node }

#define MENU(id, label, ...)\
	FOR_EACH(DECLARE, __VA_ARGS__)\
	const char id##_label[] PROGMEM = label;\
	struct menu_node *const id##_items[] PROGMEM = {\
		FOR_EACH(DEFINE, __VA_ARGS__)\
	};\
	const struct menu_node_shadow id##_shadow PROGMEM = {\
		MT_SUBMENU, id##_label, id##_items,\
		ARRAY_SIZE(id##_items),\
		0, 0, 0, NULL\
	};\
	struct menu_node id = {\
		&id##_shadow, 0, false, 0\
	};

#define FIELD(...) FIELD_(__COUNTER__, __VA_ARGS__)
#define EXIT() EXIT_(__COUNTER__)

#define DECLARE_FIELD_(count, label, value, min, max, precision, unit)\
	const char m_fieldlabel##count[] PROGMEM = label;\
	const char m_fieldunit##count[] PROGMEM = unit;\
	const struct menu_node_shadow m_fieldshadow##count PROGMEM = {\
		MT_FIELD,\
		m_fieldlabel##count,\
		NULL,\
		0,\
		min,\
		max,\
		precision,\
		m_fieldunit##count\
	};\
	struct menu_node m_field##count = {\
		&m_fieldshadow##count, 0, false, value\
	};

#define DECLARE_EXIT_(count)\
	const struct menu_node_shadow m_exitshadow##count PROGMEM = {\
		MT_EXIT, NULL, NULL, 0, 0, 0, 0, NULL\
	};\
	struct menu_node m_exit##count = {\
		&m_exitshadow##count, 0, false, 0\
	};

#define DECLARE_SUBMENU(id)
#define DEFINE_FIELD_(count,...) &m_field##count
#define DEFINE_SUBMENU(id) &id
#define DEFINE_EXIT_(count) &m_exit##count

enum {
	MS_NORMAL        = 0,
	MS_INVERTED      = 1 << 0,
	MS_LINE_INVERTED = 1 << 1,
};

enum {
	MI_ESC,
	MI_UP,
	MI_DOWN,
	MI_OK,
};

typedef void (*print_fn)(struct menu_display_driver *drv, int x, int y, const char *str, int style);
typedef void (*clear_line_fn)(struct menu_display_driver *drv, int line_idx);
typedef void (*clear_fn)(struct menu_display_driver *drv);
typedef void (*display_fn)(struct menu_display_driver *drv);

struct menu_display_driver {
	int char_x;
	int char_y;
	print_fn print;
	clear_line_fn clear_line;
	clear_fn clear;
	display_fn display;
};

enum {
	MT_SUBMENU,
	MT_FIELD,
	MT_OPERATION,
	MT_EXIT,
};

struct menu_node_shadow {
	uint8_t type;
	const char *label;
	struct menu_node *const *items;
	uint8_t items_len;
	int16_t min;
	int16_t max;
	uint8_t precision;
	const char *unit;
};

struct menu_node {
	const struct menu_node_shadow *shadow;
	uint8_t cursor_idx;
	bool editing;
	int16_t value_int;
};

struct menu {
	struct menu_display_driver *driver;
	struct menu_node *root;
	struct menu_node *active;
	int16_t editing_int;
	bool parent_init;
};

void menu_update(struct menu *menu);
void menu_input(struct menu *menu, int input);

#endif