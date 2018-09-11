#if EV_CHILD_ENABLE
void ev_child_start(EV_P_ ev_child *w) EV_THROW
{
#if EV_MULTIPLICITY
	/* child watchers are only supported in the default loop */
	assert (loop == ev_default_loop_ptr);
#endif
	if (expect_false(ev_is_active(w)))
		return;

	EV_FREQUENT_CHECK;

	ev_start(EV_A_ (W)w, 1);
	wlist_add(&childs[w->pid & ((EV_PID_HASHSIZE) - 1)], (WL)w);

	EV_FREQUENT_CHECK;
}

void ev_child_stop(EV_P_ ev_child *w) EV_THROW
{
	clear_pending(EV_A_ (W)w);
	if (expect_false(!ev_is_active (w)))
		return;

	EV_FREQUENT_CHECK;

	wlist_del(&childs[w->pid & ((EV_PID_HASHSIZE) - 1)], (WL)w);
	ev_stop(EV_A_ (W)w);

	EV_FREQUENT_CHECK;
}
#endif
