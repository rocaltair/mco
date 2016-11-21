#if (MCO_BACKEND == MCO_KQUEUE)

void mco_init_mpoll(mco_schedule *S)
{
}

void mco_poll(mco_schedule *S)
{
}

void mco_wait(mco_schedule *S, int fd, int flag)
{
}

#endif
