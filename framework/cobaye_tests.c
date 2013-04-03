
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>

#include "cobaye_ncurses.h"
#include "cobaye_menus.h"

#ifndef max
#define max(a, b)	((a>b)?a:b)
#endif

int cobaye_count = 0;
struct menu *cobaye_list = NULL;

static int cobaye_pid = 0;

static char reportpath[512];
static FILE *logfile = NULL;
static FILE *reportfile = NULL;

/* tests statistics */
static struct timeval stat_start_test;
static struct timeval stat_stop_test;
static int stat_nb_pass = 0;
static int stat_nb_fail = 0;
static int stat_nb_test = 0;

/* State machine for pipe redirection */
#define STATUS_EOT	0
#define STATUS_INIT	1
#define STATUS_STRING	2

int cobaye_process_std(int in1, int out1, int in2, int out2)
{
	int err;
	static int output = 0;
	static int status = STATUS_EOT;

	char data[2] = "\0\0";
	struct timeval timeout;
	fd_set rdfs;

	timeout.tv_sec = 0;
	timeout.tv_usec = 500000;

	FD_ZERO(&rdfs);
	FD_SET(in1, &rdfs);
	if (in2 >= 0) {
		FD_SET(in2, &rdfs);
	}

	err = select(max(in1, in2) + 1, &rdfs, NULL, NULL, &timeout);
	if (err > 0) {
		if (FD_ISSET(in1, &rdfs)) {
			if (read(in1, data, 1) > 0) {
				if (status == STATUS_EOT) {
					if (data[0] == 0x02) {
						status = STATUS_INIT;
					}
				} else {
					if (data[0] == 0x17) {
						status = STATUS_EOT;
					} else {
						if (status == STATUS_INIT) {
							output = data[0] - '0';
							status = STATUS_STRING;
						} else if (status ==
							   STATUS_STRING) {
							if (output == 0) {
								if (logfile) {
									fwrite
									    (data,
									     1,
									     1,
									     logfile);
								}
								if (cobaye_ncurses_enabled()) {
									waddstr
									    (ctxwin,
									     data);
									wrefresh
									    (ctxwin);
								} else {
									printf
									    ("%s",
									     data);
									fflush
									    (stdout);
								}
							} else {
								cobaye_display_cobaye_status
								    (output,
								     data);
							}
						}
					}
				}
			} else {
				err = 0;
			}
		}

		if (in2 >= 0 && FD_ISSET(in2, &rdfs)) {
			if (read(in2, data, 1) > 0) {
				write(out2, data, 1);
				if (data[0] == '\r') {
					data[0] = '\n';
				}
				if (logfile)
					fwrite(data, 1, 1, logfile);
				waddstr(ctxwin, data);
				wrefresh(ctxwin);
			} else {
				err = 0;
			}
		}
	}
	return err;
}

int cobaye_forked(void)
{
	if (getpid() == cobaye_pid) {
		return 0;
	}
	return 1;
}

int cobaye_fork(struct cobaye_test *test)
{
	int ret = 0;
	int pid = 0;
	int out[2];
	int in[2];

	ret = pipe(out);
	ret = pipe(in);

	pid = fork();
	if (pid == 0) {
		/* perform test */
		close(out[0]);
		close(in[1]);
		dup2(out[1], 1);
		dup2(out[1], 2);
		dup2(in[0], 0);
		close(out[1]);
		close(in[0]);
		exit(test->main());
	} else if (pid > 0) {
		/* listen to test result */
		close(out[1]);
		close(in[0]);

		while (1) {
			int err;
			int status;

			test->result = -1;
			cobaye_process_std(out[0], -1, 0, in[1]);

			err = waitpid(pid, &status, WNOHANG);
			if (err == pid) {
				while (cobaye_process_std(out[0], -1, -1, -1) >
				       0) ;
				ret = test->result = status;
				break;
			}
		}
		close(out[0]);
		close(in[1]);
	} else {
		/* fork failed */
		ret = -1;
	}

	return ret;
}

