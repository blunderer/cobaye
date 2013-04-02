
#ifndef TESTS_H
#define TESTS_H

#include "cobaye.h"
#include "menu.h"

extern struct cobaye_test *cobaye_start_cobaye_list;
extern struct cobaye_test *cobaye_end_cobaye_list;

extern int cobaye_count;
extern struct menu *cobaye_list;

int cobaye_forked(void);
int cobaye_run_tst(struct menu *menu);
int cobaye_build_list(struct menu *menu);
int cobaye_display_tests(struct menu *entry);
struct cobaye_test *cobaye_test_exist(char *data);

#endif /* TESTS_H */
