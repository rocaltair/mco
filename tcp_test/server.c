#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include "mco.h"
#include "fd.h"
#include "net.h"

#define PORT 6688

struct remote_host {
	char ip[32];
	int port;
};

void client_handle(mco_schedule *S, void *ud)
{
	char buf[32];
	time_t t;
	int cli_fd = (int)(uintptr_t)ud;
	int n;
	time(&t);
	ctime_r(&t, buf);
	n = mco_write(S, cli_fd, buf, strlen(buf) + 1);
	close(cli_fd);
	// printf("disconnect, wrote=%d...\n", n);
}

void listen_service(mco_schedule *S, void *ud)
{
	int cli_fd = 0;
	struct remote_host host;
	int listen_fd = (int)(uintptr_t)ud;
	assert(listen_fd > 0);
	printf("start accept\n");
	while ((cli_fd = mco_accept(S, listen_fd, host.ip, &host.port)) > 0) {
		int id = mco_new(S, 0, client_handle, (void *)(uintptr_t)cli_fd);
		printf("accept %s:%d\n", host.ip, host.port);
		mco_resume_later(S, id);
	}
	printf("end accept\n");
}

void start_service(mco_schedule *S)
{
	int id;
	int listen_fd;
	printf("start listen on *:%d...\n", PORT);
	listen_fd = mco_announce(S, 1, "0.0.0.0", PORT);
	id = mco_new(S, 0, listen_service, (void *)(uintptr_t)listen_fd);
	assert(id >= 0);
	mco_resume(S, id);
	mco_run(S, 0);
}

int main(int argc, char **argv)
{
	mco_schedule *S = mco_open(0);
	start_service(S);
	mco_close(S);
	return 0;
}