int cobaye_set_curr_logfile(char *name)
{
	char filepath[512] = "";

	if (confmenu[item_log_file].value == 1) {
		sprintf(filepath, "%s/%s.txt", reportpath, name);

		if (logfile) {
			fclose(logfile);
			logfile = NULL;
		}
		logfile = fopen(filepath, "w");
		if (!logfile) {
			cobaye_display_status("Failed to open file %s",
					      filepath);
			return -1;
		}
	}

	return 0;
}

int cobaye_init_logging(void)
{
	time_t now;
	char buffer[512] = "";
	char *hostname = buffer;
	char *filepath = buffer;

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
	mkdir(filepath, 0777);
	strcat(filepath, "/Report.txt");

	reportfile = fopen(filepath, "w");
	if (!reportfile) {
		cobaye_display_status("Failed to open file %s", filepath);
		return -1;
	}

	now = time(NULL);
	gethostname(hostname, 512);

	/* This is the header of the Report.txt file.
	   Test duration and # of test are not know yet so we just 
	   add a line filled with whitespaces so that we can insert 
	   them later (see cobaye_close_logging).
	 */

	fprintf(reportfile, "######################################\n");
	fprintf(reportfile, "##            Test Report           ##\n");
	fprintf(reportfile, "######################################\n");
	fprintf(reportfile, "\n");
	fprintf(reportfile, "- test host:        %s\n", hostname);
	fprintf(reportfile, "- test date (GMT):  %s\n", asctime(gmtime(&now)));
	fprintf(reportfile, "- test duration:    sec\n");
	fprintf(reportfile, "- test passed:      \n");
	fprintf(reportfile, "                                      \n");
	fprintf(reportfile, "############### RESULTS ##############\n");
	fprintf(reportfile, "\n");
	fflush(reportfile);

	return 0;
}

int cobaye_close_logging(void)
{
	if (logfile) {
		fclose(logfile);
		logfile = NULL;
	}
	if (reportfile) {
		int i = 0;
		int duration = 0;
		char buffer[512];
		char *filepath = buffer;

		strcpy(filepath, reportpath);
		strcat(filepath, "/Report.txt");
		fclose(reportfile);
		reportfile = fopen(filepath, "r+");
		fseek(reportfile, 0, SEEK_SET);

		duration =
		    (stat_stop_test.tv_sec - stat_start_test.tv_sec) * 1000;
		duration +=
		    (stat_stop_test.tv_usec - stat_start_test.tv_usec) / 1000;

		duration = 1020;
		for (i = 0; i < 6; i++) {
			fgets(buffer, 512, reportfile);
		}
		fprintf(reportfile, "- test duration:    %d sec\n",
			duration / 1000);
		fprintf(reportfile, "- test passed:      %d/%d\n", stat_nb_pass,
			stat_nb_test);
		fflush(reportfile);
		fclose(reportfile);

		reportfile = NULL;
	}
	return 0;
}

