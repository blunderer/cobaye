
#ifndef COBAYE_H
#define COBAYE_H

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

/* tests flags */
#define TST_DEFAULT	0x0000
#define TST_NO_FORK     0x0001
#define TST_NO_USER	0x0002
#define TST_BENCHMARK	0x0004

#define NAME_LEN	64
#define DESCR_LEN	256

typedef int (*test_fn) (void);
typedef int (*report_fn) (int, char *, int, char *, int);

struct cobaye_test {
	char name[NAME_LEN];
	char descr[DESCR_LEN];
	int result;
	int flags;
	test_fn main;
};

struct cobaye_report {
	char ext[NAME_LEN];
	char path[512];
	report_fn report;
	FILE *stream;
};

#define DECLARE_TEST(tst)               static struct cobaye_test *__cobaye_test_##tst __attribute__((__section__(".cobaye.test"))) = &tst
#define DECLARE_REPORT(report)          static struct cobaye_report *__cobaye_report_##report __attribute__((__section__(".cobaye.report"))) = &report

#define DECLARE_RES(name, data, size)	extern void *__ ## name ## _size; \
					extern void *__ ## name ## _data; \
					static long size = (long)&__ ## name ## _size; \
					static char *data = (char*)&__ ## name ## _data

#ifdef COBAYE_FRAMEWORK

#define cobaye_declare_resource(res, data, size)	DECLARE_RES(res, data, size)
#define cobaye_declare_test(test)	DECLARE_TEST(test)
#define cobaye_declare_report(report)	DECLARE_REPORT(report)
#define cobaye_return(result)		exit(result)

int cobaye_printf(char *fmt, ...);
int cobaye_scanf(char *fmt, ...);
int cobaye_status(int id, char *fmt, ...);
int cobaye_run_command(const char *command);

void cobaye_stdout_get_unsafe(void);
void cobaye_stdout_put_unsafe(void);

#else /* COBAYE_FRAMEWORK */

#define cobaye_declare_resource(res, data, size)	static char *data = NULL; \
							static long size = 0

#define cobaye_declare_test(test)       int main(void) { return test.main(); }
#define cobaye_return(result)           exit(result)

#define cobaye_printf(...)		printf(__VA_ARGS__)
#define cobaye_scanf(...)		scanf(__VA_ARGS__)
#define cobaye_status(id, fmt, ...)	printf(fmt, __VA_ARGS__); printf("\n")

#define cobaye_stdout_get_unsafe()
#define cobaye_stdout_put_unsafe()

#endif /* COBAYE_FRAMEWORK */

#endif /* COBAYE_H */
