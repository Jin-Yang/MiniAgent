#if EV_USE_IOCP
	#include "ev_iocp.c"
#endif
#if EV_USE_PORT
	#include "ev_port.c"
#endif
#if EV_USE_KQUEUE
	#include "ev_kqueue.c"
#endif
#if EV_USE_EPOLL
	#include "ev_epoll.c"
#endif
#if EV_USE_POLL
	#include "ev_poll.c"
#endif
#if EV_USE_SELECT
	#include "ev_select.c"
#endif

/* return true if we are running with elevated privileges and should ignore env variables */
inline_size int ecb_cold enable_secure (void)
{
#ifdef _WIN32
	return 0;
#else
	return getuid () != geteuid () || getgid () != getegid ();
#endif
}

unsigned int ecb_cold ev_supported_backends(void) EV_THROW
{
	unsigned int flags = 0;

	if (EV_USE_PORT)
		flags |= EVBACKEND_PORT;
	if (EV_USE_KQUEUE)
		flags |= EVBACKEND_KQUEUE;
	if (EV_USE_EPOLL)
		flags |= EVBACKEND_EPOLL;
	if (EV_USE_POLL)
		flags |= EVBACKEND_POLL;
	if (EV_USE_SELECT)
		flags |= EVBACKEND_SELECT;

	return flags;
}

unsigned int ecb_cold ev_recommended_backends(void) EV_THROW
{
	unsigned int flags;

	flags = ev_supported_backends();

#ifndef __NetBSD__
	/* kqueue is borked on everything but netbsd apparently */
	/* it usually doesn't work correctly on anything but sockets and pipes */
	flags &= ~EVBACKEND_KQUEUE;
#endif
#ifdef __APPLE__
	/* only select works correctly on that "unix-certified" platform */
	flags &= ~EVBACKEND_KQUEUE; /* horribly broken, even for sockets */
	flags &= ~EVBACKEND_POLL;   /* poll is based on kqueue from 10.5 onwards */
#endif
#ifdef __FreeBSD__
	flags &= ~EVBACKEND_POLL;   /* poll return value is unusable (http://forums.freebsd.org/archive/index.php/t-10270.html) */
#endif

	return flags;
}

unsigned int ecb_cold ev_embeddable_backends(void) EV_THROW
{
	int flags = EVBACKEND_EPOLL | EVBACKEND_KQUEUE | EVBACKEND_PORT;

	/* epoll embeddability broken on all linux versions up to at least 2.6.23 */
	if (ev_linux_version() < 0x020620) /* disable it on linux < 2.6.32 */
		flags &= ~EVBACKEND_EPOLL;

	return flags;
}

/* initialise a loop structure, must be zero-initialised */
static void noinline ecb_cold loop_init(EV_P_ unsigned int flags) EV_THROW
{
	if (backend == 0) {
		origflags = flags;

#if EV_USE_REALTIME
		if (have_realtime == 0) {
			struct timespec ts;

			if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
				have_realtime = 1;
		}
#endif

#if EV_USE_MONOTONIC
		if (have_monotonic == 0) {
			struct timespec ts;

			if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
				have_monotonic = 1;
		}
#endif

#ifndef _WIN32
		/* pid check not overridable via env */
		if (flags & EVFLAG_FORKCHECK)
			curpid = getpid();
#endif

		if (!(flags & EVFLAG_NOENV) && !enable_secure() && getenv("LIBEV_FLAGS"))
			flags = atoi(getenv("LIBEV_FLAGS"));

		ev_rt_now       = ev_time();
		mn_now          = get_clock();
		now_floor       = mn_now;
		rtmn_diff       = ev_rt_now - mn_now;
#if EV_FEATURE_API
		invoke_cb       = ev_invoke_pending;
#endif

		io_blocktime       = 0.;
		timeout_blocktime  = 0.;
		backend            = 0;
		backend_fd         = -1;
		sig_pending        = 0;
#if EV_ASYNC_ENABLE
		async_pending      = 0;
#endif
		pipe_write_skipped = 0;
		pipe_write_wanted  = 0;
		evpipe[0]          = -1;
		evpipe[1]          = -1;
#if EV_USE_INOTIFY
		fs_fd = flags & EVFLAG_NOINOTIFY ? -1 : -2;
#endif
#if EV_USE_SIGNALFD
		sigfd = flags & EVFLAG_SIGNALFD ? -2 : -1;
#endif

		if ((flags & EVBACKEND_MASK) == 0)
			flags |= ev_recommended_backends ();

#if EV_USE_IOCP
		if (backend == 0 && (flags & EVBACKEND_IOCP))
			backend = iocp_init(EV_A_ flags);
#endif
#if EV_USE_PORT
		if (backend == 0 && (flags & EVBACKEND_PORT))
			backend = port_init(EV_A_ flags);
#endif
#if EV_USE_KQUEUE
		if (backend == 0 && (flags & EVBACKEND_KQUEUE))
			backend = kqueue_init(EV_A_ flags);
#endif
#if EV_USE_EPOLL
		if (backend == 0 && (flags & EVBACKEND_EPOLL))
			backend = epoll_init(EV_A_ flags);
#endif
#if EV_USE_POLL
		if (backend == 0 && (flags & EVBACKEND_POLL))
			backend = poll_init(EV_A_ flags);
#endif
#if EV_USE_SELECT
		if (backend == 0 && (flags & EVBACKEND_SELECT))
			backend = select_init(EV_A_ flags);
#endif

		ev_prepare_init(&pending_w, pendingcb);

#if EV_SIGNAL_ENABLE || EV_ASYNC_ENABLE
		ev_init(&pipe_w, pipecb);
		ev_set_priority(&pipe_w, EV_MAXPRI);
#endif
	}
}

