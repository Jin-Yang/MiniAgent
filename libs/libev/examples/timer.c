#include <time.h>
#include <stdio.h>
#include <stdint.h>

#include "libev/ev.h"

ev_timer timeout_watcher;
ev_timer repeate_watcher;
ev_timer oneshot_watcher;

// another callback, this time for a time-out
static void timeout_cb (EV_P_ ev_timer *w, int revents)
{
        (void) w;
        (void) revents;
        printf("timeout at %ju\n", (uintmax_t)time(NULL));

        /* this causes the innermost ev_run to stop iterating */
        ev_break (EV_A_ EVBREAK_ONE);
}
static void repeate_cb (EV_P_ ev_timer *w, int revents)
{
        (void) w;
        (void) revents;
        printf("repeate at %ju\n", (uintmax_t)time(NULL));
}
static void oneshot_cb (EV_P_ ev_timer *w, int revents)
{
        (void) w;
        (void) revents;
        printf("oneshot at %ju\n", (uintmax_t)time(NULL));
        ev_timer_stop(EV_A_ w);
}

int main (void)
{
        time_t result;
        EV_P EV_DEFAULT; /* OR ev_default_loop(0) */

        result = time(NULL);
        printf("  start at %ju\n", (uintmax_t)result);

        /* run only once in 2s later */
        ev_timer_init(&oneshot_watcher, oneshot_cb, 2.0, 0.);
        ev_timer_start(EV_A_ &oneshot_watcher);

        /* run in 5 seconds later, and repeat every second */
        ev_timer_init(&repeate_watcher, repeate_cb, 5., 1.);
        ev_timer_start(EV_A_ &repeate_watcher);

        /* timeout in 10s later, and also quit. */
        ev_timer_init(&timeout_watcher, timeout_cb, 10., 0.);
        ev_timer_start(EV_A_ &timeout_watcher);

        /* now wait for events to arrive. */
        ev_run(EV_A_ 0);

        return 0;
}
