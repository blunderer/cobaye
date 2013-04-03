
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>

#include "cobaye_ncurses.h"

static pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

void cobaye_stdout_get_unsafe(void)
{
	putchar(0x02);
	putchar('0');
	fflush(stdout);
}

void cobaye_stdout_put_unsafe(void)
{
	putchar(0x17);
	fflush(stdout);
}

int cobaye_printf(char *fmt, ...)
{
	int ret = 0;
	va_list list;
	va_start(list, fmt);

	pthread_mutex_lock(&print_lock);
	if (cobaye_forked()) {
		putchar(0x02);
		putchar('0');
		ret = vfprintf(stdout, fmt, list);
		putchar(0x17);
		fflush(stdout);
	} else {
		vwprintw(ctxwin, fmt, list);
		wrefresh(ctxwin);
	}
	pthread_mutex_unlock(&print_lock);

	va_end(list);

	return ret;
}

int cobaye_status(int id, char *fmt, ...)
{
	int ret = 0;
	va_list list;
	va_start(list, fmt);

	if (id > 0) {
		pthread_mutex_lock(&print_lock);
		if (cobaye_forked()) {
			putchar(0x02);
			putchar('0' + id);
			ret = vfprintf(stdout, fmt, list);
			putchar(0x17);
			fflush(stdout);
		} else {
			cobaye_display_cobaye_vstatus(id, fmt, list);
		}
		pthread_mutex_unlock(&print_lock);
	}

	va_end(list);
	return ret;
}

int cobaye_scanf(char *fmt, ...)
{
	int ret = 0;
	va_list list;
	va_start(list, fmt);

	pthread_mutex_lock(&print_lock);
	if (cobaye_forked()) {
		ret = vscanf(fmt, list);
	} else {
		ret = vwscanw(ctxwin, fmt, list);
	}
	pthread_mutex_unlock(&print_lock);
	va_end(list);
	return ret;
}