/* free up a loop structure */
void ecb_cold ev_loop_destroy(EV_P)
{
	int i;

#if EV_MULTIPLICITY
	if (EV_A == NULL)
		return;
#endif

#if EV_CLEANUP_ENABLE
	/* queue cleanup watchers (and execute them) */
	if (expect_false(cleanupcnt)) {
		queue_events(EV_A_ (W *)cleanups, cleanupcnt, EV_CLEANUP);
		EV_INVOKE_PENDING;
	}
#endif

#if EV_CHILD_ENABLE
	if (ev_is_default_loop(EV_A) && ev_is_active(&childev)) {
		ev_ref(EV_A); /* child watcher */
		ev_signal_stop(EV_A_ &childev);
	}
#endif

	if (ev_is_active(&pipe_w)) {
		/*ev_ref (EV_A);*/
		/*ev_io_stop (EV_A_ &pipe_w);*/
		if (evpipe [0] >= 0)
			EV_CLOSE_FD(evpipe[0]);
		if (evpipe [1] >= 0)
			EV_CLOSE_FD(evpipe[1]);
	}

#if EV_USE_SIGNALFD
	if (ev_is_active(&sigfd_w))
		close(sigfd);
#endif

#if EV_USE_INOTIFY
	if (fs_fd >= 0)
		close(fs_fd);
#endif

	if (backend_fd >= 0)
		close(backend_fd);

#if EV_USE_IOCP
	if (backend == EVBACKEND_IOCP)
		iocp_destroy(EV_A);
#endif
#if EV_USE_PORT
	if (backend == EVBACKEND_PORT)
		port_destroy(EV_A);
#endif
#if EV_USE_KQUEUE
	if (backend == EVBACKEND_KQUEUE)
		kqueue_destroy(EV_A);
#endif
#if EV_USE_EPOLL
	if (backend == EVBACKEND_EPOLL)
		epoll_destroy(EV_A);
#endif
#if EV_USE_POLL
	if (backend == EVBACKEND_POLL)
		poll_destroy(EV_A);
#endif
#if EV_USE_SELECT
	if (backend == EVBACKEND_SELECT)
		select_destroy(EV_A);
#endif

	for (i = NUMPRI; i--; ) {
		array_free(pending, [i]);
#if EV_IDLE_ENABLE
		array_free(idle, [i]);
#endif
	}

	ev_free(anfds);
	anfds = 0;
	anfdmax = 0;

	/* have to use the microsoft-never-gets-it-right macro */
	array_free(rfeed, EMPTY);
	array_free(fdchange, EMPTY);
	array_free(timer, EMPTY);
#if EV_PERIODIC_ENABLE
	array_free(periodic, EMPTY);
#endif
#if EV_FORK_ENABLE
	array_free(fork, EMPTY);
#endif
#if EV_CLEANUP_ENABLE
	array_free(cleanup, EMPTY);
#endif
	array_free(prepare, EMPTY);
	array_free(check, EMPTY);
#if EV_ASYNC_ENABLE
	array_free(async, EMPTY);
#endif

	backend = 0;

#if EV_MULTIPLICITY
	if (ev_is_default_loop(EV_A))
#endif
		ev_default_loop_ptr = 0;
#if EV_MULTIPLICITY
	else
		ev_free(EV_A);
#endif
}

#if EV_USE_INOTIFY
inline_size void infy_fork(EV_P);
#endif

inline_size void loop_fork(EV_P)
{
#if EV_USE_PORT
	if (backend == EVBACKEND_PORT  )
		port_fork(EV_A);
#endif
#if EV_USE_KQUEUE
	if (backend == EVBACKEND_KQUEUE)
		kqueue_fork(EV_A);
#endif
#if EV_USE_EPOLL
	if (backend == EVBACKEND_EPOLL )
		epoll_fork(EV_A);
#endif
#if EV_USE_INOTIFY
	infy_fork(EV_A);
#endif

#if EV_SIGNAL_ENABLE || EV_ASYNC_ENABLE
	if (ev_is_active (&pipe_w) && postfork != 2) {
		/* pipe_write_wanted must be false now, so modifying fd vars should be safe */
		ev_ref(EV_A);
		ev_io_stop(EV_A_ &pipe_w);

		if (evpipe[0] >= 0)
			EV_CLOSE_FD(evpipe [0]);

		evpipe_init(EV_A);
		/* iterate over everything, in case we missed something before */
		ev_feed_event(EV_A_ &pipe_w, EV_CUSTOM);
	}
#endif

	postfork = 0;
}

