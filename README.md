# mco

C-coroutine event framework based on ucontext

tested on Mac OSX(10.12) & CentOS 6.5 & RaspberryPi 2B+(armv6 32bit)

# Features

mco_yield & mco_resume

timer & mco_sleep in coroutine

epoll(linux)/kqueue(mac os/freebsd)/poll(cygwin) backend

# Examples

sleep

proxy

datetime

# Test

see test.sh

we use nc(netcat, telnet like) to connect proxy/forward,

which forward tcp connection to datetime service,

and datetime service will return datetime to nc peer over proxy/forward.

# TODO

signal handle

