
#include "cobaye_ncurses.h"
#include "cobaye_menus.h"

struct menu *seqmenu = NULL;

static int seq_count = 0;
static int seq_mode = 0;

int cobaye_seq_mode(void)
{
	return seq_mode;
}

int cobaye_run_seq(struct menu *entry)
{
	int i = 0;
	int config_bkp = confmenu[item_repeat].value;

	seq_mode = 1;
	for (i = 0; i < seq_count; i++) {
		if (seqmenu[i].value) {
			confmenu[item_repeat].value = atoi(seqmenu[i].string);
			if (seqmenu[i].test)
				cobaye_run_tst(&seqmenu[i]);
		}
	}
	seq_mode = 0;
	confmenu[item_repeat].value = config_bkp;

	return 1;
}

int cobaye_edit_seq(struct menu *entry)
{
	int i = 0;

	for (i = 0; i < seq_count; i++) {
		cobaye_display_menu_entry(ctxwin, &seqmenu[i], i);
	}
	cobaye_display_menu(ctxwin, seqmenu, seq_count);

	return 0;
}

void cobaye_build_seq(struct menu *entry)
{
	FILE *seq = fopen(entry->string, "r");

	if (seq) {
		int i = 1;
		char seqname[128];
		char data[128];

		fgets(seqname, 127, seq);
		seqname[strlen(seqname) - 1] = '\0';
		fgets(data, 127, seq);
		sscanf(data, "%d", &seq_count);

		cobaye_display_status("loaded file %s: %s (%d tests)",
				      entry->string, seqname, seq_count);

		free(seqmenu);
		seqmenu = calloc(seq_count + 3, sizeof(struct menu));

		seqmenu[0].type = menutype_title;
		strncpy(seqmenu[0].name, seqname, 16);
		while (i <= seq_count && !feof(seq)) {
			char *cobaye_name;
			fgets(data, 127, seq);
			data[strlen(data) - 1] = '\0';

			cobaye_name = strchr(data, ':');
			if (cobaye_name) {
				int cobaye_iter = 0;
				struct cobaye_test *newtest = NULL;
				cobaye_name++;
				cobaye_iter = atoi(data);
				newtest = cobaye_test_exist(cobaye_name);
				if (newtest) {
					seqmenu[i].type = menutype_bool;
					seqmenu[i].test = newtest;
					seqmenu[i].value = 1;
					sprintf(seqmenu[i].name, "%s (x%d)",
						cobaye_name, cobaye_iter);
					sprintf(seqmenu[i].string, "%d",
						cobaye_iter);
					i++;
				}
			}
		}
		fclose(seq);

		entry[1].type = menutype_func;
		cobaye_list[cobaye_count - 3].type = menutype_func;
		sprintf(cobaye_list[cobaye_count - 3].name, "sequence: %s",
			seqname);

		seqmenu[seq_count + 1].type = menutype_disabled;
		seqmenu[seq_count + 2].type = menutype_func;
		seqmenu[seq_count + 2].func = cobaye_menu_quit;
		strcpy(seqmenu[seq_count + 2].name, "Back");
		strcpy(seqmenu[seq_count + 2].help, "Go back to main menu.");
		seq_count += 3;
	}
}

int cobaye_load_seq(struct menu *entry)
{
	cobaye_menu_prompt(entry);

	if (entry->string[0] == 0) {
		cobaye_display_status("nothing to load.");
	} else {
		struct stat buf;
		if (stat(entry->string, &buf) == 0) {
			cobaye_build_seq(entry);
		} else {
			cobaye_display_status("load %s failed: %d (%s).",
					      entry->string, errno,
					      strerror(errno));
		}
	}
	return 0;
}
