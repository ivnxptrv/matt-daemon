#!/usr/bin/env bash
# Matt_daemon end-to-end test suite.
# Self-elevates via sudo. Drives the daemon over real TCP sockets
# (bash /dev/tcp — no nc dependency) and validates via log + lock + ps state.
#
# Usage:   ./test.sh
# Returns: exit 0 if all pass, 1 otherwise.

set -u

BIN=${BIN:-./Matt_daemon}
LOCK=/var/lock/matt_daemon.lock
LOG=/var/log/matt_daemon/matt_daemon.log
PORT=4242

if [[ $EUID -ne 0 ]]; then
    exec sudo --preserve-env=BIN "$0" "$@"
fi

PASS=0
FAIL=0
pass() { echo "  [PASS] $*"; PASS=$((PASS + 1)); }
fail() { echo "  [FAIL] $*"; FAIL=$((FAIL + 1)); }
note() { echo; echo "── $*"; }

#── helpers ────────────────────────────────────────────────────────────────

reset() {
    pkill -KILL -x Matt_daemon 2>/dev/null || true
    sleep 0.2
    rm -f "$LOCK"
    mkdir -p "$(dirname "$LOG")"
    : > "$LOG"
}

# Wait until "started. PID:" appears in the log — most reliable readiness
# signal, since it's the last line the grandchild writes during startup.
wait_for_ready() {
    for _ in $(seq 1 40); do
        grep -q "started. PID:" "$LOG" 2>/dev/null && return 0
        sleep 0.1
    done
    return 1
}

wait_for_no_lock() {
    for _ in $(seq 1 40); do
        [[ ! -e "$LOCK" ]] && return 0
        sleep 0.1
    done
    return 1
}

start_daemon() {
    "$BIN"
    wait_for_ready
}

# Open a TCP connection on fd 3, send one line, close.
send_line() {
    exec 3<>/dev/tcp/localhost/$PORT
    printf '%s\n' "$1" >&3
    sleep 0.2
    exec 3>&-
}

send_quit() {
    send_line "quit"
    wait_for_no_lock
}

#── tests ──────────────────────────────────────────────────────────────────

test_non_root_rejection() {
    note "Non-root invocation is rejected with stderr message"
    local out
    out=$(sudo -u nobody "$BIN" 2>&1) || true
    [[ "$out" == *"must run as root"* ]] \
        && pass "stderr contains 'must run as root'" \
        || fail "expected stderr 'must run as root', got: $out"
}

test_normal_startup() {
    note "Daemon starts, acquires lock, writes canonical log lines"
    reset
    start_daemon || { fail "daemon did not become ready"; return; }

    pgrep -x Matt_daemon >/dev/null \
        && pass "process alive" || fail "no process"
    [[ -e "$LOCK" ]] \
        && pass "lock file present at $LOCK" || fail "no lock at $LOCK"
    [[ -e "$LOG"  ]] \
        && pass "log file present at $LOG" || fail "no log at $LOG"

    local line
    for line in "Matt_daemon: Started." \
                "Matt_daemon: Creating server." \
                "Matt_daemon: Server created." \
                "Matt_daemon: Entering Daemon mode." \
                "Matt_daemon: started. PID:"; do
        grep -F "$line" "$LOG" >/dev/null \
            && pass "log line: '$line'" \
            || fail "log missing: '$line'"
    done

    if head -n1 "$LOG" \
        | grep -qE '^\[[0-9]{2}/[0-9]{2}/[0-9]{4}-[0-9]{2}:[0-9]{2}:[0-9]{2}\]'
    then
        pass "timestamp format [DD/MM/YYYY-HH:MM:SS]"
    else
        fail "timestamp wrong: $(head -n1 "$LOG")"
    fi

    if head -n1 "$LOG" | grep -qE '\[ INFO \]'; then
        pass "level format '[ INFO ]' with spaces"
    else
        fail "level format wrong: $(head -n1 "$LOG")"
    fi
}

test_second_instance_rejection() {
    note "Second instance prints lock error to stderr"
    local out
    out=$("$BIN" 2>&1) || true
    [[ "$out" == *"Can't open :$LOCK"* ]] \
        && pass "stderr contains 'Can't open :$LOCK'" \
        || fail "expected lock error, got: $out"
}

