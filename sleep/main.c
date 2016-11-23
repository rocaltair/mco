#include <stdio.h>
#include <stdlib.h>
#include "mco.h"

void func(mco_schedule *S, void *ud)
{
	int i;
	int id = mco_running(S);
	int n = (int)(uintptr_t)ud;
	for (i = 0; i < n; i++) {
		int t = random() % 5 + 1;
		mco_sleep(S, t * 100);
		printf("id=%d,i=%d,t=%d\n", id, i, t);
	}
}

void mco_test(mco_schedule *S)
{
	int id1, id2;
	id1 = mco_new(S, 0, func, (void *)(uintptr_t)5);
	id2 = mco_new(S, 0, func, (void *)(uintptr_t)10);
	mco_resume(S, id1);
	mco_resume(S, id2);
	mco_run(S, 0);
}

int main(int argc, char **argv)
{
	mco_schedule *S = mco_open(0);
	mco_test(S);
	mco_close(S);
	return 0;
}
