#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include "mco.h"

int mco_nblock(int fd)
{
	return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL)|O_NONBLOCK);
}

int mco_read(mco_schedule *S, int fd, void *buf, size_t n)
{
	int m;
	while((m=read(fd, buf, n)) < 0 && errno == EAGAIN)
		mco_wait(S, fd, 'r');
	return m;
}

int mco_write(mco_schedule *S, int fd, const void *buf, size_t n)
{
	int m, tot;
	for(tot=0; tot<n; tot+=m){
		while((m=write(fd, (char*)buf+tot, n-tot)) < 0 && errno == EAGAIN)
			mco_wait(S, fd, 'w');
		if(m < 0)
			return m;
		if(m == 0)
			break;
	}
	return tot;
}
