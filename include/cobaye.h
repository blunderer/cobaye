
#ifndef COBAYE_H
#define COBAYE_H

/* tests flags */
#define TST_DEFAULT	0x0000
#define TST_NO_FORK     0x0001
#define TST_NO_USER	0x0002

typedef int (*test_fn) (void);

struct cobaye_test {
	char name[40];
	char descr[256];
	int result;
	int flags;
	test_fn main;
};

#define DECLARE_TEST(tst)               static struct cobaye_test *__cobaye_##tst __attribute__((__section__(".cobaye"))) = &tst

#define DECLARE_RES(name, data, size)	extern void *__ ## name ## _size; \
					extern void *__ ## name ## _data; \
					static long size = (long)&__ ## name ## _size; \
					static char *data = (char*)&__ ## name ## _data

#ifdef COBAYE_FRAMEWORK

#define cobaye_declare_resource(res, data, size)	DECLARE_RES(res, data, size)
#define cobaye_declare_test(test)	DECLARE_TEST(test)
#define cobaye_return(result)		exit(result)

int cobaye_printf(char *fmt, ...);
int cobaye_scanf(char *fmt, ...);
int cobaye_status(int id, char *fmt, ...);

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
