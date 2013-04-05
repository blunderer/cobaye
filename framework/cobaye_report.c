
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#include "cobaye_ncurses.h"
#include "cobaye_menus.h"
#include "cobaye_report.h"

static char reportpath[512] = "";
static FILE *tst_output = NULL;
static FILE *txt_report = NULL;
static FILE *csv_report = NULL;
static FILE *tex_report = NULL;
static FILE *xml_report = NULL;
static FILE *html_report = NULL;

/* tests statistics */
static struct timeval stat_start_tests;
static struct timeval stat_stop_tests;
static struct timeval stat_start_test;
static struct timeval stat_stop_test;

static int stat_nb_pass = 0;
static int stat_nb_fail = 0;
static int stat_nb_test = 0;

static int cobaye_report_status(int type, char *name, int iter, char *str, int id)
{
	switch(type) {
	case TST_RUN:
		cobaye_display_status("%s Starting", name);
		break;
	case TST_ERROR:
		cobaye_display_status("%s Error %d", name, id);
		break;
	case TST_PASS:
		cobaye_display_status("%s Success", name);
		break;
	case TST_FAIL:
		cobaye_display_status("%s Failed", name, id);
		break;
	default:
		break;
	}
	return 0;
}

static int cobaye_report_stdout(int type, char *name, int iter, char *str, int id)
{
	switch(type) {
	case TST_RUN:
		printf("  %s(%d): Running\n", name, iter);
		break;
	case TST_ERROR:
		printf("  %s(%d): Error %d: %s\n", name, iter, id, strerror(id));
		break;
	case TST_PASS:
		printf("  %s(%d): Success\n", name, iter);
		break;
	case TST_SKIP:
		printf("  %s(%d): Skip\n", name, iter);
		break;
	case TST_FAIL:
		printf("  %s(%d): Fail %d\n", name, iter, id);
		break;
	case TST_STRING:
		if(id == 0)
			printf("%c", str[0]);
		break;
	default:
		break;
	}
	fflush(stdout);
	return 0;
}

static int cobaye_report_ui(int type, char *name, int iter, char *str, int id)
{
	switch(type) {
	case TST_RUN:
		wattron(ctxwin, A_BOLD | COLOR_PAIR(3));
		wprintw(ctxwin, "\n\n");
		wprintw(ctxwin, "  %s(%d): Running\n", name, iter);
		wattroff(ctxwin, A_BOLD | COLOR_PAIR(3));
		break;
	case TST_ERROR:
		wattron(ctxwin, A_BOLD | COLOR_PAIR(3));
		wprintw(ctxwin, "  %s(%d): Error %d: %s\n", name, iter, id, strerror(id));
		wattroff(ctxwin, A_BOLD | COLOR_PAIR(3));
		break;
	case TST_PASS:
		wattron(ctxwin, A_BOLD | COLOR_PAIR(3));
		wprintw(ctxwin, "  %s(%d): Success\n", name, iter);
		wattroff(ctxwin, A_BOLD | COLOR_PAIR(3));
		break;
	case TST_FAIL:
		wattron(ctxwin, A_BOLD | COLOR_PAIR(3));
		wprintw(ctxwin, "  %s(%d): Fail %d\n", name, iter, id);
		wattroff(ctxwin, A_BOLD | COLOR_PAIR(3));
		break;
	case TST_STRING:
		wattron(ctxwin, A_BOLD);
		if(id == 0)
			waddstr(ctxwin, str);
		else
			cobaye_display_cobaye_status(id, str);
		wattroff(ctxwin, A_BOLD);
		break;
	case TST_STOP:
		wattron(ctxwin, A_BOLD | COLOR_PAIR(3));
		wprintw(ctxwin, "\n\n");
		wprintw(ctxwin, "  Press enter to continue\n");
		wattroff(ctxwin, A_BOLD | COLOR_PAIR(3));
		break;
	default:
		break;
	}
	wrefresh(ctxwin);
	return 0;
}

static int cobaye_report_set_test_ouput(char *name)
{
	char filepath[512] = "";

	sprintf(filepath, "%s/%s.txt", reportpath, name);

	if (tst_output) {
		fclose(tst_output);
	}
	tst_output = fopen(filepath, "w");
	if (!tst_output) {
		return -1;
	}

	return 0;
}

static int cobaye_report_txt(int type, char *name, int iter, char *str, int id)
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
		cobaye_report_set_test_ouput(name);
		break;
	case TST_SKIP:
		fprintf(tst_output, "  %s(%d): Skip\n", name, iter);
		fprintf(txt_report, "SKIP\t\t%s(%d)\n", name, iter);
		break;
	case TST_ERROR:
		fprintf(tst_output, "  %s(%d): Error %d: %s\n", name, iter, id, strerror(id));
		fprintf(txt_report, "ERROR\t\t%s(%d)\n", name, iter);
		break;
	case TST_PASS:
		fprintf(tst_output, "  %s(%d): Success\n", name, iter);
		fprintf(txt_report, "PASS\t\t%s(%d)\n", name, iter);
		break;
	case TST_FAIL:
		fprintf(tst_output, "  %s(%d): Fail %d\n", name, iter, id);
		fprintf(txt_report, "FAIL\t\t%s(%d)\n", name, iter);
		break;
	case TST_STRING:
		if(id == 0)
			fprintf(tst_output, "%c", str[0]);
		break;
	case TST_STOP:
		break;
	case TST_CLOSE:
		break;
	default:
		break;
	}
	fflush(txt_report);
	fflush(tst_output);

	return 0;
}

