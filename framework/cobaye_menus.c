
#include "cobaye_ncurses.h"

struct menu optmenu[7] = {
	{
	 .type = menutype_title,
	 .name = "Menu",
	 },
	{
	 .type = menutype_func,
	 .name = "Run tests",
	 .help = "Display all registered tests",
	 .func = cobaye_display_tests,
	 },
	{
	 .type = menutype_func,
	 .name = "Configure",
	 .help = "Configure execution and output of tests",
	 .func = cobaye_menu_configure,
	 },
	{
	 .type = menutype_func,
	 .name = "Load sequence",
	 .help = "Load a sequence of tests to be run",
	 .func = cobaye_load_seq,
	 },
	{
	 .type = menutype_disabled,
	 .name = "Edit sequence",
	 .help = "Edit a sequence of tests",
	 .func = cobaye_edit_seq,
	 },
	{
	 .type = menutype_disabled,
	 .name = "",
	 },
	{
	 .type = menutype_func,
	 .name = "Quit",
	 .help = "Exit the test framework",
	 .func = cobaye_menu_quit,
	 },
};

struct menu testmenu[3] = {
	{
	 .type = menutype_title,
	 .name = "This test",
	 },
	{
	 .type = menutype_func,
	 .name = "Run",
	 .help = "run this test.",
	 .func = cobaye_run_tst,
	 },
	{
	 .type = menutype_func,
	 .name = "Back",
	 .help = "Go back to test list.",
	 .func = cobaye_menu_quit,
	 },
};

struct menu confmenu[15] = {
	{
	 .type = menutype_title,
	 .name = "Configuration",
	 },
	{
	 .type = menutype_disabled,
	 .name = "Running configuration",
	 },
	{
	 .type = menutype_bool,
	 .name = "Stop on Error",
	 .help =
	 "Control wether sequence will stop on first error, or continue until the end.",
	 .func = NULL,
	 .value = 0,
	 },
	{
	 .type = menutype_int,
	 .name = "Repeat",
	 .help = "How many time to we run the test in a loop",
	 .func = NULL,
	 .value = 1,
	 },
	{
	 .type = menutype_disabled,
	 .name = "",
	 },
	{
	 .type = menutype_disabled,
	 .name = "Logging configuration",
	 },
	{
	 .type = menutype_bool,
	 .name = "TXT report",
	 .help = "Write a txt output report to a file.",
	 .func = NULL,
	 .value = 0,
	 },
	{
	 .type = menutype_bool,
	 .name = "CSV report",
	 .help = "Write a cvs output report to a file.",
	 .func = NULL,
	 .value = 0,
	 },
	{
	 .type = menutype_bool,
	 .name = "TEX report",
	 .help = "Write a tex output report to a file.",
	 .func = NULL,
	 .value = 0,
	 },
	{
	 .type = menutype_bool,
	 .name = "XML report",
	 .help = "Write an xml output report to a file.",
	 .func = NULL,
	 .value = 0,
	 },
	{
	 .type = menutype_bool,
	 .name = "HTML report",
	 .help = "Write an html output report to a file.",
	 .func = NULL,
	 .value = 0,
	 },
	{
	 .type = menutype_char,
	 .name = "Log directory name",
	 .help = "Set the name of the output file (or directory)",
	 .func = NULL,
	 .string = "/tmp/results",
	 },
	{
	 .type = menutype_bool,
	 .name = "Prepend date",
	 .help =
	 "Do we prepend the current date to the output file/directory name",
	 .func = NULL,
	 .value = 0,
	 },
	{
	 .type = menutype_disabled,
	 .name = "",
	 },
	{
	 .type = menutype_func,
	 .name = "Back",
	 .help = "Go back to main menu",
	 .func = cobaye_menu_quit,
	 },
};

int cobaye_menu_configure(struct menu *entry)
{
	unsigned int i = 0;

	for (i = 0; i < (sizeof(confmenu) / sizeof(struct menu)); i++) {
		cobaye_display_menu_entry(ctxwin, &confmenu[i], i);
	}
	cobaye_display_menu(ctxwin, confmenu, i);
	return 0;
}
