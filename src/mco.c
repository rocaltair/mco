#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <ucontext.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <execinfo.h>
#include <unistd.h>
#include <stdio.h>
#include "mco.h"
#include "htimer.h"
#include "mpoll.h"

#ifndef MCO_DEFAULT_ST_SZ
# define MCO_DEFAULT_ST_SZ (32 * 1024)
#endif 
#define MAX(a, b) ((a) > (b) ? (a) : (b))


/*
 * #define ENABLE_MCO_DEBUG
 */

#ifdef ENABLE_MCO_DEBUG
#include <stdio.h>
# define DLOG(fmt, ...) fprintf(stderr, "<mco>" fmt "\n", ##__VA_ARGS__)
#else
# define DLOG(...)
#endif

struct mcoco;
typedef struct mcoco mcoco;

typedef void (*mkctx_func)();

struct mco_schedule {
	ucontext_t main;
	htimer_mgr_t timer_mgr;
	int dst_sz;
	int running;

	int nco;
	int nactive;
	int cap;
	mcoco **co;
	struct poll m_poll;

	int later_list[1024];
	int later_cnt;
};

struct mcoco {
	int status;
	int st_sz;
	char *stack;
	mco_func func;
	ucontext_t ctx;
	void *ud;
};

struct timer_handler {
	htimer_t timer;
	mco_schedule *S;
	int id;
};

static mcoco* new_mcoco(mco_schedule *S,
			 int st_sz,
			 mco_func func,
			 void *ud);
static void delete_mcoco(mcoco *C);
static void mco_main(uint32_t low, uint32_t high);
static void mco_on_resume_later(mco_schedule *S);

mco_schedule* mco_open(int st_sz)
{
	mco_schedule * S = malloc(sizeof(*S));
	S->dst_sz = MAX(st_sz, MCO_DEFAULT_ST_SZ);
	S->cap = 16;
	S->nco = 0;
	S->running = -1;
	S->later_cnt = 0;
	htimer_mgr_init(&S->timer_mgr);
	mco_init_mpoll(S);
	S->co = malloc(sizeof(mcoco *) * S->cap);
	memset(S->co, 0, sizeof(mcoco *) * S->cap);
	return S;
}

static void timer_cb(htimer_t *handler)
{
	struct timer_handler *pth = (struct timer_handler *)handler;
	mco_resume(pth->S, pth->id);
}

void mco_sleep(mco_schedule *S, int ms)
{
	int id = S->running;
	struct timer_handler th;
	mco_assert(id >= 0 && id < S->cap);
	th.S = S;
	th.id = id;
	htimer_init(&S->timer_mgr, &th.timer);
	htimer_start(&th.timer, timer_cb, ms, 0);
	mco_yield(S);
}

void mco_run(mco_schedule *S, int flag)
{
	do {
		mco_on_resume_later(S);
		if (mco_active_sz(S) <= 0)
			return;
		mco_poll(S);
	} while(flag == MCO_RUN_DEFAULT);
}

void mco_close(mco_schedule *S)
{
	int id;
	for (id = 0; id < S->cap; id++) {
		mcoco *C = S->co[id];
		if(C == NULL)
			continue;
		delete_mcoco(C);
		S->co[id] = NULL;
	}
	free(S->co);
	S->co = NULL;
	free(S);
}

int mco_new(mco_schedule *S, int st_sz, mco_func func, void *ud)
{
	int i;
	int id = -1;
	mcoco *C = NULL;
	if (S->nco >= S->cap) {
		int ocap = S->cap;
		mcoco ** co = realloc(S->co, sizeof(mcoco *) * ocap * 2);
		if (co == NULL)
			goto err_nomem;
		S->cap *= 2;
		S->co = co;
		mco_assert(S->co != NULL);
		memset(S->co + ocap, 0, sizeof(mcoco *) * ocap);
	}
	C = new_mcoco(S, st_sz, func, ud);
err_nomem:
	if (C == NULL) {
		errno = ENOMEM;
		return -1;
	}
	for (i = 0; i <= S->cap; i++) {
		id = (i + S->nco) % S->cap;
		if (S->co[id] == NULL)
			break;
	}
	S->nco++;
	S->co[id] = C;
	return id;
}

int mco_create(mco_schedule *S, int st_sz, mco_func func, void *ud)
{
	int id = mco_new(S, st_sz, func, ud);
	mco_resume(S, id);	
	return id;
}

