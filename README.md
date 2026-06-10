# Matt_daemon

A UNIX daemon listening on TCP port 4242.

## Build

    make            # build ./Matt_daemon
    make re         # rebuild from scratch
    make fclean     # remove binary + objects

## Run

Requires root.

    sudo ./Matt_daemon

- Lock file: `/var/lock/matt_daemon.lock`
- Log file:  `/var/log/matt_daemon/matt_daemon.log`
  (format `[DD/MM/YYYY-HH:MM:SS] [ LEVEL ] - message`)
- Up to 3 concurrent clients
- Send `quit` over the socket, or any catchable signal, for clean shutdown

Convenience targets:

    make kill       # sudo kill -TERM $(pgrep Matt_daemon)
    make log        # tail -f the log file

## Test

    make test       # doctest unit + integration suite (no root needed)
    sudo ./test.sh  # end-to-end suite against a live daemon

## Layout

    include/   class headers
    src/       sources (main.cpp + one .cpp per class)
    tests/     doctest unit/integration tests + vendored doctest.h
    Makefile
    test.sh