int cobaye_run_tst(struct menu *entry)
{
	int ret = 0;
	char test_short_name[128];

	cobaye_pid = getpid();

	if (!cobaye_seq_mode()) {
		wclear(ctxwin);
		stat_nb_test = 0;
		stat_nb_pass = 0;
		stat_nb_fail = 0;

		gettimeofday(&stat_start_test, NULL);

		if (confmenu[item_log_file].value) {
			ret = cobaye_init_logging();
			if (ret < 0) {
				return -1;
			}
		}
	}

	if (entry->test == NULL) {
		cobaye_run_seq(entry);
	} else {
		int iter = 0;
		char *convert_name = test_short_name;
		strcpy(test_short_name, entry->test->name);
		while (*convert_name) {
			switch (*convert_name) {
			case ' ':
			case '-':
			case '\t':
			case '\r':
			case '\n':
			case '\\':
			case '"':
			case '\'':
			case '/':
			case '*':
				*convert_name = '_';
				break;
			}
			convert_name++;
		}
		if (((entry->test->flags & TST_NO_USER) == 0)
		    && !cobaye_ncurses_enabled()) {
			printf("  %s(%d): Skipping\n", entry->test->name, iter);
			if (logfile) {
				fprintf(logfile, "==%d== %s(%d): Skipping\n",
					getpid(), entry->test->name, iter);
				fflush(logfile);
			}
			if (reportfile) {
				fprintf(reportfile,
					"SKIPPED\t\t\tTest '%s'\t(iteration %d): cannot run in batch mode because it requires user input\n",
					entry->test->name, iter);
				fflush(reportfile);
			}
		} else if (cobaye_set_curr_logfile(test_short_name) == 0) {
			for (iter = 0; iter < confmenu[item_repeat].value;
			     iter++) {
				if (cobaye_ncurses_enabled()) {
					wattron(ctxwin, A_BOLD | COLOR_PAIR(3));
					wprintw(ctxwin, "\n\n");
					wprintw(ctxwin, "  %s(%d): Running\n",
						entry->test->name, iter);
					wrefresh(ctxwin);
					wattroff(ctxwin, A_BOLD);
				} else {
					printf("  %s(%d): Running\n",
					       entry->test->name, iter);
				}
				cobaye_display_status("Running test %s(%d)",
						      entry->test->name, iter);
				if (logfile) {
					fprintf(logfile,
						"==%d== %s(%d): Running\n",
						getpid(), entry->test->name,
						iter);
					fflush(logfile);
				}

				if (entry->test->flags & TST_NO_FORK) {
					ret = entry->test->main();
				} else {
					ret = cobaye_fork(entry->test);
				}
				stat_nb_test++;

				if (cobaye_ncurses_enabled()) {
					wattron(ctxwin, A_BOLD);
				}
				if (ret < 0) {
					if (cobaye_ncurses_enabled()) {
						wprintw(ctxwin,
							"  error starting %d while trying to run %s(%d): %s\n",
							ret, entry->test->name,
							iter, strerror(errno));
					} else {
						printf
						    ("  error starting %d while trying to run %s(%d): %s\n",
						     ret, entry->test->name,
						     iter, strerror(errno));
					}
					if (reportfile) {
						fprintf(reportfile,
							"CANNOT RUN\t\t\tTest '%s'\n(iteration %d): see %s.txt for details\n",
							entry->test->name, iter,
							test_short_name);
						fflush(reportfile);
					}
				} else {
					if (entry->test->flags & TST_NO_FORK
					    || WIFEXITED(ret)) {
						if ((entry->test->
						     flags & TST_NO_FORK) ==
						    0) {
							ret = WEXITSTATUS(ret);
						}
						if (ret == 0) {
							stat_nb_pass++;
						}
						if (cobaye_ncurses_enabled()) {
							wprintw(ctxwin,
								"  %s(%d): Returned code %d\n",
								entry->test->
								name, iter,
								ret);
						} else {
							printf
							    ("  %s(%d): Returned code %d\n",
							     entry->test->name,
							     iter, ret);
						}
						cobaye_display_status
						    ("test %s(%d) exited with code %d",
						     entry->test->name, iter,
						     ret);
						if (reportfile) {
							if (ret == 0) {
								fprintf
								    (reportfile,
								     "PASS\t\t\tTest '%s'\t(iteration %d)\n",
								     entry->
								     test->name,
								     iter);
							} else {
								fprintf
								    (reportfile,
								     "FAILED\t\t\tTest '%s'\t(iteration %d): see %s.txt for details\n",
								     entry->
								     test->name,
								     iter,
								     test_short_name);
							}
							fflush(reportfile);
						}
						if (logfile) {
							fprintf(logfile,
								"==%d== %s(%d): Exited with code %d\n\n",
								getpid(),
								entry->test->
								name, iter,
								ret);
							fflush(logfile);
						}
					} else {
						stat_nb_fail++;
						if (cobaye_ncurses_enabled()) {
							wprintw(ctxwin,
								"  %s(%d): Terminated by signal %d\n",
								entry->test->
								name, iter,
								WTERMSIG(ret));
						} else {
							printf
							    ("  %s(%d): Terminated by signal %d\n",
							     entry->test->name,
							     iter,
							     WTERMSIG(ret));
						}
						cobaye_display_status
						    ("test %s(%d) terminated by signal %d",
						     entry->test->name, iter,
						     WTERMSIG(ret));
						if (reportfile) {
							fprintf(reportfile,
								"FAILED\t\t\ttest %s iteration %d: see %s.txt for details\n",
								entry->test->
								name, iter,
								test_short_name);
							fflush(reportfile);
						}
						if (logfile) {
							fprintf(logfile,
								"==%d== %s(%d): Terminated by signal %d\n\n",
								getpid(),
								entry->test->
								name, iter,
								WTERMSIG(ret));
							fflush(logfile);
						}
						ret = -1;
					}
				}

				if ((ret != 0)
				    && (confmenu[item_stop_on_error].value ==
					1))
					break;
			}
		}
	}
	if (!cobaye_seq_mode()) {
		if (cobaye_ncurses_enabled()) {
			wprintw(ctxwin, "\n\n");
			wprintw(ctxwin, "  Press enter to continue\n");
			wrefresh(ctxwin);
			wattroff(ctxwin, A_BOLD | COLOR_PAIR(3));
		}

		gettimeofday(&stat_stop_test, NULL);

		if (confmenu[item_log_file].value) {
			cobaye_close_logging();
		}
		if (cobaye_ncurses_enabled()) {
			while (1) {
				int pressed = getch();
				if ((pressed == '\n') ||
				    (pressed == ' ') ||
				    (pressed == 'q') || (pressed == 'Q')) {
					break;
				}
			}
		}
	}

	return 1;
}