static void mco_on_resume_later(mco_schedule *S)
{
	int i;
	for (i = 0; i < S->later_cnt; i++) {
		int id = S->later_list[i];
		if (id >= 0)
			mco_resume(S, id);
	}
	S->later_cnt = 0;
}

void mco_resume_later(mco_schedule *S, int id)
{
	mco_assert(S->later_cnt + 1 < sizeof(S->later_list)/sizeof(S->later_list[0]));
	S->later_list[S->later_cnt++] = id;
}

void mco_resume(mco_schedule *S, int id)
{
	mcoco *C = NULL;
	DLOG("resume,id=%d,running=%d", id, S->running);
	mco_assert(id >= 0 && id < S->cap);
	/*
	 * mco_assert(S->running == -1);
	 */

	C = S->co[id];
	mco_assert(C != NULL);

	/* TODO, return if status == MCO_RUNING? */
	mco_assert(C->status != MCO_RUNING && "don't resume co while running");
	if (S->running >= 0) {
		mco_resume_later(S, id);
		return;
	}

	switch(C->status) {
	case MCO_READY:
		S->nactive++;
		mco_assert(S->nactive <= S->nco);
		getcontext(&C->ctx);
		C->ctx.uc_stack.ss_sp = C->stack;
		C->ctx.uc_stack.ss_size = C->st_sz;
		C->ctx.uc_stack.ss_flags = 0;
		C->ctx.uc_link = &S->main;
		C->status = MCO_RUNING;
		S->running = id;
		makecontext(&C->ctx, (mkctx_func)mco_main, 2,
			    (uint32_t)(uintptr_t)S,
			    (uint32_t)((uintptr_t)S>>32));
		swapcontext(&S->main, &C->ctx);
		break;
	case MCO_SUSPEND:
		S->running = id;
		C->status = MCO_RUNING;
		swapcontext(&S->main, &C->ctx);
		break;
	}
}

void mco_yield(mco_schedule *S)
{
	mcoco *C;
	int id = S->running;
	mco_assert(id >= 0 && id < S->cap);

	C = S->co[id];
	mco_assert(C != NULL);
	DLOG("yield,status=%d", C->status);
	mco_assert(C->status == MCO_RUNING);
	C->status = MCO_SUSPEND;
	S->running = -1;
	swapcontext(&C->ctx, &S->main);
}

int mco_status(mco_schedule *S, int id)
{
	mcoco *C;
	mco_assert(id >= 0 && id < S->cap);
	C = S->co[id];
	if (C == NULL)
		return MCO_DEAD;
	return C->status;
}

int mco_running(mco_schedule *S)
{
	return S->running;
}


int mco_active_sz(mco_schedule *S)
{
	return S->nactive;
}

struct poll * mco_get_poll(mco_schedule *S)
{
	return &S->m_poll;
}

htimer_mgr_t * mco_get_timer_mgr(mco_schedule *S)
{
	return &S->timer_mgr;
}

static mcoco* new_mcoco(mco_schedule *S,
			 int st_sz,
			 mco_func func,
			 void *ud)
{
	mcoco *C = malloc(sizeof(*C));
	if (C == NULL)
		return NULL;

	C->status = MCO_READY;
	C->func = func;
	C->ud = ud;

	if (st_sz <= 0)
		st_sz = S->dst_sz;
	else if (st_sz < MCO_DEFAULT_ST_SZ)
		st_sz = MCO_DEFAULT_ST_SZ;
	C->st_sz = st_sz;
	C->stack = malloc(st_sz);

	if (C->stack == NULL) {
		free(C);
		C = NULL;
	}
	return C;
}

static void delete_mcoco(mcoco *C)
{
	free(C->stack);
	C->stack = NULL;
	free(C);
}

static void mco_main(uint32_t low, uint32_t high)
{
	uintptr_t p = (uintptr_t)low | ((uintptr_t) high << 32);
	mco_schedule *S = (mco_schedule *)p;
	int id = S->running;
	mcoco *C = S->co[id];
	C->func(S, C->ud);
	S->co[id] = NULL;
	S->running = -1;
	delete_mcoco(C);
	S->nco--;
	S->nactive--;
	mco_assert(S->nactive <= S->nco);
}

void mco_dump_traceback(const char *caller)
{
	const size_t max_frame_size = 64; 
	void *frames[max_frame_size];
	size_t frame_size;
	fprintf(stderr, "Stack Trace Start for <%s>\n", caller);
	frame_size = backtrace(frames, sizeof(frames) / sizeof(frames[0]));
	/*you can also use backtrace_symbols here */
	backtrace_symbols_fd(frames, frame_size, STDERR_FILENO);
}

