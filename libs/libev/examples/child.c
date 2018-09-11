
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "libev/ev.h"

ev_child cw;

static void child_cb (EV_P_ ev_child *w, int revents)
{
	ev_child_stop(EV_A_ w);
	printf ("process %d exited with status %x\n", w->rpid, w->rstatus);
}

int main (void)
{
	/* use the default event loop unless you have special needs */
        EV_P EV_DEFAULT; /* OR ev_default_loop(0) */

	pid_t pid = fork();
	if (pid < 0) {  /* error */
		perror("fork()");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {  /* child, the forked child executes here */
		sleep(1);
		exit(EXIT_SUCCESS);
	}

	/* parent */
	ev_child_init(&cw, child_cb, pid, 0);
	ev_child_start(EV_A_ &cw);

	/* now wait for events to arrive */
        ev_run(EV_A_ 0);

	return 0;
}
