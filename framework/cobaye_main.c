
#include "cobaye_ncurses.h"
#include "cobaye_menus.h"
#include "cobaye_seq.h"

static int ncurses_init = 0;

WINDOW *teststatuswin;
WINDOW *statuswin;
WINDOW *titlewin;
WINDOW *menuwin;
WINDOW *mainwin;
WINDOW *helpwin;
WINDOW *ctxwin;

cobaye_declare_resource(README, README_data, README_size);

int cobaye_ncurses_enabled(void)
{
	return ncurses_init;
}

void cobaye_init_curses()
{
	mainwin = initscr();
	keypad(stdscr, TRUE);

	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_BLACK, COLOR_WHITE);
	init_pair(3, COLOR_BLUE, COLOR_WHITE);
	init_pair(4, COLOR_WHITE, COLOR_BLUE);

	ncurses_init = 1;
}

void cobaye_exit_curses()
{
	delwin(titlewin);
	endwin();
}

void cobaye_display_home(void)
{
	FILE *home = NULL;
	wclear(ctxwin);

	home = fmemopen(README_data, README_size, "r");
	if (home) {
		char buffer[128];
		wattron(ctxwin, COLOR_PAIR(3));
		while (fgets(buffer, 127, home)) {
			char *line = buffer;
			if (buffer[0] == '*') {
				wattron(ctxwin, A_BOLD);
				line++;
			} else {
				waddstr(ctxwin, " ");
			}
			waddstr(ctxwin, line);
			wattroff(ctxwin, A_BOLD);
		}
		fclose(home);
		wattroff(ctxwin, COLOR_PAIR(3));
	}
	wrefresh(ctxwin);
}

int usage(void)
{
	printf("usage: ");
	printf("cobaye [options]\n");
	printf("[options] are:\n");
	printf
	    (" -b <name>: 		batch mode: run seq/test named <name> without loading interface\n");
	printf(" -s <filename>: 	load sequence named <filename>\n");
	printf(" -o <directory>: 	output logs to <directory>\n");
	printf
	    (" -d: 			prepend date to output <directory name>\n");
	printf(" -l: 			list all available tests\n");
	printf(" -h: 			display this help\n");
	exit(1);
}

static char *batch_mode = NULL;

int parse_cmdline(int argc, char **argv)
{
	int index = 0;
	while (argc) {
		if (argv[0][0] == '-') {
			switch (argv[0][1]) {
			case 'd':
				confmenu[item_date].value = 1;
				break;
			case 'l':
				for (index = 1; index < cobaye_count - 3;
				     index++) {
					printf(" - %s (%s)\n",
					       cobaye_list[index].name,
					       (cobaye_list[index].test->
						flags & TST_NO_USER) ? "auto" :
					       "manual");
				}
				exit(0);
				break;
			case 'h':
				usage();
				break;
			default:
				if (argc <= 1) {
					printf
					    ("missing value for argument '%s'\n",
					     argv[0]);
					usage();
				}
				switch (argv[0][1]) {
				case 'b':
					batch_mode = argv[1];
					argc--;
					argv++;
					break;
				case 's':
					strcpy(optmenu[item_load].string,
					       argv[1]);
					cobaye_build_seq(&optmenu[item_load]);
					if (optmenu[item_edit].type !=
					    menutype_func) {
						optmenu[item_load].string[0] =
						    '\0';
					}
					argc--;
					argv++;
					break;
				case 'o':
					confmenu[item_log_file].value = 1;
					strcpy(confmenu[item_filename].string,
					       argv[1]);
					argc--;
					argv++;
					break;
				default:
					printf("unknown argument '%s'\n",
					       argv[0]);
					usage();
					break;
				}
			}
		} else {
			printf("unknown argument '%s'\n", argv[0]);
			usage();
		}
		argc--;
		argv++;
	}
	return 0;
}

int main(int argc, char **argv)
{
	unsigned int i;
	int x, y, center;
	char titlestr[40];

	cobaye_count = cobaye_build_list(NULL);
	cobaye_list = calloc(cobaye_count, sizeof(struct menu));
	cobaye_count = cobaye_build_list(cobaye_list);

	parse_cmdline(argc - 1, argv + 1);

	if (batch_mode) {
		int ret = 0;
		struct menu entry;
		entry.test = cobaye_test_exist(batch_mode);
		if (entry.test) {
			cobaye_run_tst(&entry);
		} else if (strcmp(batch_mode, optmenu[item_load].string) == 0) {
			cobaye_run_tst(&entry);
		} else {
			printf("test '%s' doesn't exist\n", batch_mode);
			ret = -1;
		}
		return ret;
	}

	cobaye_init_curses();

	getmaxyx(mainwin, y, x);

	if (x < 60 || y < 20) {
		cobaye_exit_curses();
		printf("Need a terminal at least 60x30\n");
		exit(1);
	}

	teststatuswin = subwin(mainwin, 4, 28, y - 14, 2);
	statuswin = subwin(mainwin, 1, x - 2, y - 2, 1);
	titlewin = subwin(mainwin, 4, x - 2, 1, 1);
	menuwin = subwin(mainwin, y - 9, 30, 6, 1);
	helpwin = subwin(mainwin, 5, 28, y - 9, 2);
	ctxwin = subwin(mainwin, y - 9, x - 33, 6, 32);

	wbkgd(teststatuswin, COLOR_PAIR(2));
	wbkgd(statuswin, COLOR_PAIR(2));
	wbkgd(titlewin, COLOR_PAIR(2));
	wbkgd(menuwin, COLOR_PAIR(2));
	wbkgd(helpwin, COLOR_PAIR(3));
	wbkgd(ctxwin, COLOR_PAIR(2));

	scrollok(ctxwin, 1);

	wattron(titlewin, A_BOLD | A_UNDERLINE);
	center = x - 2 - sprintf(titlestr, "COBAYE\n");
	center /= 2;
	mvwprintw(titlewin, 1, center, titlestr);
	wattroff(titlewin, A_BOLD | A_UNDERLINE);
	center = x - 2 - sprintf(titlestr, "- test framework -\n");
	center /= 2;
	mvwprintw(titlewin, 2, center, titlestr);

	wrefresh(titlewin);
	wrefresh(menuwin);
	wrefresh(ctxwin);

	cobaye_display_home();
	cobaye_display_status("");
	cobaye_display_help("Select a menu.");

	for (i = 0; i < (sizeof(optmenu) / sizeof(struct menu)); i++) {
		cobaye_display_menu_entry(menuwin, &optmenu[i], i);
	}

	while (1) {
		int pressed;
		cobaye_display_status("");
		cobaye_display_menu(menuwin, optmenu, i);
		cobaye_display_status("Do you really want to exit? (y|N)");
		pressed = getch();
		if ((pressed == 'Y') || (pressed == 'y'))
			break;
	}

	cobaye_exit_curses();

	return 0;
}

struct cobaye_test *cobaye_start_cobaye_list
    __attribute__ ((__section__(".cobaye"))) = NULL;