test_user_input_logged() {
    note "Arbitrary input is logged as 'User input:' at LOG level"
    send_line "Hello"
    send_line "xd"
    sleep 0.2
    grep -F "Matt_daemon: User input: Hello" "$LOG" >/dev/null \
        && pass "'Hello' logged" || fail "'Hello' not logged"
    grep -F "Matt_daemon: User input: xd" "$LOG" >/dev/null \
        && pass "'xd' logged" || fail "'xd' not logged"
    grep -F "[ LOG ]" "$LOG" >/dev/null \
        && pass "LOG level present" || fail "no LOG-level entries"
}

test_quit_shuts_down() {
    note "Sending 'quit' triggers clean shutdown"
    send_quit \
        && pass "lock released after quit" \
        || fail "lock not released"
    grep -F "Matt_daemon: Request quit." "$LOG" >/dev/null \
        && pass "'Request quit.' logged" || fail "'Request quit.' missing"
    grep -F "Matt_daemon: Quitting." "$LOG" >/dev/null \
        && pass "'Quitting.' logged" || fail "'Quitting.' missing"
    pgrep -x Matt_daemon >/dev/null \
        && fail "process still alive" || pass "process exited"
}

test_signal_shuts_down() {
    note "SIGTERM triggers clean shutdown via signalfd path"
    reset
    start_daemon || { fail "daemon did not start"; return; }
    local pid
    pid=$(pgrep -x Matt_daemon)
    kill -TERM "$pid"
    wait_for_no_lock \
        && pass "lock released after SIGTERM" \
        || fail "lock not released"
    grep -F "Matt_daemon: Signal handler." "$LOG" >/dev/null \
        && pass "'Signal handler.' logged" || fail "missing"
    grep -F "Matt_daemon: Quitting." "$LOG" >/dev/null \
        && pass "'Quitting.' logged" || fail "missing"
}

test_restartable_after_clean_exit() {
    note "Daemon can restart after a clean shutdown (lock truly released)"
    reset
    start_daemon || { fail "first start failed"; return; }
    send_quit    || { fail "shutdown failed";   return; }
    start_daemon \
        && pass "second start succeeded" \
        || fail "could not restart"
}

test_3_client_cap() {
    note "4th simultaneous client is rejected (input dropped)"
    reset
    start_daemon || { fail "daemon did not start"; return; }

    # Hold three slots open.
    exec 4<>/dev/tcp/localhost/$PORT
    exec 5<>/dev/tcp/localhost/$PORT
    exec 6<>/dev/tcp/localhost/$PORT
    sleep 0.3

    # Verify the three are real clients.
    printf 'FROM_CLIENT_1\n' >&4
    sleep 0.2
    grep -F "FROM_CLIENT_1" "$LOG" >/dev/null \
        && pass "client 1 input logged" || fail "client 1 input lost"

    # 4th attempt: daemon should accept-then-close immediately.
    exec 7<>/dev/tcp/localhost/$PORT
    printf 'SHOULD_BE_DROPPED\n' >&7 2>/dev/null || true
    sleep 0.3

    if grep -q "SHOULD_BE_DROPPED" "$LOG"; then
        fail "4th client input was logged (cap not enforced)"
    else
        pass "4th client input dropped"
    fi

    exec 4>&- 5>&- 6>&- 7>&- 2>/dev/null || true
    sleep 0.2
    send_quit >/dev/null 2>&1 || true
}


#── runner ─────────────────────────────────────────────────────────────────

cleanup_on_exit() {
    pkill -KILL -x Matt_daemon 2>/dev/null || true
    rm -f "$LOCK"
}
trap cleanup_on_exit EXIT

echo "Matt_daemon E2E test suite"

test_non_root_rejection
test_normal_startup
test_second_instance_rejection
test_user_input_logged
test_quit_shuts_down
test_signal_shuts_down
test_restartable_after_clean_exit
test_3_client_cap
test_fragmented_quit

echo
echo "──────────────────────────"
echo "  PASS: $PASS    FAIL: $FAIL"
if [[ $FAIL -gt 0 ]]; then
    echo
    echo "── last 20 log lines for context ──"
    tail -n 20 "$LOG" 2>/dev/null || true
    exit 1
fi
exit 0
