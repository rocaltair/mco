#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "mco.h"
#include "fd.h"
#include "net.h"

int listen_port = 1234;
int target_port = 80;
const char * target_host = "115.239.210.27";

static void *mkcopy(mco_schedule *S, int r, int w)
{
	int *p = malloc(sizeof(*p));
	p[0] = r;
	p[1] = w;
	return p;
}

static void rwhandle(mco_schedule *S, void *p)
{
	int n;
	int *a = p;
	int r = a[0];
	int w = a[1];
	char buf[512];
	free(a);
	while((n = mco_read(S, r, buf, sizeof(buf)/sizeof(buf[0]))) > 0)
		mco_write(S, w, buf, n);
	shutdown(r, SHUT_RD);
	close(r);
	printf("disconnect\n");
}

void client_handle(mco_schedule *S, void *ud)
{
	int id1, id2;
	int peer1 = (int)(uintptr_t)ud;
	int peer2 = mco_dial(S, 1, target_host, target_port);
	printf("dial to %s:%d\n", target_host, target_port);
	if (peer2 < 0)
		return;
	id1 = mco_new(S, 0, rwhandle, mkcopy(S, peer1, peer2));
	id2 = mco_new(S, 0, rwhandle, mkcopy(S, peer2, peer1));
	mco_resume(S, id1);
	mco_resume(S, id2);
}

void listen_service(mco_schedule *S, void *ud)
{
	int cli_fd = 0;
	char ip[32];
	int port;
	int listen_fd = (int)(uintptr_t)ud;
	while ((cli_fd = mco_accept(S, listen_fd, ip, &port)) > 0) {
		int id = mco_new(S, 0, client_handle, (void *)(uintptr_t)cli_fd);
		printf("accept %s:%d\n", ip, port);
		mco_resume(S, id);
	}
}

void start_service(mco_schedule *S)
{
	int id;
	int listen_fd;
	printf("forward %d to %s:%d...\n", listen_port, target_host, target_port);
	listen_fd = mco_announce(S, 1, "0.0.0.0", listen_port);
	id = mco_new(S, 0, listen_service, (void *)(uintptr_t)listen_fd);
	mco_resume(S, id);
	mco_run(S, 0);
}

int main(int argc, char **argv)
{
	mco_schedule *S;
	if (argc < 4) {
		fprintf(stderr, "usage:\n\t%s <listen_port> <target_host> <target_port>\n", argv[0]);
		return 1;
	}
	listen_port = atoi(argv[1]);
	target_host = argv[2];
	target_port = atoi(argv[3]);

	S = mco_open(0);
	start_service(S);
	mco_close(S);
	return 0;
}

