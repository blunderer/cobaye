#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>

#include "cobaye_framework.h"

static struct cobaye_report report;

static int cobaye_report_txt(int type, char *name, int iter, char *str, int id)
{
	char date[18];
	time_t now = time(NULL);
	struct tm *now_time = localtime(&now);

	sprintf(date, "%04d-%02d-%02d %02d:%02d:%02d",
			now_time->tm_year + 1900, now_time->tm_mon + 1,
			now_time->tm_mday, now_time->tm_hour, now_time->tm_min,
			now_time->tm_sec);

	switch (type) {
	case TST_OPEN:
		break;
	case TST_START:
		break;
	case TST_RUN:
		fprintf(report.stream, "#\n");
		fprintf(report.stream, "# Running %s #%d\n", name, iter);
		fprintf(report.stream, "#\n");
		break;
	case TST_SKIP:
		fprintf(report.stream, "# SKIP\n\n");
		break;
	case TST_ERROR:
		fprintf(report.stream, "# ERROR\n\n");
		break;
	case TST_PASS:
		fprintf(report.stream, "# PASS\n\n");
		break;
	case TST_FAIL:
		fprintf(report.stream, "# FAIL (error %d)\n\n", id);
		break;
	case TST_STRING:
		if (id == 1 || id == 2) {
			fprintf(report.stream, ">> %s", str);
			if (*(str + strlen(str) - 1) != '\n')
				fprintf(report.stream, "\n");
		} else if (id == 0) {
			fprintf(report.stream, "<< %s", str);
		}
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
	.report = cobaye_report_txt,
	.ext = "txt"
}; 

cobaye_declare_report(report);
