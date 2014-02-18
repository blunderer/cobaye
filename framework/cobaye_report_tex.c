#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>

#include "cobaye_framework.h"

static struct cobaye_report report;

static int cobaye_report_tex(int type, char *name, int iter, char *str, int id)
{
	char date[18];
	time_t now = time(NULL);
	struct tm *now_time = localtime(&now);

	sprintf(date, "%04d-%02d-%02d %02d:%02d:%02d",
			now_time->tm_year + 1900, now_time->tm_mon + 1,
			now_time->tm_mday, now_time->tm_hour, now_time->tm_min,
			now_time->tm_sec);

	switch(type) {
	case TST_OPEN:
		break;
	case TST_START:
		break;
	case TST_RUN:
		break;
	case TST_SKIP:
		break;
	case TST_ERROR:
		break;
	case TST_PASS:
		break;
	case TST_FAIL:
		break;
	case TST_STRING:
		break;
	case TST_STOP:
		break;
	case TST_CLOSE:
		break;
	default:
		break;
	}
	fflush(report.stream);

	return 0;
}

static struct cobaye_report report = {
	.report = cobaye_report_tex,
	.ext = "tex"
}; 

cobaye_declare_report(report);
