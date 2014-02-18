#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>

#include "cobaye_framework.h"

static char reportpath[512] = "";

/* tests statistics */
static struct timeval stat_start_tests;
static struct timeval stat_stop_tests;
static struct timeval stat_start_test;
static struct timeval stat_stop_test;

static int stat_nb_pass = 0;
static int stat_nb_fail = 0;
static int stat_nb_test = 0;

int cobaye_build_report_list(struct cobaye_report ***list)
{
	*list = &cobaye_start_report_list + 1;

	return (&cobaye_end_report_list - &cobaye_start_report_list) - 1;
}

static int cobaye_report_stdout(int type, char *name, int iter, char *str, int id)
{
	switch(type) {
	case TST_START:
		break;
	case TST_RUN:
		printf("********************************\n");
		printf("Running %s #%d\n", name, iter);
		break;
	case TST_ERROR:
		printf("%s(%d): Error %d: %s\n", name, iter, id, strerror(id));
		break;
	case TST_PASS:
		printf("%s(%d): Success\n", name, iter);
		break;
	case TST_SKIP:
		printf("%s(%d): Skip\n", name, iter);
		break;
	case TST_FAIL:
		printf("%s(%d): Fail %d\n", name, iter, id);
		break;
	case TST_STRING:
		if (id == 1 || id == 2)
			printf(">> %s", str);
		else if (id == 0)
			printf("<< %s", str);
		fflush(stdout);
		break;
	case TST_CLOSE:
		break;
	default:
		break;
	}
	fflush(stdout);
	return 0;
}

int cobaye_report_open(void)
{
	unsigned int index;
	time_t now;
	char buffer[512] = "";
	char *hostname = buffer;
	char *filepath = buffer;
	char *filename = buffer;

	if (!cobaye_conf[item_report].value)
		return 0;

	if(reportpath[0])
		return 0;

	strcpy(filepath, cobaye_conf[item_filename].string);

	if (cobaye_conf[item_date].value == 1) {
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
		filename = strrchr(cobaye_conf[item_filename].string, '/');
		if (filename) {
			filename++;
		} else {
			filename = cobaye_conf[item_filename].string;
		}
		strcat(filepath, filename);
	}

	strcpy(reportpath, filepath);
	filename += strlen(reportpath);
	mkdir(reportpath, 0777);

	for (index = 0; index < cobaye_report_count; index++) {
		if (cobaye_conf[item_report].value & (1 << index)) {
			printf("Open new report type %s in %s\n", cobaye_report[index]->ext, reportpath);
			strcpy(cobaye_report[index]->path, reportpath);
			sprintf(filename, "/Report.%s", cobaye_report[index]->ext);
			cobaye_report[index]->stream = fopen(filepath, "w");
		}
	}
	strcpy(filename, "");

	gethostname(hostname, 512);
	cobaye_report_all(TST_OPEN, hostname, 0, NULL, 0);

	gettimeofday(&stat_start_tests, NULL);

	return 0;
}

int cobaye_report_close(void)
{
	unsigned int index;

	if (!cobaye_conf[item_report].value)
		return 0;

	gettimeofday(&stat_stop_tests, NULL);
	cobaye_report_all(TST_CLOSE, NULL, 0, NULL, 0);

	for (index = 0; index < cobaye_report_count; index++) {
		if (cobaye_conf[item_report].value & (1 << index)) {
			fclose(cobaye_report[index]->stream);
			cobaye_report[index]->stream = NULL;
			strcpy(cobaye_report[index]->path, "");
		}
	}

	strcpy(reportpath, "");

	return 0;
}

int cobaye_report_all(int type, char *name, int iter, char *str, int id)
{
	unsigned int index = 0;

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

	cobaye_report_stdout(type, name, iter, str, id);

	if (!cobaye_conf[item_report].value)
		return 0;

	for (index = 0; index < cobaye_report_count; index++) {
		if (cobaye_conf[item_report].value & (1 << index)) {
			cobaye_report[index]->report(type, name, iter, str, id);
			fflush(cobaye_report[index]->stream);
		}
	}

	return 0;
}

