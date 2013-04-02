
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

/*
   cobaye_scan_hex
   returns:  -1 if aborted
              0 if no values were received
              number of digits if a value was received
*/
#define HEX_STRING_LEN  9
int cobaye_scan_hex(int *value)
{
	char received[HEX_STRING_LEN];
	char *start = received;
	char *end;
	int valid = 1;
	char pressed;
	int position = 0;
	int retval;
	int base = 16;

	received[HEX_STRING_LEN - 1] = ' ';
	while (valid) {
		cobaye_scanf("%c", &pressed);
		switch (pressed) {
		case '0' ... '9':
		case 'a' ... 'f':
		case 'A' ... 'F':
			if (position < 8) {
				received[position] = pressed;
				position++;
			} else {
				cobaye_printf("Only 8 digits are allowed\n");
			}
			break;
		case 0x08:	/* backspace */
			if (position > 0)
				position--;
			break;
		case 0x0d:	/* enter */
			*value = strtol(start, &end, base);
			if (start == end) {
				retval = 0;
			} else {
				retval = position;
			}
			valid = 0;
			break;
		case 'q':
		case 'Q':
			cobaye_printf("\n");
			retval = -1;
			valid = 0;
			break;
		default:
			cobaye_printf
			    ("invalid character received, aborting!\n");
			valid = 0;
			retval = -1;
			break;
		}
	}
	return (retval);
}