struct cobaye_test *cobaye_test_exist(char *data)
{
	int i = 1;
	for (i = 1; i < cobaye_count; i++) {
		if (strcmp(data, cobaye_list[i].name) == 0) {
			return cobaye_list[i].test;
		}
	}

	return NULL;
}

int cobaye_test_submenu(struct menu *entry)
{
	unsigned int i;

	for (i = 0; i < sizeof(testmenu) / sizeof(struct menu); i++) {
		testmenu[i].test = entry->test;
		cobaye_display_menu_entry(ctxwin, &testmenu[i], i);
	}
	cobaye_display_menu(ctxwin, testmenu, i);

	return 0;
}

int cobaye_display_tests(struct menu *entry)
{
	int i;

	for (i = 0; i < cobaye_count; i++) {
		cobaye_display_menu_entry(ctxwin, &cobaye_list[i], i);
	}
	cobaye_display_menu(ctxwin, cobaye_list, cobaye_count);
	return 0;
}

int cobaye_build_list(struct menu *menu)
{
	int index = 1;
	struct cobaye_test **ptr = &cobaye_start_cobaye_list;

	if (menu) {
		menu[0].type = menutype_title;
		strcpy(menu[0].name, "Tests");
	}

	do {
		if (*ptr) {
			if (menu) {
				menu[index].type = menutype_func;
				sprintf(menu[index].help, "Run %s test",
					(*ptr)->name);
				strcpy(menu[index].name, (*ptr)->name);
				menu[index].func = cobaye_test_submenu;
				menu[index].test = (*ptr);
			}
			index++;
		}
		ptr++;
	} while (ptr != &cobaye_end_cobaye_list);

	if (menu) {
		menu[index].type = menutype_disabled;
		menu[index].func = cobaye_test_submenu;
		menu[index].test = NULL;
		strcpy(menu[index].name, "sequence");
		strcpy(menu[index].help,
		       "launch the previously loaded test sequence.");

		menu[index + 1].type = menutype_disabled;
		strcpy(menu[index + 1].name, "");

		menu[index + 2].func = cobaye_menu_quit;
		strcpy(menu[index + 2].name, "Back");
		strcpy(menu[index + 2].help, "Go back to main menu.");
	}
	index += 3;

	return index;
}
