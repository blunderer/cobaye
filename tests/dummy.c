
#include <stdio.h>
#include <pthread.h>
#include "cobaye.h"

/* once the resource is declared in the makefile,
 * this macro makes it reachable in your C code:
 */
cobaye_declare_resource(text_txt, text, text_len);

void *th_func(void *t)
{
	long id = (long)t;

	cobaye_status(id, "hello from %ld\n", id);

	return NULL;
}

int test_main(void)
{
	int ret;
	pthread_t th[2];

	pthread_create(&th[0], NULL, th_func, (void*)1);
	pthread_create(&th[1], NULL, th_func, (void*)2);

	cobaye_printf("get resource content: %s (len=%ld)\n", text, text_len);
	cobaye_printf("choose the return code: ");
	cobaye_scanf("%d", &ret);
	cobaye_printf("\n");

	pthread_join(th[0], NULL);
	pthread_join(th[1], NULL);

	return ret;
}

struct cobaye_test dummy = {
	.main = test_main,
	.name = "dummy",
	.descr = "dummmy test",
	.flags = TST_DEFAULT,
};

cobaye_declare_test(dummy);

