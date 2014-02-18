#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>

#include "cobaye_framework.h"

static pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t scan_lock = PTHREAD_MUTEX_INITIALIZER;

void cobaye_stdout_get_unsafe(void)
{
	fflush(stdout);
}

void cobaye_stdout_put_unsafe(void)
{
	fflush(stdout);
}

int cobaye_printf(char *fmt, ...)
{
	int ret = 0;
	va_list list;

	va_start(list, fmt);
	pthread_mutex_lock(&print_lock);
	ret = vfprintf(stdout, fmt, list);
	fflush(stdout);
	pthread_mutex_unlock(&print_lock);
	va_end(list);

	return ret;
}

int cobaye_scanf(char *fmt, ...)
{
	int ret = 0;
	va_list list;

	va_start(list, fmt);
	pthread_mutex_lock(&scan_lock);
	ret = vscanf(fmt, list);
	pthread_mutex_unlock(&scan_lock);
	va_end(list);

	return ret;
}

