
#ifndef MENU_CORE_H
#define MENU_CORE_H

#include "cobaye_ncurses.h"

enum menutype {
	menutype_disabled = 0,
	menutype_func,
	menutype_bool,
	menutype_int,
	menutype_char,
	menutype_title,
};

struct menu {
	enum menutype type;
	char name[32];
	char help[128];
	int (*func) (struct menu *);

	int value;
	char string[512];
	struct cobaye_test *test;
};

int cobaye_display_menu(WINDOW * win, struct menu *menu, int mn_items);
int cobaye_display_menu_title(WINDOW * win, char *name);
int cobaye_display_menu_entry(WINDOW * win, struct menu *menu, int id);

int cobaye_menu_quit(struct menu *entry);
int cobaye_menu_configure(struct menu *entry);
int cobaye_menu_prompt(struct menu *entry);

int cobaye_menu_quit(struct menu *entry);

#endif /* MENU_CORE_H */
