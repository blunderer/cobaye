#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cobaye_framework.h"

static int usage(void)
{
	printf("usage: ");
	printf("cobaye [options] <name>\n");
	printf("[options] are:\n");
	printf(" <name>: 		batch mode: run seq/test named <name> if not manual\n");
	printf(" -o <directory>: 	output logs to <directory>\n");
	printf(" -f <format>: 		comma separated list of fmt: " RTYPE " or all\n");
	printf(" -m: 			declare manual tests to be auto tests\n");
	printf(" -d: 			prepend date to output <directory name>\n");
	printf(" -l: 			list all available tests\n");
	printf(" -i <test>: 		display info about test or sequence\n");
	printf(" -h: 			display this help\n");
	exit(1);
}

struct cobaye_test *cobaye_start_cobaye_list
    __attribute__ ((__section__(".cobaye.test"))) = NULL;
struct cobaye_report *cobaye_start_report_list
    __attribute__ ((__section__(".cobaye.report"))) = NULL;

unsigned int cobaye_count; 
unsigned int cobaye_report_count; 
struct cobaye_entry *cobaye_list;
struct cobaye_report **cobaye_report;

struct cobaye_entry cobaye_conf[] = {
	[item_date] = {
		.type = cobaye_int,
		.name = "prepend date",
		.value = 0,
	},
	[item_repeat] = {
		.type = cobaye_int,
		.name = "repeat",
		.value = 1,
	},
	[item_filename] = {
		.type = cobaye_str,
		.name = "repor filename",
		.string = "report",
	},
	[item_report] = {
		.type = cobaye_int,
		.name = "report type",
		.value = 0,
	},
};

struct cobaye_entry *cobaye_test_exist(char *name)
{
	unsigned int i;

	for (i = 0; i < cobaye_count; i++) {
		if (strcmp(name, cobaye_list[i].name) == 0)
			return &cobaye_list[i];
	}

	return NULL;
}

static char *parse_cmdline(int argc, char **argv)
{
	unsigned int index = 0;

	while (argc) {
		char opt = argv[0][1];
		char *value = argv[1];

		if (argv[0][0] != '-')
			break;

		if (argv[0][2]) {
			value = argv[0] + 2;
			argc++;
			argv--;
		} else if (argc == 1) {
			value = NULL;
		}

		switch (opt) {
		case '-':
			break;
		case 'm':
			for (index = 0; index < cobaye_count; index++)
				cobaye_list[index].test->flags |= TST_NO_USER;
			break;
		case 'd':
			cobaye_conf[item_date].value = 1;
			break;
		case 'i':
			if (value) {
				struct cobaye_entry *entry = cobaye_test_exist(value);

				if (entry) {
					printf("test: '%s':\n", entry->name);
					printf("descr: %s\n", entry->test->descr);
					printf("flags: ");
					if (!entry->test->flags)
						printf("none");
					if (entry->test->flags & TST_NO_FORK)
						printf("NO_FORK");
					if (entry->test->flags & TST_NO_USER)
						printf("NO_USER");
					if (entry->test->flags & TST_BENCHMARK)
						printf("BENCH");
					printf("\n");
				} else {
					entry = cobaye_build_seq(value);
					if (entry) {
						int i;
						printf("sequence: '%s': %d entries\n", entry->name, entry->value);
						for (i = 0; i < entry->value; i++)
							printf("  - test '%s' x %d\n", entry[i+1].name, entry[i+1].value);
					} else {
						printf("test '%s' doesn't exist\n", value);
					}
				}
			} else {
				printf("option -i expects a test or sequence name\n");
			}
			exit(0);
			break;
		case 'l':
			for (index = 0; index < cobaye_count; index++)
				printf(" - %s (%s)\n", cobaye_list[index].name, (cobaye_list[index].test-> flags & TST_NO_USER) ? "auto" : "manual");
			exit(0);
			break;
		case 'h':
			usage();
			break;
		case 'f':
			if (value) {
				if(strstr(value, "all")) {
					cobaye_conf[item_report].value = 0xffffffff;
				} else {
					for (index = 0; index < cobaye_report_count; index++)
						if (strstr(value, cobaye_report[index]->ext))
							cobaye_conf[item_report].value |= (1 << index);
				}
				argc--;
				argv++;
			} else {
				printf("option -f expects a format specifier\n");
			}
			break;
		case 'o':
			if (value) {
				cobaye_conf[item_filename].string = strdup(value);
				argc--;
				argv++;
			} else {
				printf("option -o expects a report name\n");
			}
			break;
		default:
			printf("unknown argument '%s'\n", argv[0]);
			usage();
			break;
		}
		argc--;
		argv++;
	}

	return argv[0];
}

static int cobaye_build_list(struct cobaye_entry *list)
{
	int index = 0;
	struct cobaye_test **ptr = &cobaye_start_cobaye_list;

	do {
		if (*ptr) {
			if (list) {
				list[index].type = cobaye_func;
				strcpy(list[index].name, (*ptr)->name);
				list[index].test = (*ptr);
				list[index].seq = NULL;
				list[index].value = 1;
			}
			index++;
		}
		ptr++;
	} while (ptr != &cobaye_end_cobaye_list);

	return index;
}

int main(int argc, char **argv)
{
	int ret;
	char *target;
	struct cobaye_entry *entry;

	/* prepare test list */
	cobaye_count = cobaye_build_list(NULL);
	cobaye_list = calloc(cobaye_count, sizeof(struct cobaye_entry));
	cobaye_count = cobaye_build_list(cobaye_list);

	cobaye_report_count = cobaye_build_report_list(&cobaye_report);

	/* run cobaye */
	target = parse_cmdline(argc - 1, argv + 1);
	if (!target)
		usage();

	entry = cobaye_test_exist(target);
	if (entry) {
		ret = cobaye_run_tst(entry);
	} else  {
		struct cobaye_entry *seq = cobaye_build_seq(target);
		if (seq) {
			ret = cobaye_run_tst(seq);
			cobaye_destroy_seq(seq);
		} else {
			ret = -1;
			printf("test '%s' doesn't exist\n", target);
		}
	}

	return ret;
}
