
struct ev_once
{
	ev_io io;
	ev_timer to;
	void (*cb)(int revents, void *arg);
	void *arg;
};

static void once_cb (EV_P_ struct ev_once *once, int revents)
{
	void (*cb)(int revents, void *arg) = once->cb;
	void *arg = once->arg;

	ev_io_stop(EV_A_ &once->io);
	ev_timer_stop(EV_A_ &once->to);
	ev_free(once);

	cb(revents, arg);
}

static void once_cb_io(EV_P_ ev_io *w, int revents)
{
	struct ev_once *once;

	once = (struct ev_once *)(((char *)w) - offsetof(struct ev_once, io));
	once_cb(EV_A_ once, revents | ev_clear_pending(EV_A_ &once->to));
}

static void once_cb_to (EV_P_ ev_timer *w, int revents)
{
	struct ev_once *once;

	once = (struct ev_once *)(((char *)w) - offsetof(struct ev_once, to));
	once_cb(EV_A_ once, revents | ev_clear_pending(EV_A_ &once->io));
}

void ev_once(EV_P_ int fd, int events, ev_tstamp timeout, void (*cb)(int revents, void *arg), void *arg) EV_THROW
{
	struct ev_once *once;

	once = (struct ev_once *)ev_malloc(sizeof(struct ev_once));
	if (expect_false(once == NULL)) {
		cb(EV_ERROR | EV_READ | EV_WRITE | EV_TIMER, arg);
		return;
	}
	once->cb  = cb;
	once->arg = arg;

	ev_init(&once->io, once_cb_io);
	if (fd >= 0) {
		ev_io_set(&once->io, fd, events);
		ev_io_start(EV_A_ &once->io);
	}

	ev_init(&once->to, once_cb_to);
	if (timeout >= 0.) {
		ev_timer_set(&once->to, timeout, 0.);
		ev_timer_start(EV_A_ &once->to);
	}
}
