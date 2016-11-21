#if (MCO_BACKEND == MCO_POLL)
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "htimer.h"
#include "mco.h"
#include "mpoll.h"

void mco_init_mpoll(mco_schedule *S)
{
	int i;
	struct poll *m_poll = mco_get_poll(S);
	memset(m_poll->fds, 0, sizeof(struct pollfd) * MCO_FD_MAX_SZ);
	for (i = 0; i < MCO_FD_MAX_SZ; i++) {
		m_poll->fds[i].fd = -1;
		m_poll->co_map[i] = -1;
	}
}

void mco_poll(mco_schedule *S)
{
	int i;
	int n;
	struct poll *m_poll = mco_get_poll(S);
	struct pollfd *fds = m_poll->fds;
	int nfds = m_poll->nfds;
	struct htimer_mgr_s *timer_mgr = mco_get_timer_mgr(S);
	int next = htimer_next_timeout(timer_mgr);
	if (next < 0)
		next = 1000;
	assert(mco_running(S) < 0);

	n = poll(fds, nfds, next);
	htimer_perform(timer_mgr);
	if (n <= 0) {
		return;
	}
	for (i = 0; i < nfds; i++) {
		struct pollfd *p = &m_poll->fds[i];
		int fd = p->fd;
		if (fd >= 0 && p->events & p->revents) {
			int id = m_poll->co_map[fd];
			p->fd = -1;
			p->revents = 0;
			p->events = 0;
			m_poll->co_map[fd] = -1;
			mco_resume(S, id);
		}
	}
}

void mco_wait(mco_schedule *S, int fd, int flag)
{
	int i;
	int idx;
	struct pollfd *p = NULL;
	int id = mco_running(S);
	struct poll *m_poll = mco_get_poll(S);
	assert(id >= 0);
	assert(flag == 'r' || flag == 'w');
	for (i = 0; i < MCO_FD_MAX_SZ; i++) {
		idx = (i + m_poll->nfds) % MCO_FD_MAX_SZ;
		p = &m_poll->fds[idx];
		if (p->events == 0)
			break;
	}
	assert(p != NULL && p->events == 0);
	p->fd = fd;
	p->events |= flag == 'r' ? POLLIN : 0;
	p->events |= flag == 'w' ? POLLOUT : 0;
	p->revents = 0;
	m_poll->co_map[fd] = id;
	if (m_poll->nfds < MCO_FD_MAX_SZ)
		m_poll->nfds++;
	mco_yield(S);
}

#endif
