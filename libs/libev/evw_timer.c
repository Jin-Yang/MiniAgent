
void noinline ev_timer_start(EV_P_ ev_timer *w) EV_THROW
{
	if (expect_false(ev_is_active(w)))
		return;

	ev_at(w) += mn_now;
	assert(w->repeat >= 0.); /* ev_timer_start called with negative timer repeat value. */

	EV_FREQUENT_CHECK;

	++timercnt;
	ev_start(EV_A_ (W)w, timercnt + HEAP0 - 1);
	array_needsize(ANHE, timers, timermax, ev_active(w) + 1, EMPTY2);
	ANHE_w(timers[ev_active(w)]) = (WT)w;
	ANHE_at_cache(timers[ev_active(w)]);
	upheap(timers, ev_active(w));

	EV_FREQUENT_CHECK;

	/* internal timer heap corruption */
	assert(ANHE_w(timers[ev_active(w)]) == (WT)w);
}

void noinline ev_timer_stop(EV_P_ ev_timer *w) EV_THROW
{
	clear_pending(EV_A_ (W)w);
	if (expect_false (!ev_is_active (w)))
		return;

	EV_FREQUENT_CHECK;

	int active = ev_active (w);

	assert(ANHE_w(timers[active]) == (WT)w); /* internal timer heap corruption. */

	--timercnt;
	if (expect_true(active < timercnt + HEAP0)) {
		timers[active] = timers[timercnt + HEAP0];
		adjustheap(timers, timercnt, active);
	}

	ev_at(w) -= mn_now;

	ev_stop(EV_A_ (W)w);

	EV_FREQUENT_CHECK;
}

void noinline ev_timer_again(EV_P_ ev_timer *w) EV_THROW
{
	EV_FREQUENT_CHECK;

	clear_pending(EV_A_ (W)w);

	if (ev_is_active(w)) {
		if (w->repeat) {
			ev_at(w) = mn_now + w->repeat;
			ANHE_at_cache(timers [ev_active (w)]);
			adjustheap(timers, timercnt, ev_active (w));
		} else {
			ev_timer_stop (EV_A_ w);
		}
	} else if (w->repeat) {
		ev_at(w) = w->repeat;
		ev_timer_start(EV_A_ w);
	}

	EV_FREQUENT_CHECK;
}

ev_tstamp ev_timer_remaining(EV_P_ ev_timer *w) EV_THROW
{
	return ev_at(w) - (ev_is_active(w) ? mn_now : 0.);
}

