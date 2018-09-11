
#if EV_PERIODIC_ENABLE
void noinline ev_periodic_start(EV_P_ ev_periodic *w) EV_THROW
{
	if (expect_false(ev_is_active(w)))
		return;

	if (w->reschedule_cb) {
		ev_at(w) = w->reschedule_cb(w, ev_rt_now);
	} else if (w->interval) {
		assert(w->interval >= 0.); /* ev_periodic_start called with negative interval value. */
		periodic_recalc(EV_A_ w);
	} else {
		ev_at (w) = w->offset;
	}

	EV_FREQUENT_CHECK;

	++periodiccnt;
	ev_start(EV_A_ (W)w, periodiccnt + HEAP0 - 1);
	array_needsize(ANHE, periodics, periodicmax, ev_active (w) + 1, EMPTY2);
	ANHE_w(periodics [ev_active (w)]) = (WT)w;
	ANHE_at_cache(periodics [ev_active (w)]);
	upheap(periodics, ev_active (w));

	EV_FREQUENT_CHECK;

	assert(ANHE_w(periodics[ev_active(w)]) == (WT)w); /* internal periodic heap corruption. */
}

void noinline ev_periodic_stop(EV_P_ ev_periodic *w) EV_THROW
{
	clear_pending(EV_A_ (W)w);
	if (expect_false(!ev_is_active(w)))
		return;

	EV_FREQUENT_CHECK;

	int active = ev_active(w);

	assert(ANHE_w (periodics [active]) == (WT)w); /* internal periodic heap corruption. */

	--periodiccnt;

	if (expect_true(active < periodiccnt + HEAP0)) {
		periodics[active] = periodics[periodiccnt + HEAP0];
		adjustheap(periodics, periodiccnt, active);
	}

	ev_stop(EV_A_ (W)w);

	EV_FREQUENT_CHECK;
}

void noinline ev_periodic_again(EV_P_ ev_periodic *w) EV_THROW
{
	/* TODO: use adjustheap and recalculation */
	ev_periodic_stop(EV_A_ w);
	ev_periodic_start(EV_A_ w);
}
#endif

