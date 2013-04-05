
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
#include "cobaye_report.h"
#include "cobaye_menus.h"

#ifndef max
#define max(a, b)	((a>b)?a:b)
#endif

int cobaye_count = 0;
struct menu *cobaye_list = NULL;

static int cobaye_pid = 0;

/* State machine for pipe redirection */
#define STATUS_EOT	0
#define STATUS_INIT	1
#define STATUS_STRING	2

int cobaye_process_std(int in_test, int out_test, int in_cobaye)
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
	FD_SET(in_test, &rdfs);
	if (in_cobaye >= 0) {
		FD_SET(in_cobaye, &rdfs);
	}

	err = select(max(in_test, in_cobaye) + 1, &rdfs, NULL, NULL, &timeout);
	if (err > 0) {
		if (FD_ISSET(in_test, &rdfs)) {
			err = read(in_test, data, 1);

			switch(data[0]) {
			case 0x02:		/* Start of transmition */
				status = STATUS_INIT;
				break;
			case 0x17:		/* End of transmition */
				status = STATUS_EOT;
				break;
			default:
				if(status == STATUS_INIT) {
					output = data[0] - '0';
					status = STATUS_STRING;
				} else if(status == STATUS_STRING) {
					cobaye_report_all(TST_STRING, NULL, 0, data, output);
				}
				break;
			}
		} else if (in_cobaye >= 0 && FD_ISSET(in_cobaye, &rdfs)) {
			read(in_cobaye, data, 1);
			write(out_test, data, 1);
			if (data[0] == '\r' || data[0] == '\n') {
				data[0] = '\n';
			}
			cobaye_report_all(TST_STRING, NULL, 0, data, 0);
		} else {
			err = 0;
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
		/* prepare all stdin/err/out */
		close(in[1]);
		close(out[0]);
		dup2(in[0], 0);
		dup2(out[1], 1);
		dup2(out[1], 2);
		close(in[0]);
		close(out[1]);

		/* perform test */
		exit(test->main());

	} else if (pid > 0) {
		/* listen to test result */
		close(out[1]);
		close(in[0]);

		while (1) {
			int err;
			int status;

			test->result = -1;
			cobaye_process_std(out[0], in[1], 0);

			err = waitpid(pid, &status, WNOHANG);
			if (err == pid) {
				while (cobaye_process_std(out[0], -1, -1) > 0);
				ret = WEXITSTATUS(status);
				test->result = WEXITSTATUS(status);
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

static int cobaye_run_tst_pre(struct menu *entry)
{
	int ret;
	int iter;
	char test_short_name[128];
	char *convert_name = test_short_name;

	if (!cobaye_seq_mode()) {
		wclear(ctxwin);

		ret = cobaye_report_open();
		if (ret < 0) {
			return -1;
		}
	}

	if (entry->test == NULL) {
		return cobaye_run_seq(entry);
	} 

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
	return 0;
}

static int cobaye_run_tst_post(struct menu *entry)
{
	if (!cobaye_seq_mode()) {
		cobaye_report_close();

		if (cobaye_ncurses_enabled()) {
			while (1) {
				int pressed = getch();
				if ((pressed == '\n') || (pressed == ' ') || (pressed == 'q') || (pressed == 'Q')) {
					break;
				}
			}
		}
	}
	return 0;
}

int cobaye_run_tst(struct menu *entry)
{
	int ret = 0;

	cobaye_pid = getpid();

	cobaye_run_tst_pre(entry);
	
	if (entry->test) {
		int iter;

		cobaye_report_all(TST_START, entry->test->name, 0, NULL, 0);

		for (iter = 0; iter < confmenu[item_repeat].value; iter++) {
			if (((entry->test->flags & TST_NO_USER) == 0) && !cobaye_ncurses_enabled()) {
				cobaye_report_all(TST_SKIP, entry->test->name, iter, NULL, 0);
				break;
			}
			cobaye_report_all(TST_RUN, entry->test->name, iter, NULL, 0);

			if (entry->test->flags & TST_NO_FORK) {
				ret = entry->test->main();
			} else {
				ret = cobaye_fork(entry->test);
			}

			if (ret < 0) {
				cobaye_report_all(TST_ERROR, entry->test->name, iter, NULL, ret);
			} else if (ret == 0) {
				cobaye_report_all(TST_PASS, entry->test->name, iter, NULL, ret);
			} else {
				cobaye_report_all(TST_FAIL, entry->test->name, iter, NULL, ret);
				ret = -1;
			}

			if ((ret != 0) && (confmenu[item_stop_on_error].value == 1))
				break;
		}

		cobaye_report_all(TST_STOP, entry->test->name, 0, NULL, 0);

	}

	cobaye_run_tst_post(entry);


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
