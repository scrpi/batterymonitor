#include <string.h>

#include "menu.h"

static struct menu_node_shadow shadow;
static struct menu_node *shadow_items[16];
static char buf[32];

static void display_submenu(struct menu *menu, struct menu_node *item,
                            struct menu_node_shadow *data, int line) {
	struct menu_display_driver *drv = menu->driver;
	strncpy_P(buf, data->label, sizeof(buf));
	drv->print(drv, 1, line, buf, MS_NORMAL);
	drv->print(drv, strlen(buf) + 1, line, "\x1A", MS_NORMAL);
}

static void display_field(struct menu *menu, struct menu_node *item,
                          struct menu_node_shadow *data, int line) {
	struct menu_display_driver *drv = menu->driver;
	char buf[32];
	int i, len;
	char *s;

	strncpy_P(buf, data->label, sizeof(buf));
	drv->print(drv, 1, line, buf, MS_NORMAL);

	if (item->editing) {
		snprintf(buf, sizeof(buf), "%d", menu->editing_int);
	} else {
		snprintf(buf, sizeof(buf), "%d", item->value_int);
	}

	if (data->precision) {
		s = buf + strlen(buf) + 1;
		for (i = data->precision; i >= 0; --i) {
			*s = *(s - 1);
			s--;
		}
		*s = '.';
	}

	len = strlen(buf);
	s = buf + len;
	*s++ = ' ';
	len++;
	strncpy_P(s, data->unit, sizeof(buf) - len);

	if (item->editing) {
		drv->print(drv, drv->char_x - strlen(buf), line, buf, MS_INVERTED);
	} else {
		drv->print(drv, drv->char_x - strlen(buf), line, buf, MS_NORMAL);
	}
}

static void display_exit(struct menu *menu, struct menu_node *item,
                         struct menu_node_shadow *data, int line)
{
	static const char exit[] PROGMEM = "\x1B" "EXIT";
	struct menu_display_driver *drv = menu->driver;
	strncpy_P(buf, exit, sizeof(buf));
	drv->print(drv, 1, line, buf, MS_NORMAL);
}

static void copy_shadow(const struct menu_node *node) {
	static const struct menu_node *cached;
	if (node != cached) {
		memcpy_P(&shadow, node->shadow, sizeof(shadow));
		cached = node;
	}
}

static void copy_shadow_items(const void *src, size_t len) {
	const void *cached;
	if (src != cached) {
		memcpy_P(shadow_items, src, sizeof(shadow_items[0]) * len);
		cached = src;
	}
}

void menu_update(struct menu *menu) {
	struct menu_display_driver *drv;
	struct menu_node *item;
	int line = 0;
	int cursor;
	int items_len;
	int i;

	if (!menu) {
		return;
	}

	if (!menu->active) {
		menu->active = menu->root;
	}

	drv = menu->driver;
	if (!drv) {
		return;
	}

	copy_shadow(menu->active);
	copy_shadow_items(shadow.items, shadow.items_len);
	items_len = shadow.items_len;

	drv->clear(drv);

	// Print menu title
	strncpy_P(buf, shadow.label, sizeof(buf));
	drv->print(drv, drv->char_x / 2 - strlen(buf) / 2,
	           0, buf, MS_LINE_INVERTED
	);

	// Print menu items
	for (i = 0; i < items_len; ++i) {
		line = i + 1; // Offset by one to account for title
		if (menu->active->cursor_idx == i) {
			drv->print(drv, 0, line, ">", MS_NORMAL);
		}
		item = shadow_items[i];
		copy_shadow(item);
		switch (shadow.type) {
		case MT_SUBMENU:
			display_submenu(menu, item, &shadow, line);
			break;
		case MT_FIELD:
			display_field(menu, item, &shadow, line);
			break;
		case MT_EXIT:
			display_exit(menu, item, &shadow, line);
			break;
		}
	}

	drv->display(drv);
}

static struct menu_node *node_parent(struct menu *menu, struct menu_node *node) {
	struct menu_node *p, **to_visit;
	int to_visit_cap, to_visit_len;
	int i, need_cap;

	to_visit_cap = 4;
	to_visit = (struct menu_node **)malloc(sizeof(*to_visit) * to_visit_cap);
	to_visit_len = 0;

	p = menu->root;
	while (p) {
		copy_shadow(p);
		for (i = 0; i < shadow.items_len; ++i) {
			if (shadow_items[i] == node) {
				goto out;
			}
		}

		// node not found in p's items, keep looking lower in the graph
		need_cap = to_visit_cap - to_visit_len + shadow.items_len;
		if (need_cap > to_visit_cap) {
			to_visit = (struct menu_node **)realloc(to_visit, sizeof(*to_visit) * need_cap);
			to_visit_cap = need_cap;
		}
		memcpy(to_visit + to_visit_len, shadow.items, shadow.items_len);
		to_visit_len += shadow.items_len;

		if (--to_visit_len > 0) {
			p = to_visit[to_visit_len];
		}
		else {
			p = NULL;
		}
	}

out:
	free(to_visit);
	return p;
}


static bool handle_submenu_input(struct menu *menu, struct menu_node *node, struct menu_node_shadow *data, int input) {
	if (input == MI_OK) {
		menu->active = node;
		return true;
	}
	return false;
}

static bool handle_field_input(struct menu *menu, struct menu_node *node, struct menu_node_shadow *data, int input) {
	if (!node->editing) {
		if (input == MI_OK) {
			menu->editing_int = node->value_int;
			node->editing = true;
			return true;
		}
		return false;
	}

	switch (input) {
	case MI_ESC:
		node->editing = false;
		return true;
	case MI_UP:
		menu->editing_int++;
		return true;
	case MI_DOWN:
		menu->editing_int--;
		return true;
	case MI_OK:
		node->value_int = menu->editing_int;
		node->editing = false;
		return true;
	}
	return false;
}

static bool handle_exit_input(struct menu *menu, struct menu_node *node, struct menu_node_shadow *data, int input) {
	if (input == MI_OK) {
		menu->active = node_parent(menu, node);
		if (menu->active == NULL) {
			menu->active = menu->root;
		}
		return true;
	}
	return false;
}

// Returns true if the input was handled
typedef bool (*input_handler)(struct menu *menu, struct menu_node *node, struct menu_node_shadow *data, int input);
static input_handler input_handlers[] = {
	[MT_SUBMENU]   = handle_submenu_input,
	[MT_FIELD]     = handle_field_input,
	[MT_OPERATION] = NULL,
	[MT_EXIT]      = handle_exit_input,
};

void menu_input(struct menu *menu, int input) {
	struct menu_node *active, *selected;
	int items_len;
	bool handled;

	if (!menu) {
		return;
	}

	if (!menu->active) {
		menu->active = menu->root;
	}
	active = menu->active;

	copy_shadow(menu->active);
	copy_shadow_items(shadow.items, shadow.items_len);
	selected = shadow_items[active->cursor_idx];
	items_len = shadow.items_len;
	copy_shadow(selected);

	handled = false;

	if (input_handlers[shadow.type]){
		handled = input_handlers[shadow.type](menu, selected, &shadow, input);
	}

	if (!handled) {
		// Node didn't handle the input, fall back to navigation
		switch(input) {
		case MI_UP:
			active->cursor_idx--;
			active->cursor_idx %= items_len;
			break;
		case MI_DOWN:
			active->cursor_idx++;
			active->cursor_idx %= items_len;
			break;
		case MI_ESC:
			menu->active = node_parent(menu, active);
			if (menu->active == NULL) {
				menu->active = menu->root;
			}
		}
	}
}