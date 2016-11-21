#ifndef  _SOCKET_H_3FOKBHG5_
#define  _SOCKET_H_3FOKBHG5_

#if defined (__cplusplus)
extern "C" {
#endif

struct mco_schedule;

int mco_lookup(char *name, uint32_t *ip);

int mco_announce(struct mco_schedule *S, int istcp, char *server, int port);
int mco_accept(struct mco_schedule *S, int fd, char *server, int *port);
int mco_dial(struct mco_schedule *S, int istcp, char *server, int port);

#if defined (__cplusplus)
}	/*end of extern "C"*/
#endif

#endif /* end of include guard:  _SOCKET_H_3FOKBHG5_ */


