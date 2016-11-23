#if MCO_BACKEND == MCO_EPOLL
#include <sys/epoll.h>
#include <assert.h>
#include "htimer.h"
#include "mco.h"

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

void mco_init_mpoll(mco_schedule *S)
{
	struct poll *m_poll = mco_get_poll(S);
	int count = 1024;
	m_poll->epollfd = epoll_create(count);
}

void mco_poll(mco_schedule *S)
{
	int i, n;
	struct epoll_event events[128];
	struct poll *m_poll = mco_get_poll(S);
	int epollfd = m_poll->epollfd;
	struct htimer_mgr_s *timer_mgr = mco_get_timer_mgr(S);
	int next = htimer_next_timeout(timer_mgr);

	if (next < 0)
		next = 1000;
	assert(mco_running(S) < 0);

	n = epoll_wait(epollfd, events, sizeof(events)/sizeof(events[0]), next);
	if (n <= 0) {
		return;
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
		mco_resume(S, id);
	}
}

void mco_wait(mco_schedule *S, int fd, int flag)
{
	int id = mco_running(S);
	struct poll *m_poll = mco_get_poll(S);
	struct epoll_event ev;

	assert(id >= 0);
	assert(flag == 'r' || flag == 'w');
	ev.events = flag == 'r' ? EPOLLIN : EPOLLOUT;
	ev.data.u64 = touint64(fd, id);
	if (epoll_ctl(m_poll->epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		return;
	}
	mco_yield(S);
}

#endif
