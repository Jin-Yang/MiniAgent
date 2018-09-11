#if EV_SIGNAL_ENABLE || EV_ASYNC_ENABLE
static void noinline ecb_cold evpipe_init (EV_P)
{
	if (ev_is_active(&pipe_w) == 0) {
		int fds [2];

# if EV_USE_EVENTFD
		fds[0] = -1;
		fds[1] = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
		if (fds[1] < 0 && errno == EINVAL)
			fds[1] = eventfd(0, 0);

		if (fds[1] < 0)
# endif
		{
			while (pipe(fds))
				ev_syserr("(libev) error creating signal/async pipe");

			fd_intern(fds[0]);
		}

		evpipe[0] = fds[0];
		if (evpipe[1] < 0) {
			evpipe[1] = fds[1]; /* first call, set write fd */
		} else {
			/*
			 * on subsequent calls, do not change evpipe [1]
			 * so that evpipe_write can always rely on its value.
			 * this branch does not do anything sensible on windows,
			 * so must not be executed on windows
			 */
			dup2(fds[1], evpipe[1]);
			close(fds[1]);
		}

		fd_intern(evpipe[1]);

		ev_io_set(&pipe_w, evpipe[0] < 0 ? evpipe[1] : evpipe[0], EV_READ);
		ev_io_start(EV_A_ &pipe_w);
		ev_unref(EV_A); /* watcher should not keep loop alive */
	}
}

inline_speed void evpipe_write(EV_P_ EV_ATOMIC_T *flag)
{
	/* push out the write before this function was called, acquire flag */
	ECB_MEMORY_FENCE;

	if (expect_true(*flag))
		return;

	*flag = 1;

	/* make sure flag is visible before the wakeup */
	ECB_MEMORY_FENCE_RELEASE;

	pipe_write_skipped = 1;

	/* make sure pipe_write_skipped is visible before we check pipe_write_wanted */
	ECB_MEMORY_FENCE;

	if (pipe_write_wanted) {
		int old_errno;

		pipe_write_skipped = 0;
		ECB_MEMORY_FENCE_RELEASE;

		/* save errno because write will clobber it */
		old_errno = errno;

#if EV_USE_EVENTFD
		if (evpipe[0] < 0) {
			uint64_t counter = 1;

			write(evpipe[1], &counter, sizeof(uint64_t));
		} else
#endif
		{
#ifdef _WIN32
			WSABUF buf;
			DWORD sent;
			buf.buf = &buf;
			buf.len = 1;
			WSASend(EV_FD_TO_WIN32_HANDLE(evpipe [1]), &buf, 1, &sent, 0, 0, 0);
#else
			write(evpipe[1], &(evpipe[1]), 1);
#endif
		}

		errno = old_errno;
	}
}

/* called whenever the libev signal pipe got some events (signal, async) */
static void pipecb (EV_P_ ev_io *iow, int revents)
{
	(void) iow;
	int i;

	if (revents & EV_READ) {
#if EV_USE_EVENTFD
		if (evpipe[0] < 0) {
			uint64_t counter;

			read(evpipe[1], &counter, sizeof(uint64_t));
		} else
#endif
		{
			char dummy[4];
#ifdef _WIN32
			WSABUF buf;
			DWORD recvd;
			DWORD flags = 0;
			buf.buf = dummy;
			buf.len = sizeof(dummy);
			WSARecv(EV_FD_TO_WIN32_HANDLE(evpipe [0]), &buf, 1, &recvd, &flags, 0, 0);
#else
			read(evpipe[0], &dummy, sizeof(dummy));
#endif
		}
	}

	pipe_write_skipped = 0;

	ECB_MEMORY_FENCE; /* push out skipped, acquire flags */

#if EV_SIGNAL_ENABLE
	if (sig_pending) {
		sig_pending = 0;

		ECB_MEMORY_FENCE;

		for (i = EV_NSIG - 1; i--; )
			if (expect_false(signals[i].pending))
				ev_feed_signal_event(EV_A_ i + 1);
	}
#endif

#if EV_ASYNC_ENABLE
	if (async_pending) {
		async_pending = 0;

		ECB_MEMORY_FENCE;

		for (i = asynccnt; i--; ) {
			if (asyncs [i]->sent) {
				asyncs [i]->sent = 0;
				ECB_MEMORY_FENCE_RELEASE;
				ev_feed_event (EV_A_ asyncs [i], EV_ASYNC);
			}
		}
	}
#endif
}

/*****************************************************************************/

void ev_feed_signal(int signum) EV_THROW
{
#if EV_MULTIPLICITY
	EV_P;
	ECB_MEMORY_FENCE_ACQUIRE;
	EV_A = signals[signum - 1].loop;

	if (!EV_A)
		return;
#endif

	signals[signum - 1].pending = 1;
	evpipe_write(EV_A_ &sig_pending);
}

static void ev_sighandler(int signum)
{
#ifdef _WIN32
	signal(signum, ev_sighandler);
#endif

	ev_feed_signal(signum);
}

void noinline ev_feed_signal_event(EV_P_ int signum) EV_THROW
{
	WL w;

	if (expect_false(signum <= 0 || signum >= EV_NSIG))
		return;

	--signum;

#if EV_MULTIPLICITY
	/*
	 * it is permissible to try to feed a signal to the wrong loop
	 * or, likely more useful, feeding a signal nobody is waiting for
	 */
	if (expect_false(signals[signum].loop != EV_A))
		return;
#endif

	signals[signum].pending = 0;
	ECB_MEMORY_FENCE_RELEASE;

	for (w = signals[signum].head; w; w = w->next)
		ev_feed_event(EV_A_ (W)w, EV_SIGNAL);
}

#if EV_USE_SIGNALFD
static void sigfdcb(EV_P_ ev_io *iow, int revents)
{
	struct signalfd_siginfo si[2], *sip; /* these structs are big */

	for (;;) {
		ssize_t res = read(sigfd, si, sizeof(si));

		/* not ISO-C, as res might be -1, but works with SuS */
		for (sip = si; (char *)sip < (char *)si + res; ++sip)
			ev_feed_signal_event(EV_A_ sip->ssi_signo);

		if (res < (ssize_t)sizeof(si))
			break;
	}
}
#endif
#endif
