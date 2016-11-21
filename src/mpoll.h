#ifndef  _MPOLL_H_JJAT0I5Y_
#define  _MPOLL_H_JJAT0I5Y_

#if defined (__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "mco.h"

#define MCO_POLL 0
#define MCO_EPOLL 1
#define MCO_KQUEUE 2

#if 0
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)
# define MCO_BACKEND MCO_KQUEUE
struct poll {
	uint64_t timeout;
	int kqueue;
}; 
#endif
#endif

#if defined(__linux__)
# define MCO_BACKEND MCO_EPOLL
struct poll {
	uint64_t timeout;
	int epoll;
};
#endif


#if !defined(MCO_BACKEND)
# include <poll.h>
# define MCO_BACKEND MCO_POLL
# define MCO_FD_MAX_SZ 1024
struct poll {
	uint64_t timeout;
	int nfds;
	struct pollfd fds[MCO_FD_MAX_SZ];
	int co_map[MCO_FD_MAX_SZ];
};
#endif

void mco_init_mpoll(mco_schedule *S);
void mco_poll(mco_schedule *S);
void mco_wait(mco_schedule *S, int fd, int flag);

#if defined (__cplusplus)
}	/*end of extern "C"*/
#endif

#endif /* end of include guard:  _MPOLL_H_JJAT0I5Y_ */

