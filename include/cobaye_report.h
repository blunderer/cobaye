
#ifndef REPORT_H
#define REPORT_H

/* type for report entries */
#define TST_OPEN        0
#define TST_START       1
#define TST_STOP        2
#define TST_RUN         3
#define TST_SKIP        4
#define TST_ERROR       5
#define TST_PASS        6
#define TST_FAIL        7
#define TST_STRING      8
#define TST_CLOSE       9

int cobaye_report_all(int type, char *name, int iter, char *str, int id);
int cobaye_report_close(void);
int cobaye_report_open(void);

#endif /* REPORT_H */
