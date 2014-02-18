#ifndef COBAYE_FRAMEWORK_H
#define COBAYE_FRAMEWORK_H

#include "cobaye.h"

#define COBAYE_ITEM_MAX (item_last)

enum cobaye_conf {
	item_stop_on_error = 0,
	item_repeat,
	item_report,
	item_filename,
	item_date,
	item_last,
};

enum cobaye_type {
	cobaye_inval = -1,
	cobaye_func = 0,
	cobaye_int = 1,
	cobaye_str = 2,
};

struct cobaye_entry {
	enum cobaye_type type;
	char name[NAME_LEN];

	struct cobaye_test *test;
	struct cobaye_entry *seq;

	union {
		int value;
		char *string;
	};
};

extern struct cobaye_entry cobaye_conf[COBAYE_ITEM_MAX];
extern struct cobaye_test *cobaye_start_cobaye_list;
extern struct cobaye_test *cobaye_end_cobaye_list;
extern struct cobaye_report *cobaye_start_report_list;
extern struct cobaye_report *cobaye_end_report_list;

extern unsigned int cobaye_count;
extern unsigned int cobaye_report_count;
extern struct cobaye_entry *cobaye_list;
extern struct cobaye_report **cobaye_report;

/* cobaye main */
struct cobaye_entry *cobaye_test_exist(char *name);

/* cobaye_test */
int cobaye_forked(void);
int cobaye_run_tst(struct cobaye_entry *entry);

/* cobaye_seq */
int cobaye_run_seq(struct cobaye_entry *entry);
struct cobaye_entry *cobaye_build_seq(char *name);
void cobaye_destroy_seq(struct cobaye_entry *seq);

/* cobaye_report */
int cobaye_build_report_list(struct cobaye_report ***list);
int cobaye_report_all(int type, char *name, int iter, char *str, int id);
int cobaye_report_close(void);
int cobaye_report_open(void);

#endif /* COBAYE_FRAMEWORK_H */
