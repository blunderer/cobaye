
#include "cobaye_ncurses.h"

#define MENU_UP		259
#define MENU_DOWN	258
#define MENU_ENTER	10
#define MENU_SPACE	32
#define MENU_ESCAPE	27
#define MENU_MAJQ	81
#define MENU_Q		113

/* Menu utils:
	- generic call backs
	- generic display funcs
*/

int cobaye_menu_prompt(struct menu *entry)
{
	char data[512];
	int x, y;
	WINDOW *popupborder = NULL;
	WINDOW *popup = NULL;

	cobaye_display_help
	    ("Enter the path of the seq file to load and validate with enter.");

	getmaxyx(mainwin, y, x);
	popupborder = newwin(8, 52, (y - 8) / 2, (x - 52) / 2);
	popup = newwin(6, 50, (y - 6) / 2, (x - 50) / 2);
	wbkgd(popup, COLOR_PAIR(2));

	mvwaddstr(popupborder, 0, 19, "LOAD SEQUENCE");
	mvwaddstr(popup, 1, 1, "Enter the path to the seq file to load.");

	wrefresh(popupborder);
	wrefresh(popup);

	memset(data, 0, 512);
	memset(data, ' ', 46);
	wattron(popup, COLOR_PAIR(4));
	mvwaddstr(popup, 3, 2, data);

	mvwgetstr(popup, 3, 2, data);
	if (entry->type == menutype_int) {
		entry->value = atoi(data);
	} else {
		strncpy(entry->string, data, sizeof(entry->string));
	}

	delwin(popupborder);
	delwin(popup);
	touchwin(mainwin);
	wrefresh(mainwin);

	return 0;
}

int cobaye_menu_quit(struct menu *entry)
{
	return 1;
}

/* Menu handling functions */
int cobaye_menu_redraw(WINDOW * win, struct menu *menu, int count, int offset)
{
	int i = 0;

	wclear(win);

	cobaye_display_menu_title(win, menu->name);
	for (i = offset; i < count; i++) {
		cobaye_display_menu_entry(win, &menu[i], i - offset);
	}

	return 0;
}

int cobaye_display_menu(WINDOW * win, struct menu *menu, int nb_items)
{
	int x, y;
	int index = 1;
	int ret = 0;
	int offset = 0;
	int offset_range = 0;

	wattron(win, COLOR_PAIR(1));
	cobaye_display_help(menu[index].help);
	cobaye_display_menu_entry(win, &menu[index], index);
	wattroff(win, COLOR_PAIR(1));

	getmaxyx(win, y, x);
	if (nb_items > (y - 4)) {
		offset_range = nb_items - (y - 4);
	}
	wrefresh(win);

	while (ret == 0) {
		int index_base = index;
		int pressed = getch();

		cobaye_display_menu_entry(win, &menu[index], index - offset);
		wrefresh(win);

		switch (pressed) {
		case 'Q':
		case 'q':
		case KEY_LEFT:
			ret = 1;
			break;
		case '\n':
		case ' ':
		case KEY_RIGHT:
			if (menu[index].type == menutype_func) {
				if (menu[index].func) {
					ret = menu[index].func(&menu[index]);
					cobaye_menu_redraw(win, menu, nb_items,
							   offset);
				}
			} else if (menu[index].type == menutype_int) {
				cobaye_menu_prompt(&menu[index]);
			} else if (menu[index].type == menutype_char) {
				cobaye_menu_prompt(&menu[index]);
			} else if (menu[index].type == menutype_bool) {
				menu[index].value = (menu[index].value + 1) % 2;
			}
			break;
		case KEY_PPAGE:
			index -= y - 4;
			while (index > 0 && ((menu[index].name[0] == 0) ||
					     (menu[index].type ==
					      menutype_disabled)))
				index--;
			if (index < 1)
				index = nb_items - 1;
			break;
		case KEY_NPAGE:
			index += y - 4;
			while (index < nb_items
			       && ((menu[index].name[0] == 0)
				   || (menu[index].type == menutype_disabled)))
				index++;
			if (index > nb_items - 1)
				index = 1;
			break;
		case KEY_UP:
			index--;
			while (index > 0 && ((menu[index].name[0] == 0) ||
					     (menu[index].type ==
					      menutype_disabled)))
				index--;
			if (index < 1)
				index = nb_items - 1;
			break;
		case KEY_DOWN:
			index++;
			while (index < nb_items
			       && ((menu[index].name[0] == 0)
				   || (menu[index].type == menutype_disabled)))
				index++;
			if (index > nb_items - 1)
				index = 1;
			break;
		default:
			break;
		}

		if (ret == 0) {
			if (index - index_base > 0) {
				if (index + 3 >= y + offset) {
					offset += index - index_base;
				}
			} else if (index - index_base < 0) {
				if (index < offset) {
					offset += index - index_base;
				}
			}
			if (offset < 0) {
				offset = 0;
			}
			if (offset > offset_range) {
				offset = offset_range;
			}
			wscrl(win, index - index_base);
			cobaye_menu_redraw(win, menu, nb_items, offset);

			wattron(win, COLOR_PAIR(1));
			cobaye_display_help(menu[index].help);
			cobaye_display_menu_entry(win, &menu[index],
						  index - offset);
			wattroff(win, COLOR_PAIR(1));
			wrefresh(win);
		}
	}
	return 0;
}

int cobaye_display_menu_entry(WINDOW * win, struct menu *menu, int id)
{
	int err = 0;

	wclrtoeol(win);

	/* display title */
	switch (menu->type) {
	case menutype_title:
		cobaye_display_menu_title(win, menu->name);
		break;
	case menutype_disabled:
		if (menu->name[0])
			mvwprintw(win, 3 + id, 1, "  --  %s", menu->name);
		break;
	case menutype_func:
		mvwprintw(win, 3 + id, 1, "  ->  %s", menu->name);
		break;
	case menutype_int:
		mvwprintw(win, 3 + id, 1, "[%3d] %s", menu->value, menu->name);
		break;
	case menutype_char:
		mvwprintw(win, 3 + id, 1, "[...] %s (%.32s)", menu->name,
			  menu->string);
		wclrtoeol(win);
		break;
	case menutype_bool:
		mvwprintw(win, 3 + id, 1, "[ %c ] %s",
			  (menu->value == 1) ? '*' : ' ', menu->name);
		break;
	default:
		err = -1;
		break;
	}

	return err;
}

int cobaye_display_menu_title(WINDOW * win, char *name)
{
	int x, center;

	x = getmaxx(win);
	center = (x - strlen(name)) / 2;

	/* display title */
	wclear(win);
	wattron(win, A_BOLD | A_UNDERLINE);
	mvwprintw(win, 1, center, name);
	wattroff(win, A_BOLD | A_UNDERLINE);

	return 0;
}
