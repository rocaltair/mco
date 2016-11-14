#ifndef  _MCO_H_XWVVDJDC_
#define  _MCO_H_XWVVDJDC_

#if defined (__cplusplus)
extern "C" {
#endif

typedef enum {
	MCO_DEAD = 0,
	MCO_READY = 1,
	MCO_RUNING = 2,
	MCO_SUSPEND = 3,
} mco_status_t;

typedef enum {
	MCO_RUN_DEFAULT = 0,
	MCO_RUN_ONCE = 1,
} mco_flag_t;

struct mco_schedule;
typedef struct mco_schedule mco_schedule;

typedef void (*mco_func)(mco_schedule *S, void *ud);

mco_schedule* mco_open(int st_sz);
void mco_close(mco_schedule *S);
void mco_run(mco_schedule *S, int flag);
void mco_sleep(mco_schedule *S, int ms);

int mco_running(mco_schedule *S);
int mco_new(mco_schedule *S, int st_sz, mco_func func, void *ud);
void mco_resume(mco_schedule *S, int id);
void mco_yield(mco_schedule *S);
int mco_status(mco_schedule *S, int id);

#if defined (__cplusplus)
}	/*end of extern "C"*/
#endif

#endif /* end of include guard:  _MCO_H_XWVVDJDC_ */

