
void noinline ev_io_start (EV_P_ ev_io *w) EV_THROW
{
	int fd = w->fd;

	if (expect_false(ev_is_active (w)))
		return;

	/* libev: ev_io_start called with negative fd. */
	assert(fd >= 0);
	/* libev: ev_io_start called with illegal event mask. */
	assert (!(w->events & ~(EV__IOFDSET | EV_READ | EV_WRITE)));

	EV_FREQUENT_CHECK;

	ev_start(EV_A_ (W)w, 1);
	array_needsize(ANFD, anfds, anfdmax, fd + 1, array_init_zero);
	wlist_add(&anfds[fd].head, (WL)w);

	/* libev: ev_io_start called with corrupted watcher. */
	assert(((WL)w)->next != (WL)w);

	fd_change(EV_A_ fd, (w->events & EV__IOFDSET) | EV_ANFD_REIFY);
	w->events &= ~EV__IOFDSET;

	EV_FREQUENT_CHECK;
}

void noinline ev_io_stop(EV_P_ ev_io *w) EV_THROW
{
	clear_pending(EV_A_ (W)w);
	if (expect_false(!ev_is_active(w)))
		return;

	/* libev: ev_io_stop called with illegal fd (must stay constant after start!) */
	assert (w->fd >= 0 && w->fd < anfdmax);

	EV_FREQUENT_CHECK;

	wlist_del(&anfds[w->fd].head, (WL)w);
	ev_stop (EV_A_ (W)w);

	fd_change(EV_A_ w->fd, EV_ANFD_REIFY);

	EV_FREQUENT_CHECK;
}
