#include "ev.h"

#if EV_FORK_ENABLE
void ev_fork_start(EV_P_ ev_fork *w) EV_THROW
{
	if (expect_false(ev_is_active(w)))
		return;

	EV_FREQUENT_CHECK;

	ev_start(EV_A_ (W)w, ++forkcnt);
	array_needsize(ev_fork *, forks, forkmax, forkcnt, EMPTY2);
	forks[forkcnt - 1] = w;

	EV_FREQUENT_CHECK;
}

void ev_fork_stop(EV_P_ ev_fork *w) EV_THROW
{
	int active;

	clear_pending(EV_A_ (W)w);
	if (expect_false(!ev_is_active(w)))
		return;

	EV_FREQUENT_CHECK;

	active = ev_active(w);
	forks[active - 1] = forks[--forkcnt];
	ev_active(forks[active - 1]) = active;

	ev_stop(EV_A_ (W)w);

	EV_FREQUENT_CHECK;
}
#endif
