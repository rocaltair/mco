#if (MCO_BACKEND == MCO_KQUEUE)
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <assert.h>
#include "mco.h"
#include "htimer.h"

void mco_init_mpoll(mco_schedule *S)
{
	struct poll *m_poll = mco_get_poll(S);
	m_poll->kqueuefd = kqueue();
}

void mco_poll(mco_schedule *S)
{
	int i, n;
	struct kevent kev[128];
	struct timespec ts;
	struct poll *m_poll = mco_get_poll(S);
	int kq = m_poll->kqueuefd;
	struct htimer_mgr_s *timer_mgr = mco_get_timer_mgr(S);
	int next = htimer_next_timeout(timer_mgr);
	if (next < 0)
		next = 1000;
	ts.tv_sec = next / 1000;
	ts.tv_nsec = next % 1000 * 1e6;
	assert(mco_running(S) < 0);

	n = kevent(kq, NULL, 0, kev, sizeof(kev)/sizeof(kev[0]), &ts);
	htimer_perform(timer_mgr);
	if (n <= 0) {
		return;
	}
	for (i = 0; i < n; i++) {
		struct kevent *p = &kev[i];
		int fd = p->ident;
		int id = (int)(uintptr_t)p->udata;
		int flags = p->flags;

		EV_SET(p, fd, flags, EV_DELETE, 0, 0, NULL); 
		kevent(kq, p, 1, NULL, 0, NULL);
		mco_resume(S, id);
	}
}

void mco_wait(mco_schedule *S, int fd, int flag)
{
	struct kevent changes[1]; 
	int ev_flag = 0;
	int id = mco_running(S);
	struct poll *m_poll = mco_get_poll(S);
	int kq = m_poll->kqueuefd;

	assert(id >= 0);
	assert(fd >= 0);
	assert(flag == 'r' || flag == 'w');

	ev_flag |= flag == 'r' ? EVFILT_READ : 0;
	ev_flag |= flag == 'w' ? EVFILT_WRITE : 0;
	EV_SET(&changes[0], fd, ev_flag, EV_ADD, 0, 0, (void *)(uintptr_t)(int)id); 
	kevent(kq, changes, 1, NULL, 0, NULL);
	mco_yield(S);
}

#endif
