/* dummy callback for pending events */
static void noinline pendingcb(EV_P_ ev_prepare *w, int revents)
{
	(void)w;
	(void)revents;
}

void noinline ev_feed_event(EV_P_ void *w, int revents) EV_THROW
{
	W w_ = (W)w;
	int pri = ABSPRI(w_);

	if (expect_false(w_->pending)) {
		pendings[pri][w_->pending - 1].events |= revents;
	} else {
		w_->pending = ++pendingcnt[pri];
		array_needsize(ANPENDING, pendings[pri], pendingmax[pri], w_->pending, EMPTY2);
		pendings[pri][w_->pending - 1].w      = w_;
		pendings[pri][w_->pending - 1].events = revents;
	}

	pendingpri = NUMPRI - 1;
}

inline_speed void feed_reverse(EV_P_ W w)
{
	array_needsize(W, rfeeds, rfeedmax, rfeedcnt + 1, EMPTY2);
	rfeeds[rfeedcnt++] = w;
}

inline_size void feed_reverse_done(EV_P_ int revents)
{
	do
		ev_feed_event(EV_A_ rfeeds [--rfeedcnt], revents);
	while (rfeedcnt);
}

inline_speed void queue_events(EV_P_ W *events, int eventcnt, int type)
{
	int i;

	for (i = 0; i < eventcnt; ++i)
		ev_feed_event (EV_A_ events [i], type);
}

/*****************************************************************************/

inline_speed void fd_event_nocheck(EV_P_ int fd, int revents)
{
	ANFD *anfd = anfds + fd;
	ev_io *w;

	for (w = (ev_io *)anfd->head; w; w = (ev_io *)((WL)w)->next) {
		int ev = w->events & revents;

		if (ev)
			ev_feed_event(EV_A_ (W)w, ev);
	}
}

/*
 * do not submit kernel events for fds that have reify set because that means
 * they changed while we were polling for new events
 */
inline_speed void fd_event(EV_P_ int fd, int revents)
{
	ANFD *anfd = anfds + fd;

	if (expect_true(anfd->reify == 0))
		fd_event_nocheck(EV_A_ fd, revents);
}

void ev_feed_fd_event(EV_P_ int fd, int revents) EV_THROW
{
	if (fd >= 0 && fd < anfdmax)
		fd_event_nocheck(EV_A_ fd, revents);
}

/*
 * make sure the external fd watch events are in-sync with the
 * kernel/libev internal state
 */
inline_size void fd_reify(EV_P)
{
	int i;

#if EV_SELECT_IS_WINSOCKET || EV_USE_IOCP
	for (i = 0; i < fdchangecnt; ++i) {
		int fd = fdchanges[i];
		ANFD *anfd = anfds + fd;

		if (anfd->reify & EV__IOFDSET && anfd->head) {
			SOCKET handle = EV_FD_TO_WIN32_HANDLE(fd);

			if (handle != anfd->handle) {
				unsigned long arg;

				/* only socket fds supported in this configuration */
				assert(ioctlsocket(handle, FIONREAD, &arg) == 0);

				/* handle changed, but fd didn't - we need to do it in two steps */
				backend_modify(EV_A_ fd, anfd->events, 0);
				anfd->events = 0;
				anfd->handle = handle;
			}
		}
	}
#endif

	for (i = 0; i < fdchangecnt; ++i) {
		int fd = fdchanges[i];
		ANFD *anfd = anfds + fd;
		ev_io *w;

		unsigned char o_events = anfd->events;
		unsigned char o_reify  = anfd->reify;

		anfd->reify  = 0;

		{ /*if (expect_true (o_reify & EV_ANFD_REIFY)) probably a deoptimisation */
			anfd->events = 0;

			for (w = (ev_io *)anfd->head; w; w = (ev_io *)((WL)w)->next)
				anfd->events |= (unsigned char)w->events;

			if (o_events != anfd->events)
				o_reify = EV__IOFDSET; /* actually |= */
		}

		if (o_reify & EV__IOFDSET)
			backend_modify (EV_A_ fd, o_events, anfd->events);
	}

	fdchangecnt = 0;
}

/* something about the given fd changed */
inline_size void fd_change(EV_P_ int fd, int flags)
{
	unsigned char reify = anfds [fd].reify;

	anfds [fd].reify |= flags;

	if (expect_true(reify == 0)) {
		++fdchangecnt;
		array_needsize(int, fdchanges, fdchangemax, fdchangecnt, EMPTY2);
		fdchanges[fdchangecnt - 1] = fd;
	}
}

/* the given fd is invalid/unusable, so make sure it doesn't hurt us anymore */
inline_speed void ecb_cold fd_kill(EV_P_ int fd)
{
	ev_io *w;

	while ((w = (ev_io *)anfds[fd].head)) {
		ev_io_stop(EV_A_ w);
		ev_feed_event(EV_A_ (W)w, EV_ERROR | EV_READ | EV_WRITE);
	}
}

/* check whether the given fd is actually valid, for error recovery */
inline_size int ecb_cold fd_valid(int fd)
{
#ifdef _WIN32
	return EV_FD_TO_WIN32_HANDLE(fd) != -1;
#else
	return fcntl(fd, F_GETFD) != -1;
#endif
}

#if 0
/* called on EBADF to verify fds */
static void noinline ecb_cold fd_ebadf (EV_P)
{
  int fd;

  for (fd = 0; fd < anfdmax; ++fd)
    if (anfds [fd].events)
      if (!fd_valid (fd) && errno == EBADF)
	fd_kill (EV_A_ fd);
}

/* called on ENOMEM in select/poll to kill some fds and retry */
static void noinline ecb_cold fd_enomem (EV_P)
{
  int fd;

  for (fd = anfdmax; fd--; )
    if (anfds [fd].events)
      {
	fd_kill (EV_A_ fd);
	break;
      }
}
#endif

/* usually called after fork if backend needs to re-arm all fds from scratch */
static void noinline fd_rearm_all(EV_P)
{
	int fd;

	for (fd = 0; fd < anfdmax; ++fd) {
		if (anfds [fd].events) {
			anfds[fd].events = 0;
			anfds[fd].emask  = 0;
			fd_change(EV_A_ fd, EV__IOFDSET | EV_ANFD_REIFY);
		}
	}
}

/* used to prepare libev internal fd's this is not fork-safe */
inline_speed void fd_intern(int fd)
{
#ifdef _WIN32
	unsigned long arg = 1;

	ioctlsocket(EV_FD_TO_WIN32_HANDLE (fd), FIONBIO, &arg);
#else
	fcntl(fd, F_SETFD, FD_CLOEXEC);
	fcntl(fd, F_SETFL, O_NONBLOCK);
#endif
}