static int cobaye_report_csv(int type, char *name, int iter, char *str, int id)
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
		fprintf(csv_report, "date,name,iter,result\n");
		break;
	case TST_SKIP:
		fprintf(csv_report, "%s,%s,%d,Skipped\n", date, name, iter);
		break;
	case TST_ERROR:
		fprintf(csv_report, "%s,%s,%d,Error %d\n", date, name, iter, id);
		break;
	case TST_PASS:
		fprintf(csv_report, "%s,%s,%d,Success\n", date, name, iter);
		break;
	case TST_FAIL:
		fprintf(csv_report, "%s,%s,%d,Failed with code %d\n", date, name, iter, id);
		break;
	default:
		break;
	}
	fflush(csv_report);

	return 0;
}

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
	fflush(tex_report);

	return 0;
}

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
		fprintf(xml_report, "<?xml version='1.0' encoding='UTF-8'?>\n");
		fprintf(xml_report, "<tests>\n");
		break;
	case TST_START:
		fprintf(xml_report, "<test name='%s' date='%s'>\n", name, date);
		break;
	case TST_RUN:
		fprintf(xml_report, "<iteration id='%d'>\n", iter);
		fprintf(xml_report, "<log>\n");
		break;
	case TST_SKIP:
		fprintf(xml_report, "</log>\n");
		fprintf(xml_report, "<result><skipped/></result>\n");
		fprintf(xml_report, "</iteration>\n");
		break;
	case TST_ERROR:
		fprintf(xml_report, "</log>\n");
		fprintf(xml_report, "<result><error code='%d'/></result>\n", id);
		fprintf(xml_report, "</iteration>\n");
		break;
	case TST_PASS:
		fprintf(xml_report, "</log>\n");
		fprintf(xml_report, "<result><passed/></result>\n");
		fprintf(xml_report, "</iteration>\n");
		break;
	case TST_FAIL:
		fprintf(xml_report, "</log>\n");
		fprintf(xml_report, "<result><failed code='%d'/></result>\n", id);
		fprintf(xml_report, "</iteration>\n");
		break;
	case TST_STRING:
		fprintf(xml_report, "%s", str);
		break;
	case TST_STOP:
		fprintf(xml_report, "</test>\n");
		break;
	case TST_CLOSE:
		fprintf(xml_report, "</tests>\n");
		break;
	default:
		break;
	}
	fflush(xml_report);

	return 0;
}

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
		fprintf(html_report, "<html>\n");
		fprintf(html_report, "<head>\n");
		fprintf(html_report, "<title>Tests %s</title>\n", date);
		fprintf(html_report, "<link rel='stylesheet' type='text/css' href='report.css'/>");
		fprintf(html_report, "</head>\n");
		fprintf(html_report, "<body>\n");
		fprintf(html_report, "<h1>Test results</h1>\n");
		fprintf(html_report, "Date %s<br/>\n", date);
		fprintf(html_report, "Host %s<br/>\n", name);

		do {
			struct stat buf;
			char csspath[512];
			sprintf(csspath, "%s/report.css", reportpath);
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
		fprintf(html_report, "<h2>%s</h2>\n", name);
		fprintf(html_report, "<table class='test-result'>\n");
		fprintf(html_report, "<tr class='table-header'>\n");
		fprintf(html_report, "<td class='test-result table-header'>Iteration</td>\n");
		fprintf(html_report, "<td class='test-result table-header'>Date</td>\n");
		fprintf(html_report, "<td class='test-result table-header'>Log</td>\n");
		fprintf(html_report, "<td class='test-result table-header'>Result</td>\n");
		fprintf(html_report, "<td class='test-result table-header'>ErrorCode</td>\n");
		fprintf(html_report, "</tr>\n");
		break;
	case TST_RUN:
		fprintf(html_report, "<tr class='test-result'>\n");
		fprintf(html_report, "<td class='test-result'>%d</td>\n", iter);
		fprintf(html_report, "<td class='test-result'>%s</td>\n", date);
		fprintf(html_report, "<td class='test-result'>\n");
		break;
	case TST_SKIP:
		fprintf(html_report, "</td>\n");
		fprintf(html_report, "<td class='test-result skip'>SKIPPED</td>\n");
		fprintf(html_report, "<td class='test-result'></td>\n");
		fprintf(html_report, "</tr>\n");
		break;
	case TST_ERROR:
		fprintf(html_report, "</td>\n");
		fprintf(html_report, "<td class='test-result error'>ERROR</td>\n");
		fprintf(html_report, "<td class='test-result'>%d</td>\n", id);
		fprintf(html_report, "</tr>\n");
		break;
	case TST_PASS:
		fprintf(html_report, "</td>\n");
		fprintf(html_report, "<td class='test-result pass'>PASS</td>\n");
		fprintf(html_report, "<td class='test-result'></td>\n");
		fprintf(html_report, "</tr>\n");
		break;
	case TST_FAIL:
		fprintf(html_report, "</td>\n");
		fprintf(html_report, "<td class='test-result fail'>FAIL</td>\n");
		fprintf(html_report, "<td class='test-result'>%d</td>\n", id);
		fprintf(html_report, "</tr>\n");
		break;
	case TST_STRING:
		if (id == 0) {
			if (str[0] == '\n')
				fprintf(html_report, "<br/>");
			fprintf(html_report, "%s", str);
		}
		break;
	case TST_STOP:
		fprintf(html_report, "</table>\n");
		break;
	case TST_CLOSE:
		fprintf(html_report, "</body>\n");
		fprintf(html_report, "</html>\n");
		break;
	default:
		break;
	}
	fflush(html_report);

	return 0;
}

