#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>

#include "cobaye_framework.h"

static struct cobaye_report report;

static int cobaye_report_xml(int type, char *name, int iter, char *str, int id)
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
		fprintf(report.stream, "<?xml version='1.0' encoding='UTF-8'?>\n");
		fprintf(report.stream, "<tests>\n");
		break;
	case TST_START:
		fprintf(report.stream, " <test name='%s' date='%s'>\n", name, date);
		break;
	case TST_RUN:
		fprintf(report.stream, "  <iteration id='%d'>\n", iter);
		fprintf(report.stream, "   <log><![CDATA[\n");
		break;
	case TST_SKIP:
		fprintf(report.stream, "   ]]></log>\n");
		fprintf(report.stream, "   <result><skipped/></result>\n");
		fprintf(report.stream, "  </iteration>\n");
		break;
	case TST_ERROR:
		fprintf(report.stream, "   ]]></log>\n");
		fprintf(report.stream, "   <result><error code='%d'/></result>\n", id);
		fprintf(report.stream, "  </iteration>\n");
		break;
	case TST_PASS:
		fprintf(report.stream, "   ]]></log>\n");
		fprintf(report.stream, "   <result><passed/></result>\n");
		fprintf(report.stream, "  </iteration>\n");
		break;
	case TST_FAIL:
		fprintf(report.stream, "   ]]></log>\n");
		fprintf(report.stream, "   <result><failed code='%d'/></result>\n", id);
		fprintf(report.stream, "  </iteration>\n");
		break;
	case TST_STRING:
		if (id == 1 || id == 2) {
			fprintf(report.stream, ">> %s", str);
			if (*(str + strlen(str) -1) != '\n')
				fprintf(report.stream, "\n");
		} else if (id == 0) {
			fprintf(report.stream, "<< %s", str);
		}
		break;
	case TST_STOP:
		fprintf(report.stream, " </test>\n");
		break;
	case TST_CLOSE:
		fprintf(report.stream, "</tests>\n");
		break;
	default:
		break;
	}
	fflush(report.stream);

	return 0;
}

static struct cobaye_report report = {
	.report = cobaye_report_xml,
	.ext = "xml"
}; 

cobaye_declare_report(report);
