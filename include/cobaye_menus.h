
#ifndef MENUS_H
#define MENUS_H

#include "cobaye_menu_core.h"

enum menuconfig {
	item_title = 0,
	item_stop_on_error = 2,
	item_repeat,
	item_log_file = 6,
	item_filename,
	item_date,
};

enum mainmenu {
	item_list = 1,
	item_config,
	item_load,
	item_edit,
	item_sep0,
	item_exit,
};

extern struct menu *seqmenu;
extern struct menu *testlist;
extern struct menu testmenu[3];
extern struct menu confmenu[11];
extern struct menu optmenu[7];

int cobaye_menu_configure(struct menu *entry);

#endif /* MENUS_H */
