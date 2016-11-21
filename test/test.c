#include <stdio.h>
#include <stdlib.h>
#include "mco.h"

typedef struct args_t {
	int n;
} args_t;

void func(mco_schedule *S, void *ud)
{
	int i;
	int id = mco_running(S);
	args_t *args = ud;
	for (i = 0; i < args->n; i++) {
		int t = random() % 5 + 1;
		mco_sleep(S, t * 100);
		printf("id=%d,i=%d,t=%d\n", id, i, t);
	}
}

void mco_test(mco_schedule *S)
{
	int id1, id2;
	args_t args1;
	args_t args2;

	args1.n = 5;
	args2.n = 10;
	id1 = mco_new(S, 0, func, &args1);
	id2 = mco_new(S, 0, func, &args2);
	mco_resume(S, id1);
	// mco_resume(S, id2);
	mco_run(S, 0);
}

int main(int argc, char **argv)
{
	mco_schedule *S = mco_open(0);
	mco_test(S);
	mco_close(S);
	return 0;
}
