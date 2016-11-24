#if (MCO_BACKEND == MCO_KQUEUE)
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "mco.h"
#include "htimer.h"

struct htimer_mgr_s;
struct htimer_mgr_s * mco_get_timer_mgr(mco_schedule *S);
struct poll * mco_get_poll(mco_schedule *S);

void mco_init_mpoll(mco_schedule *S)
{
	struct poll *m_poll = mco_get_poll(S);
	size_t sz;
	m_poll->kqueuefd = kqueue();
	m_poll->ncap = 16;
	sz = sizeof(struct kevent) * m_poll->ncap;
	m_poll->kev = malloc(sz);
	memset(m_poll->kev, 0, sz);
}

void mco_release_mpoll(mco_schedule *S)
{
	struct poll *m_poll = mco_get_poll(S);
	free(m_poll->kev);
	m_poll->ncap = 0;
	m_poll->nev = 0;
}

void mco_poll(mco_schedule *S)
{
	int i, n;
	struct timespec ts;
	struct poll *m_poll = mco_get_poll(S);
	struct kevent *kev = m_poll->kev;
	int kq = m_poll->kqueuefd;
	struct htimer_mgr_s *timer_mgr = mco_get_timer_mgr(S);
	int next = htimer_next_timeout(timer_mgr);
	if (next < 0)
		next = 1000;
	ts.tv_sec = next / 1000;
	ts.tv_nsec = next % 1000 * 1e6;
	assert(mco_running(S) < 0);

	n = kevent(kq, NULL, 0, kev, m_poll->ncap, &ts);
	htimer_perform(timer_mgr);
	if (n <= 0) {
		return;
	}
	for (i = 0; i < n; i++) {
		struct kevent *p = kev + i;
		int fd = p->ident;
		int id = (int)(uintptr_t)p->udata;
		int filter = p->filter;

		EV_SET(p, fd, filter, EV_DELETE, 0, 0, NULL); 
		kevent(kq, p, 1, NULL, 0, NULL);
		m_poll->nev--;
		mco_resume(S, id);
	}
}

void mco_wait(mco_schedule *S, int fd, int flag)
{
	struct kevent changes[1]; 
	int filter = 0;
	int id = mco_running(S);
	struct poll *m_poll = mco_get_poll(S);
	struct kevent *new_ev;
	int kq = m_poll->kqueuefd;

	assert(id >= 0);
	assert(fd >= 0);
	assert(flag == 'r' || flag == 'w');

	filter |= flag == 'r' ? EVFILT_READ : 0;
	filter |= flag == 'w' ? EVFILT_WRITE : 0;

	if (m_poll->nev + 1 > m_poll->ncap) {
		new_ev = realloc(m_poll->kev, m_poll->ncap * 2);
		if (new_ev != NULL) {
			m_poll->kev = new_ev;
			m_poll->ncap *= 2;
		}
	}

	EV_SET(&changes[0], fd, filter, EV_ADD, 0, 0, (void *)(uintptr_t)(int)id); 
	kevent(kq, changes, 1, NULL, 0, NULL);
	m_poll->nev++;
	mco_yield(S);
}

#endif
