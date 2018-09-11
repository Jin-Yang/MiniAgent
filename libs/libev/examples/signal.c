/* catch a SIGINT signal, ctrl-c */
#include "../ev.h"
#include <stdio.h>
#include <signal.h>

ev_signal signal_watcher;

static void sigint_cb(EV_P_ ev_signal *w, int revents)
{
	(void) w;
	(void) revents;

	puts("catch SIGINT");
	ev_break(EV_A_ EVBREAK_ALL);
}

int main (void)
{
	// use the default event loop unless you have special needs
        EV_P EV_DEFAULT; /* OR ev_default_loop(0) */

	ev_signal_init(&signal_watcher, sigint_cb, SIGINT);
	ev_signal_start(EV_A_ &signal_watcher);

	// now wait for events to arrive
	ev_run(EV_A_ 0);

	// break was called, so exit
	return 0;
}
