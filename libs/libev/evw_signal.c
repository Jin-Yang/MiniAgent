
#if EV_SIGNAL_ENABLE
void noinline ev_signal_start (EV_P_ ev_signal *w) EV_THROW
{
	if (expect_false(ev_is_active(w)))
		return;

	/* illegal signal number */
	assert(w->signum > 0 && w->signum < EV_NSIG);

#if EV_MULTIPLICITY
	/* a signal must not be attached to two different loops */
	assert(!signals[w->signum - 1].loop || signals[w->signum - 1].loop == loop);

	signals[w->signum - 1].loop = EV_A;
	ECB_MEMORY_FENCE_RELEASE;
#endif

	EV_FREQUENT_CHECK;

#if EV_USE_SIGNALFD
	if (sigfd == -2) {
		sigfd = signalfd(-1, &sigfd_set, SFD_NONBLOCK | SFD_CLOEXEC);
		if (sigfd < 0 && errno == EINVAL)
			sigfd = signalfd(-1, &sigfd_set, 0); /* retry without flags */
		if (sigfd >= 0) {
			fd_intern(sigfd); /* doing it twice will not hurt */

			sigemptyset(&sigfd_set);

			ev_io_init(&sigfd_w, sigfdcb, sigfd, EV_READ);
			ev_set_priority(&sigfd_w, EV_MAXPRI);
			ev_io_start(EV_A_ &sigfd_w);
			ev_unref(EV_A); /* signalfd watcher should not keep loop alive */
		}
	}

	if (sigfd >= 0) { /* TODO: check .head */
		sigaddset(&sigfd_set, w->signum);
		sigprocmask(SIG_BLOCK, &sigfd_set, 0);

		signalfd(sigfd, &sigfd_set, 0);
	}
#endif

	ev_start(EV_A_ (W)w, 1);
	wlist_add(&signals[w->signum - 1].head, (WL)w);

	if (!((WL)w)->next)
# if EV_USE_SIGNALFD
		if (sigfd < 0) /*TODO*/
# endif
		{
# ifdef _WIN32
		evpipe_init(EV_A);

		signal(w->signum, ev_sighandler);
# else
		struct sigaction sa;

		evpipe_init(EV_A);

		sa.sa_handler = ev_sighandler;
		sigfillset(&sa.sa_mask);
		sa.sa_flags = SA_RESTART; /* if restarting works we save one iteration */
		sigaction(w->signum, &sa, 0);

		if (origflags & EVFLAG_NOSIGMASK) {
			sigemptyset(&sa.sa_mask);
			sigaddset(&sa.sa_mask, w->signum);
			sigprocmask(SIG_UNBLOCK, &sa.sa_mask, 0);
		}
#endif
	}

	EV_FREQUENT_CHECK;
}

void noinline ev_signal_stop(EV_P_ ev_signal *w) EV_THROW
{
	clear_pending(EV_A_ (W)w);
	if (expect_false(!ev_is_active (w)))
		return;

	EV_FREQUENT_CHECK;

	wlist_del(&signals [w->signum - 1].head, (WL)w);
	ev_stop(EV_A_ (W)w);

	if (!signals[w->signum - 1].head) {
#if EV_MULTIPLICITY
		signals[w->signum - 1].loop = 0; /* unattach from signal */
#endif
#if EV_USE_SIGNALFD
		if (sigfd >= 0) {
			sigset_t ss;

			sigemptyset (&ss);
			sigaddset (&ss, w->signum);
			sigdelset (&sigfd_set, w->signum);

			signalfd (sigfd, &sigfd_set, 0);
			sigprocmask (SIG_UNBLOCK, &ss, 0);
		} else
#endif
			signal(w->signum, SIG_DFL);
	}

	EV_FREQUENT_CHECK;
}
#endif

