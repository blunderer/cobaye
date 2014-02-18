#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>

#include "cobaye_framework.h"

static struct cobaye_report report;

static int cobaye_report_html(int type, char *name, int iter, char *str, int id)
{
	char date[18];
	FILE *css = NULL;
	struct stat buf;
	time_t now = time(NULL);
	struct tm *now_time = localtime(&now);

	sprintf(date, "%04d-%02d-%02d %02d:%02d:%02d",
			now_time->tm_year + 1900, now_time->tm_mon + 1,
			now_time->tm_mday, now_time->tm_hour, now_time->tm_min,
			now_time->tm_sec);

	switch(type) {
	case TST_OPEN:
		fprintf(report.stream, "<html>\n");
		fprintf(report.stream, "<head>\n");
		fprintf(report.stream, "<title>Tests %s</title>\n", date);
		fprintf(report.stream, "<link rel='stylesheet' type='text/css' href='report.css'/>");
		fprintf(report.stream, "</head>\n");
		fprintf(report.stream, "<body>\n");
		fprintf(report.stream, "<h1>Test results</h1>\n");
		fprintf(report.stream, "Date %s<br/>\n", date);
		fprintf(report.stream, "Host %s<br/>\n", name);

		do {
			struct stat buf;
			char csspath[512];
			sprintf(csspath, "%s/report.css", report.path);
			if(stat(csspath, &buf) == -1) {
				FILE *css = fopen(csspath, "w");
				if(css) {
					fprintf(css, ".test-result {\n");
					fprintf(css, "  margin: 10px;\n");
					fprintf(css, "  padding: 10px;\n");
					fprintf(css, "  border: 1px solid black;\n");
					fprintf(css, "  border-collapse: collapse;\n");
					fprintf(css, "}\n");
					fprintf(css, ".table-header {\n");
					fprintf(css, "  font-weight: bold;\n");
					fprintf(css, "}\n");
					fprintf(css, ".skip {\n");
					fprintf(css, "  background-color: white;\n");
					fprintf(css, "}\n");
					fprintf(css, ".pass {\n");
					fprintf(css, "  background-color: green;\n");
					fprintf(css, "}\n");
					fprintf(css, ".fail {\n");
					fprintf(css, "  background-color: red;\n");
					fprintf(css, "}\n");
					fprintf(css, ".error {\n");
					fprintf(css, "  background-color: yellow;\n");
					fprintf(css, "}\n");
					fclose(css);
				}
			}
		} while (0);
		break;
	case TST_START:
		fprintf(report.stream, "<h2>%s</h2>\n", name);
		fprintf(report.stream, "<table class='test-result'>\n");
		fprintf(report.stream, "<tr class='table-header'>\n");
		fprintf(report.stream, "<td class='test-result table-header'>Iteration</td>\n");
		fprintf(report.stream, "<td class='test-result table-header'>Date</td>\n");
		fprintf(report.stream, "<td class='test-result table-header'>Log</td>\n");
		fprintf(report.stream, "<td class='test-result table-header'>Result</td>\n");
		fprintf(report.stream, "<td class='test-result table-header'>ErrorCode</td>\n");
		fprintf(report.stream, "</tr>\n");
		break;
	case TST_RUN:
		fprintf(report.stream, "<tr class='test-result'>\n");
		fprintf(report.stream, "<td class='test-result'>%d</td>\n", iter);
		fprintf(report.stream, "<td class='test-result'>%s</td>\n", date);
		fprintf(report.stream, "<td class='test-result'>\n");
		break;
	case TST_SKIP:
		fprintf(report.stream, "<tr class='test-result'>\n");
		fprintf(report.stream, "<td class='test-result'>%d</td>\n", iter);
		fprintf(report.stream, "<td class='test-result'>%s</td>\n", date);
		fprintf(report.stream, "<td class='test-result'>\n");
		fprintf(report.stream, "</td>\n");
		fprintf(report.stream, "<td class='test-result skip'>SKIPPED</td>\n");
		fprintf(report.stream, "<td class='test-result'></td>\n");
		fprintf(report.stream, "</tr>\n");
		break;
	case TST_ERROR:
		fprintf(report.stream, "</td>\n");
		fprintf(report.stream, "<td class='test-result error'>ERROR</td>\n");
		fprintf(report.stream, "<td class='test-result'>%d</td>\n", id);
		fprintf(report.stream, "</tr>\n");
		break;
	case TST_PASS:
		fprintf(report.stream, "</td>\n");
		fprintf(report.stream, "<td class='test-result pass'>PASS</td>\n");
		fprintf(report.stream, "<td class='test-result'></td>\n");
		fprintf(report.stream, "</tr>\n");
		break;
	case TST_FAIL:
		fprintf(report.stream, "</td>\n");
		fprintf(report.stream, "<td class='test-result fail'>FAIL</td>\n");
		fprintf(report.stream, "<td class='test-result'>%d</td>\n", id);
		fprintf(report.stream, "</tr>\n");
		break;
	case TST_STRING:
		if (id == 1 || id == 2)
			fprintf(report.stream, "&gt;&gt; %s</br>", str);
		else if (id == 0)
			fprintf(report.stream, "&lt;&lt; %s</br>", str);
		break;
	case TST_STOP:
		fprintf(report.stream, "</table>\n");
		break;
	case TST_CLOSE:
		fprintf(report.stream, "</body>\n");
		fprintf(report.stream, "</html>\n");
		break;
	default:
		break;
	}
	fflush(report.stream);

	return 0;
}

static struct cobaye_report report = {
	.report = cobaye_report_html,
	.ext = "html"
}; 

cobaye_declare_report(report);
