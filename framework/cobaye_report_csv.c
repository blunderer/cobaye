#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>

#include "cobaye_framework.h"

static struct cobaye_report report;

static int cobaye_report_csv(int type, char *name, int iter, char *str, int id)
{
	char date[20];
	time_t now = time(NULL);
	struct tm *now_time = localtime(&now);

	sprintf(date, "%04d-%02d-%02d %02d:%02d:%02d",
			now_time->tm_year + 1900, now_time->tm_mon + 1,
			now_time->tm_mday, now_time->tm_hour, now_time->tm_min,
			now_time->tm_sec);

	switch(type) {
	case TST_OPEN:
		fprintf(report.stream, "date,name,iter,log/result\n");
		break;
	case TST_SKIP:
		fprintf(report.stream, "%s,%s,%d,Skipped\n", date, name, iter);
		break;
	case TST_ERROR:
		fprintf(report.stream, "%s,%s,%d,Error %d\n", date, name, iter, id);
		break;
	case TST_PASS:
		fprintf(report.stream, "%s,%s,%d,Success\n", date, name, iter);
		break;
	case TST_FAIL:
		fprintf(report.stream, "%s,%s,%d,Failed with code %d\n", date, name, iter, id);
		break;
	case TST_STRING:
		if (id == 1 || id == 2) {
			fprintf(report.stream, "%s,%s,%d,>>%s", date, name, iter, str);
			if (*(str + strlen(str) -1) != '\n')
				fprintf(report.stream, "\n");
		} else if (id == 0) {
			fprintf(report.stream, "%s,%s,%d,<<%s", date, name, iter, str);
		}
		break;
	default:
		break;
	}

	return 0;
}

static struct cobaye_report report = {
	.report = cobaye_report_csv,
	.ext = "csv"
};

cobaye_declare_report(report);
