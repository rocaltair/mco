#if MCO_BACKEND == MCO_EPOLL
#include <sys/epoll.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "htimer.h"
#include "mco.h"

struct htimer_mgr_s;
struct htimer_mgr_s * mco_get_timer_mgr(mco_schedule *S);
struct poll * mco_get_poll(mco_schedule *S);

static uint64_t touint64(int fd, int id)
{
	uint64_t ret = 0;
	ret |= ((uint64_t)fd) << 32;
	ret |= ((uint64_t)id);
	return ret;
}

static void fromuint64(uint64_t v, int *fd, int *id)
{
	*fd = (int)(v >> 32);
	*id = (int)(v & 0xffffffff);
}

static void mco_init_mpoll(mco_schedule *S)
{
	size_t sz;
	struct poll *m_poll = mco_get_poll(S);
	/* epoll_create(n), n is unused scince Linux 2.6.8*/
	m_poll->epollfd = epoll_create(32000);
	m_poll->ncap = 16;
	m_poll->nevents = 0;

	sz = sizeof(struct epoll_event) * m_poll->ncap;
	m_poll->events = malloc(sz);
	memset(m_poll->events, 0, sz);
}

static void mco_release_mpoll(mco_schedule *S)
{
	struct poll *m_poll = mco_get_poll(S);
	free(m_poll->events);
	m_poll->events = NULL;
	m_poll->nevents = 0;
	m_poll->ncap = 0;
}

static int mco_poll(mco_schedule *S)
{
	int i, n;
	struct poll *m_poll = mco_get_poll(S);
	struct epoll_event *events = m_poll->events;
	int epollfd = m_poll->epollfd;
	struct htimer_mgr_s *timer_mgr = mco_get_timer_mgr(S);
	int next = htimer_next_timeout(timer_mgr);

	if (next < 0)
		next = 1000;
	assert(mco_running(S) < 0);

	n = epoll_wait(epollfd, events, m_poll->ncap, next);
	htimer_perform(timer_mgr);
	if (n <= 0) {
		return 0;
	}
	for (i = 0; i < n; i++) {
		int fd;
		int id;
		struct epoll_event *p = &events[i];
		uint64_t u64 = p->data.u64;
		fromuint64(u64, &fd, &id);
		if (epoll_ctl(m_poll->epollfd, EPOLL_CTL_DEL, fd, p) == -1) {
			// TODO
		}
		m_poll->nevents--;
		mco_resume(S, id);
	}
	return 0;
}

void mco_wait(mco_schedule *S, int fd, int flag)
{
	int id = mco_running(S);
	struct poll *m_poll = mco_get_poll(S);
	struct epoll_event ev;
	struct epoll_event *new_events;

	assert(id >= 0);
	assert(flag == 'r' || flag == 'w');

	if (m_poll->nevents + 1 > m_poll->ncap) {
		new_events = realloc(m_poll->events, m_poll->ncap * 2);
		if (new_events != NULL) {
			m_poll->events = new_events;
			m_poll->ncap *= 2;
		}
	}

	ev.events = flag == 'r' ? EPOLLIN : EPOLLOUT;
	ev.data.u64 = touint64(fd, id);

	if (epoll_ctl(m_poll->epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		return;
	}
	m_poll->nevents++;
	mco_yield(S);
}

#endif