int cobaye_report_open(void)
{
	time_t now;
	char buffer[512] = "";
	char *hostname = buffer;
	char *filepath = buffer;
	char *filename = buffer;

	if(reportpath[0]) {
		return 0;
	}

	strcpy(filepath, confmenu[item_filename].string);

	if (confmenu[item_date].value == 1) {
		char date[18];
		time_t now = time(NULL);
		struct tm *now_time = localtime(&now);

		sprintf(date, "%04d%02d%02d_%02d%02d%02d_",
				now_time->tm_year + 1900, now_time->tm_mon + 1,
				now_time->tm_mday, now_time->tm_hour, now_time->tm_min,
				now_time->tm_sec);

		char *filename = strrchr(filepath, '/');
		if (filename) {
			strcpy(filename + 1, date);
		} else {
			strcpy(filepath, date);
		}
		filename = strrchr(confmenu[item_filename].string, '/');
		if (filename) {
			filename++;
		} else {
			filename = confmenu[item_filename].string;
		}
		strcat(filepath, filename);
	}

	strcpy(reportpath, filepath);
	filename += strlen(reportpath);
	mkdir(reportpath, 0777);

	if (confmenu[item_txt_report].value) {
		strcpy(filename, "/Report.txt");
		txt_report = fopen(filepath, "w");
	}
	if (confmenu[item_csv_report].value) {
		strcpy(filename, "/Report.csv");
		csv_report = fopen(filepath, "w");
	}
	if (confmenu[item_tex_report].value) {
		strcpy(filename, "/Report.tex");
		tex_report = fopen(filepath, "w");
	}
	if (confmenu[item_xml_report].value) {
		strcpy(filename, "/Report.xml");
		xml_report = fopen(filepath, "w");
	}

	if (confmenu[item_html_report].value) {
		strcpy(filename, "/Report.html");
		html_report = fopen(filepath, "w");
	}

	strcpy(filename, "");

	gethostname(hostname, 512);
	cobaye_report_all(TST_OPEN, hostname, 0, NULL, 0);

	gettimeofday(&stat_start_tests, NULL);

	return 0;
}

int cobaye_report_close(void)
{
	gettimeofday(&stat_stop_tests, NULL);

	cobaye_report_all(TST_CLOSE, NULL, 0, NULL, 0);

	strcpy(reportpath, "");

	if (tst_output)
		fclose(tst_output);
	if (txt_report)
		fclose(txt_report);
	if (csv_report)
		fclose(csv_report);
	if (tex_report)
		fclose(tex_report);
	if (xml_report)
		fclose(xml_report);
	if (html_report)
		fclose(html_report);

	tst_output = NULL;
	txt_report = NULL;
	csv_report = NULL;
	tex_report = NULL;
	xml_report = NULL;
	html_report = NULL;

	return 0;
}

int cobaye_report_all(int type, char *name, int iter, char *str, int id)
{

	switch(type) {
	case TST_RUN:
		gettimeofday(&stat_start_test, NULL);
		stat_nb_test++;
		break;
	case TST_FAIL:
		stat_nb_fail++;
	case TST_SKIP:
	case TST_ERROR:
		gettimeofday(&stat_stop_test, NULL);
		break;
	case TST_PASS:
		gettimeofday(&stat_stop_test, NULL);
		stat_nb_pass++;
		break;
	default:
		break;
	}

	if (cobaye_ncurses_enabled()) {
		cobaye_report_status(type, name, iter, str, id);
		cobaye_report_ui(type, name, iter, str, id);
	} else {
		cobaye_report_stdout(type, name, iter, str, id);
	}
	if (txt_report)
		cobaye_report_txt(type, name, iter, str, id);
	if (csv_report)
		cobaye_report_csv(type, name, iter, str, id);
	if (tex_report)
		cobaye_report_tex(type, name, iter, str, id);
	if (xml_report)
		cobaye_report_xml(type, name, iter, str, id);
	if (html_report)
		cobaye_report_html(type, name, iter, str, id);

	return 0;
}

