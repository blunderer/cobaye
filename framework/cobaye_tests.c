#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "cobaye_framework.h"

#ifndef max
#define max(a, b)	((a>b)?a:b)
#endif

static int cobaye_pid = 0;

int cobaye_forked(void)
{
	if (getpid() == cobaye_pid) {
		return 0;
	}
	return 1;
}

#define IN	0
#define OUT	1
#define ERR	2

static int cobaye_fork(struct cobaye_test *test, int iter)
{
	int ret = 0;
	int pid = 0;
	int err_from_tst[2];
	int data_from_tst[2];
	int data_to_tst[2];
	int ctrl_from_tst[2];
	int ctrl_to_tst[2];

	ret = pipe(err_from_tst);
	ret = pipe(data_from_tst);
	ret = pipe(data_to_tst);
	ret = pipe(ctrl_from_tst);
	ret = pipe(ctrl_to_tst);

	pid = fork();
	if (pid == 0) {
		int ret;

		/* prepare all stdin/err/out */
		close(ctrl_to_tst[OUT]);
		close(ctrl_from_tst[IN]);
		close(err_from_tst[IN]);
		close(data_to_tst[OUT]);
		close(data_from_tst[IN]);
		dup2(data_to_tst[IN], IN);
		dup2(data_from_tst[OUT], OUT);
		dup2(err_from_tst[OUT], ERR);
		close(data_to_tst[IN]);
		close(data_from_tst[OUT]);
		close(err_from_tst[OUT]);

		/* perform test */
		ret = test->main();

		fflush(stdout);
		fflush(stderr);
		
		/* synchro with framework */
		write(ctrl_from_tst[OUT], "Q", 1);
		do {
			int cmd = 0;
			read(ctrl_to_tst[IN], &cmd, 1);
			if  (cmd)
				break;
		} while (1);

		exit(ret);
	} else if (pid > 0) {
		int status;
		int tst_running = 1;

		/* listen to test result */
		close(err_from_tst[OUT]);
		close(ctrl_from_tst[OUT]);
		close(ctrl_to_tst[IN]);
		close(data_from_tst[OUT]);
		close(data_to_tst[IN]);

		/* configure inputs to be non blocking IO */
		status = fcntl(IN, F_GETFL, 0) | O_NONBLOCK;
		fcntl(IN, F_SETFL, status);
		status = fcntl(data_from_tst[IN], F_GETFL, 0) | O_NONBLOCK;
		fcntl(data_from_tst[IN], F_SETFL, status);
		status = fcntl(err_from_tst[IN], F_GETFL, 0) | O_NONBLOCK;
		fcntl(err_from_tst[IN], F_SETFL, status);
		status = fcntl(ctrl_from_tst[IN], F_GETFL, 0) | O_NONBLOCK;
		fcntl(ctrl_from_tst[IN], F_SETFL, status);

		/* While test is running: proxy std streams */
		while (tst_running) {
			int nbytes;
			char buffer[128];

			/* check for test termination */
			nbytes = read(ctrl_from_tst[IN], buffer, 1);
			if (nbytes > 0)
				tst_running = 0;

			/* check stdout from test process */
			do {
				memset(buffer, 0, sizeof(buffer));
				nbytes = read(data_from_tst[IN], buffer, 128);
				if (nbytes > 0)
					cobaye_report_all(TST_STRING, test->name, iter, buffer, 1);
			} while (nbytes > 0);

			/* check stderr from test process */
			do {
				memset(buffer, 0, sizeof(buffer));
				nbytes = read(err_from_tst[IN], buffer, 128);
				if (nbytes > 0)
					cobaye_report_all(TST_STRING, test->name, iter, buffer, 2);
			} while (nbytes > 0);

			/* check stdin from console */
			do {
				memset(buffer, 0, sizeof(buffer));
				nbytes = read(IN, buffer, 128);
				if (nbytes > 0) {
					cobaye_report_all(TST_STRING, test->name, iter, buffer, 0);
					write(data_to_tst[OUT], buffer, nbytes);
				}
			} while (nbytes > 0);
		}

		write(ctrl_to_tst[OUT], "Q", 1);

		ret = waitpid(pid, &status, 0);
		if (ret == pid) {
			ret = WEXITSTATUS(status);
			test->result = ret;
		}

		close(err_from_tst[IN]);
		close(data_from_tst[IN]);
		close(data_to_tst[OUT]);
		close(ctrl_from_tst[IN]);
		close(ctrl_to_tst[OUT]);
	} else {
		/* fork failed */
		ret = -1;
	}

	return ret;
}

static int cobaye_run_tst_pre(struct cobaye_entry *entry)
{
	int ret;
	int iter;
	char test_short_name[NAME_LEN];
	char *convert_name = test_short_name;

	if (!entry->seq) {
		ret = cobaye_report_open();
		if (ret < 0)
			return -1;
	}

	///strcpy(test_short_name, entry->test->name);
	///while (*convert_name) {
	///	switch (*convert_name) {
	///		case ' ':
	///		case '-':
	///		case '\t':
	///		case '\r':
	///		case '\n':
	///		case '\\':
	///		case '"':
	///		case '\'':
	///		case '/':
	///		case '*':
	///			*convert_name = '_';
	///			break;
	///	}
	///	convert_name++;
	///}
	return 0;
}

static int cobaye_run_tst_post(struct cobaye_entry *entry)
{
	if (!entry->seq)
		cobaye_report_close();
	return 0;
}

int cobaye_run_tst(struct cobaye_entry *entry)
{
	int ret = 0;

	cobaye_pid = getpid();

	cobaye_run_tst_pre(entry);
	
	if (entry->test) {
		int iter;

		cobaye_report_all(TST_START, entry->test->name, 0, NULL, 0);

		for (iter = 0; iter < cobaye_conf[item_repeat].value; iter++) {
			if (((entry->test->flags & TST_NO_USER) == 0)) {
				cobaye_report_all(TST_SKIP, entry->test->name, iter, NULL, 0);
				break;
			}
			cobaye_report_all(TST_RUN, entry->test->name, iter, NULL, 0);

			/* Run the test */
			if (entry->test->flags & TST_NO_FORK)
				ret = entry->test->main();
			else
				ret = cobaye_fork(entry->test, iter);

			if (ret < 0) {
				cobaye_report_all(TST_ERROR, entry->test->name, iter, NULL, ret);
			} else if (ret == 0) {
				cobaye_report_all(TST_PASS, entry->test->name, iter, NULL, ret);
			} else {
				cobaye_report_all(TST_FAIL, entry->test->name, iter, NULL, ret);
				ret = -1;
			}

			if ((ret != 0) && (cobaye_conf[item_stop_on_error].value == 1))
				break;
		}

		cobaye_report_all(TST_STOP, entry->test->name, 0, NULL, 0);

	} else {
		cobaye_run_seq(entry);
	}

	cobaye_run_tst_post(entry);


	return 1;
}
