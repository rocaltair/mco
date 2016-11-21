#ifndef  _FD_H_DK0M30X7_
#define  _FD_H_DK0M30X7_

#if defined (__cplusplus)
extern "C" {
#endif

struct mco_schedule;
int mco_nblock(int fd);
int mco_read(struct mco_schedule *S, int fd, void *buf, size_t n);
int mco_write(struct mco_schedule *S, int fd, void *buf, size_t n);

#if defined (__cplusplus)
}	/*end of extern "C"*/
#endif

#endif /* end of include guard:  _FD_H_DK0M30X7_ */


