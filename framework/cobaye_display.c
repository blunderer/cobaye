
#include "cobaye_ncurses.h"

void cobaye_display_help(char *txt)
{
	wclear(helpwin);
	waddstr(helpwin, "Help:\n");
	waddstr(helpwin, txt);
	wrefresh(helpwin);
}

void cobaye_display_cobaye_status(int id, char *txt)
{
	id = (id - 1) % 4;
	if (txt[0] == 0x1E) {
		wclrtoeol(teststatuswin);
		mvwprintw(teststatuswin, id, 0, " %d: ", id + 1);
	} else {
		wprintw(teststatuswin, txt);
	}
	wrefresh(teststatuswin);
}

void cobaye_display_cobaye_vstatus(int id, char *txt, va_list list)
{
	if (cobaye_ncurses_enabled()) {
		id = (id - 1) % 4;
		wclrtoeol(teststatuswin);
		mvwprintw(teststatuswin, id, 0, " %d: ", id + 1);
		vwprintw(teststatuswin, txt, list);
		wrefresh(teststatuswin);
	}
}

void cobaye_display_status(char *txt, ...)
{
	if (cobaye_ncurses_enabled()) {
		va_list list;
		va_start(list, txt);
		if (cobaye_ncurses_enabled()) {
			wclear(statuswin);
			waddstr(statuswin, " status: ");
			vwprintw(statuswin, txt, list);
			wrefresh(statuswin);
		} else {
			vprintf(txt, list);
		}
		va_end(list);
	}
}
