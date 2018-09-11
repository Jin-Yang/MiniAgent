
/* this big block deduces configuration from config.h */
#ifndef EV_STANDALONE

/* {{{ Macro defination, BEGIN ***********************************************/
# if HAVE_SELECT && HAVE_SYS_SELECT_H
#  ifndef EV_USE_SELECT
#   define EV_USE_SELECT EV_FEATURE_BACKENDS
#  endif
# else
#  undef EV_USE_SELECT
#  define EV_USE_SELECT 0
# endif

# if HAVE_POLL && HAVE_POLL_H
#  ifndef EV_USE_POLL
#   define EV_USE_POLL EV_FEATURE_BACKENDS
#  endif
# else
#  undef EV_USE_POLL
#  define EV_USE_POLL 0
# endif

#if 0
# if HAVE_EPOLL_CTL && HAVE_SYS_EPOLL_H
#  ifndef EV_USE_EPOLL
#   define EV_USE_EPOLL EV_FEATURE_BACKENDS
#  endif
# else
#  undef EV_USE_EPOLL
#  define EV_USE_EPOLL 0
# endif
#endif

# if HAVE_KQUEUE && HAVE_SYS_EVENT_H
#  ifndef EV_USE_KQUEUE
#   define EV_USE_KQUEUE EV_FEATURE_BACKENDS
#  endif
# else
#  undef EV_USE_KQUEUE
#  define EV_USE_KQUEUE 0
# endif

# if HAVE_PORT_H && HAVE_PORT_CREATE
#  ifndef EV_USE_PORT
#   define EV_USE_PORT EV_FEATURE_BACKENDS
#  endif
# else
#  undef EV_USE_PORT
#  define EV_USE_PORT 0
# endif

# if HAVE_INOTIFY_INIT && HAVE_SYS_INOTIFY_H
#  ifndef EV_USE_INOTIFY
#   define EV_USE_INOTIFY EV_FEATURE_OS
#  endif
# else
#  undef EV_USE_INOTIFY
#  define EV_USE_INOTIFY 0
# endif

# if HAVE_SIGNALFD && HAVE_SYS_SIGNALFD_H
#  ifndef EV_USE_SIGNALFD
#   define EV_USE_SIGNALFD EV_FEATURE_OS
#  endif
# else
#  undef EV_USE_SIGNALFD
#  define EV_USE_SIGNALFD 0
# endif

# if HAVE_EVENTFD
#  ifndef EV_USE_EVENTFD
#   define EV_USE_EVENTFD EV_FEATURE_OS
#  endif
# else
#  undef EV_USE_EVENTFD
#  define EV_USE_EVENTFD 0
# endif
#endif

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <limits.h>
#include <signal.h>

#include "ev.h"
#include "ecb.h"

#if EV_NO_THREADS
# undef EV_NO_SMP
# define EV_NO_SMP 1
# undef ECB_NO_THREADS
# define ECB_NO_THREADS 1
#endif
#if EV_NO_SMP
# undef EV_NO_SMP
# define ECB_NO_SMP 1
#endif

#ifndef _WIN32
# include <sys/time.h>
# include <sys/wait.h>
# include <unistd.h>
#else
# include <io.h>
# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# include <windows.h>
# ifndef EV_SELECT_IS_WINSOCKET
#  define EV_SELECT_IS_WINSOCKET 1
# endif
# undef EV_AVOID_STDIO
#endif

/* OS X, in its infinite idiocy, actually HARDCODES
 * a limit of 1024 into their select. Where people have brains,
 * OS X engineers apparently have a vacuum. Or maybe they were
 * ordered to have a vacuum, or they do anything for money.
 * This might help. Or not.
 */
#define _DARWIN_UNLIMITED_SELECT 1

/* this block tries to deduce configuration from header-defined symbols and defaults */
#ifndef EV_USE_CLOCK_SYSCALL
# if __linux && __GLIBC__ == 2 && __GLIBC_MINOR__ < 17
#  define EV_USE_CLOCK_SYSCALL EV_FEATURE_OS
# else
#  define EV_USE_CLOCK_SYSCALL 0
# endif
#endif

#if !(_POSIX_TIMERS > 0)
# ifndef EV_USE_MONOTONIC
#  define EV_USE_MONOTONIC 0
# endif
# ifndef EV_USE_REALTIME
#  define EV_USE_REALTIME 0
# endif
#endif

#ifndef EV_USE_MONOTONIC
# if defined _POSIX_MONOTONIC_CLOCK && _POSIX_MONOTONIC_CLOCK >= 0
#  define EV_USE_MONOTONIC EV_FEATURE_OS
# else
#  define EV_USE_MONOTONIC 0
# endif
#endif

#ifndef EV_USE_REALTIME
# define EV_USE_REALTIME !EV_USE_CLOCK_SYSCALL
#endif


#ifndef EV_USE_SELECT
# define EV_USE_SELECT EV_FEATURE_BACKENDS
#endif

#ifndef EV_USE_POLL
# ifdef _WIN32
#  define EV_USE_POLL 0
# else
#  define EV_USE_POLL EV_FEATURE_BACKENDS
# endif
#endif

#ifndef EV_USE_EPOLL
# if __linux && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 4))
#  define EV_USE_EPOLL EV_FEATURE_BACKENDS
# else
#  define EV_USE_EPOLL 0
# endif
#endif

#ifndef EV_USE_KQUEUE
# define EV_USE_KQUEUE 0
#endif

#ifndef EV_USE_PORT
# define EV_USE_PORT 0
#endif

#ifndef EV_USE_INOTIFY
# if __linux && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 4))
#  define EV_USE_INOTIFY EV_FEATURE_OS
# else
#  define EV_USE_INOTIFY 0
# endif
#endif

#ifndef EV_PID_HASHSIZE
# define EV_PID_HASHSIZE EV_FEATURE_DATA ? 16 : 1
#endif

#ifndef EV_INOTIFY_HASHSIZE
# define EV_INOTIFY_HASHSIZE EV_FEATURE_DATA ? 16 : 1
#endif

#ifndef EV_USE_EVENTFD
# if __linux && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 7))
#  define EV_USE_EVENTFD EV_FEATURE_OS
# else
#  define EV_USE_EVENTFD 0
# endif
#endif

#ifndef EV_USE_SIGNALFD
# if __linux && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 7))
#  define EV_USE_SIGNALFD EV_FEATURE_OS
# else
#  define EV_USE_SIGNALFD 0
# endif
#endif

#if 0 /* debugging */
# define EV_VERIFY 3
# define EV_USE_4HEAP 1
# define EV_HEAP_CACHE_AT 1
#endif

#ifndef EV_VERIFY
# define EV_VERIFY (EV_FEATURE_API ? 1 : 0)
#endif

#ifndef EV_USE_4HEAP
# define EV_USE_4HEAP EV_FEATURE_DATA
#endif

#ifndef EV_HEAP_CACHE_AT
# define EV_HEAP_CACHE_AT EV_FEATURE_DATA
#endif

#if !EV_STAT_ENABLE
# undef EV_USE_INOTIFY
# define EV_USE_INOTIFY 0
#endif

#if EV_USE_INOTIFY
# include <sys/statfs.h>
# include <sys/inotify.h>
/* some very old inotify.h headers don't have IN_DONT_FOLLOW */
# ifndef IN_DONT_FOLLOW
#  undef EV_USE_INOTIFY
#  define EV_USE_INOTIFY 0
# endif
#endif

#if EV_USE_EVENTFD
/* our minimum requirement is glibc 2.7 which has the stub, but not the header */
# include <stdint.h>
# ifndef EFD_NONBLOCK
#  define EFD_NONBLOCK O_NONBLOCK
# endif
# ifndef EFD_CLOEXEC
#  ifdef O_CLOEXEC
#   define EFD_CLOEXEC O_CLOEXEC
#  else
#   define EFD_CLOEXEC 02000000
#  endif
# endif
EV_CPP(extern "C") int (eventfd) (unsigned int initval, int flags);
#endif

#if EV_USE_SIGNALFD
/* our minimum requirement is glibc 2.7 which has the stub, but not the header */
# include <stdint.h>
# ifndef SFD_NONBLOCK
#  define SFD_NONBLOCK O_NONBLOCK
# endif
# ifndef SFD_CLOEXEC
#  ifdef O_CLOEXEC
#   define SFD_CLOEXEC O_CLOEXEC
#  else
#   define SFD_CLOEXEC 02000000
#  endif
# endif
EV_CPP (extern "C") int signalfd (int fd, const sigset_t *mask, int flags);

struct signalfd_siginfo
{
  uint32_t ssi_signo;
  char pad[128 - sizeof (uint32_t)];
};
#endif

/**/

#if EV_VERIFY >= 3
#      define EV_FREQUENT_CHECK ev_verify(EV_A)
#else
#      define EV_FREQUENT_CHECK do { } while (0)
#endif

/*
 * This is used to work around floating point rounding problems.
 * This value is good at least till the year 4000.
 */
#define MIN_INTERVAL  0.0001220703125 /* 1/2**13, good till 4000 */

#define MIN_TIMEJUMP  1. /* minimum timejump that gets detected (if monotonic clock available) */
#define MAX_BLOCKTIME 59.743 /* never wait longer than this time (to detect time jumps) */

#define EV_TV_SET(tv,t) do { tv.tv_sec = (long)t; tv.tv_usec = (long)((t - tv.tv_sec) * 1e6); } while (0)
#define EV_TS_SET(ts,t) do { ts.tv_sec = (long)t; ts.tv_nsec = (long)((t - ts.tv_sec) * 1e9); } while (0)


#if ECB_MEMORY_FENCE_NEEDS_PTHREADS
/* if your architecture doesn't need memory fences, e.g. because it is
 * single-cpu/core, or if you use libev in a project that doesn't use libev
 * from multiple threads, then you can define ECB_AVOID_PTHREADS when compiling
 * libev, in which cases the memory fences become nops.
 * alternatively, you can remove this #error and link against libpthread,
 * which will then provide the memory fences.
 */
# error "memory fences not defined for your architecture, please report"
#endif

#ifndef ECB_MEMORY_FENCE
# define ECB_MEMORY_FENCE do { } while (0)
# define ECB_MEMORY_FENCE_ACQUIRE ECB_MEMORY_FENCE
# define ECB_MEMORY_FENCE_RELEASE ECB_MEMORY_FENCE
#endif

#if EV_FEATURE_CODE
# define inline_speed	   ecb_inline
#else
# define inline_speed	   static noinline
#endif

#define NUMPRI (EV_MAXPRI - EV_MINPRI + 1)

#if EV_MINPRI == EV_MAXPRI
# define ABSPRI(w) (((W)w), 0)
#else
# define ABSPRI(w) (((W)w)->priority - EV_MINPRI)
#endif

#define EMPTY	    /* required for microsofts broken pseudo-c compiler */
#define EMPTY2(a,b) /* used to suppress some warnings */

typedef ev_watcher	*W;
typedef ev_watcher_list *WL;
typedef ev_watcher_time *WT;

#define ev_active(w) ((W)(w))->active
#define ev_at(w) ((WT)(w))->at

#if EV_USE_REALTIME
/* sig_atomic_t is used to avoid per-thread variables or locking but still */
/* giving it a reasonably high chance of working on typical architectures */
static EV_ATOMIC_T have_realtime; /* did clock_gettime (CLOCK_REALTIME) work? */
#endif

#if EV_USE_MONOTONIC
static EV_ATOMIC_T have_monotonic; /* did clock_gettime (CLOCK_MONOTONIC) work? */
#endif

#ifndef EV_FD_TO_WIN32_HANDLE
# define EV_FD_TO_WIN32_HANDLE(fd) _get_osfhandle (fd)
#endif
#ifndef EV_WIN32_HANDLE_TO_FD
# define EV_WIN32_HANDLE_TO_FD(handle) _open_osfhandle (handle, 0)
#endif
#ifndef EV_CLOSE_FD
# define EV_CLOSE_FD(fd) close (fd)
#endif

#ifdef _WIN32
# include "ev_win32.c"
#endif
/* }}} Macro defination, -END- ***********************************************/

#if EV_SIGNAL_ENABLE
/* associate signal watchers to a signal signal */
typedef struct {
	EV_ATOMIC_T pending;
#if EV_MULTIPLICITY
	EV_P;
#endif
	struct ev_watcher_list *head;
} ANSIG;

static ANSIG signals[EV_NSIG - 1];
#endif

/* {{{ Structure defination, BEGIN *******************************************/
/* set in reify when reification needed */
#define EV_ANFD_REIFY 1

/* file descriptor info structure */
typedef struct {
	WL head;
	unsigned char events; /* the events watched for */
	unsigned char reify;  /* flag set when this ANFD needs reification (EV_ANFD_REIFY, EV__IOFDSET) */
	unsigned char emask;  /* the epoll backend stores the actual kernel mask in here */
	unsigned char unused;
#if EV_USE_EPOLL
	unsigned int egen;    /* generation counter to counter epoll bugs */
#endif
#if EV_SELECT_IS_WINSOCKET || EV_USE_IOCP
	SOCKET handle;
#endif
#if EV_USE_IOCP
	OVERLAPPED or, ow;
#endif
} ANFD;

/* stores the pending event set for a given watcher */
typedef struct {
	W w;
	int events; /* the pending event set for the given watcher */
} ANPENDING;

#if EV_USE_INOTIFY
	/* hash table entry per inotify-id */
	typedef struct {
		WL head;
	} ANFS;
#endif

/* Heap Entry */
#if EV_HEAP_CACHE_AT
	/* a heap element */
	typedef struct {
		ev_tstamp at;
		WT w;
	} ANHE;

	#define ANHE_w(he)	  (he).w     /* access watcher, read-write */
	#define ANHE_at(he)	  (he).at    /* access cached at, read-only */
	#define ANHE_at_cache(he) (he).at = (he).w->at /* update at from watcher */
#else
	/* a heap element */
	typedef WT ANHE;

	#define ANHE_w(he)	  (he)
	#define ANHE_at(he)	  (he)->at
	#define ANHE_at_cache(he)
#endif

#if EV_MULTIPLICITY
	struct ev_loop {
		ev_tstamp ev_rt_now;
		#define ev_rt_now ((loop)->ev_rt_now)
		#define VAR(name,decl) decl;
		#include "ev_vars.h"
		#undef VAR
	};
	#include "ev_wrap.h"
	static struct ev_loop default_loop_struct;
	/* needs to be initialised to make it a definition despite extern */
	EV_API_DECL struct ev_loop *ev_default_loop_ptr = 0;
#else
	/* needs to be initialised to make it a definition despite extern */
	ev_tstamp ev_rt_now = 0;
	#define VAR(name,decl) static decl;
	#include "ev_vars.h"
	#undef VAR
	static int ev_default_loop_ptr;
#endif

#if EV_FEATURE_API
	#define EV_RELEASE_CB	    if (expect_false(release_cb)) release_cb(EV_A)
	#define EV_ACQUIRE_CB	    if (expect_false(acquire_cb)) acquire_cb(EV_A)
	#define EV_INVOKE_PENDING   invoke_cb(EV_A)
#else
	#define EV_RELEASE_CB	    (void)0
	#define EV_ACQUIRE_CB	    (void)0
	#define EV_INVOKE_PENDING   ev_invoke_pending(EV_A)
#endif

#define EVBREAK_RECURSE 0x80
/* }}} Structure defination, -END- *******************************************/

#include "evw_misc.c"
#include "evw_fd.c"
#include "evw_heap.c"
#include "evw_pipe.c"

#if EV_CHILD_ENABLE
static WL childs[EV_PID_HASHSIZE];
static ev_signal childev;

#ifndef WIFCONTINUED
	#define WIFCONTINUED(status) 0
#endif

/* handle a single child status event */
inline_speed void child_reap (EV_P_ int chain, int pid, int status)
{
	ev_child *w;
	int traced = WIFSTOPPED (status) || WIFCONTINUED (status);

	for (w = (ev_child *)childs[chain & ((EV_PID_HASHSIZE) - 1)]; w; w = (ev_child *)((WL)w)->next) {
		if ((w->pid == pid || !w->pid) && (!traced || (w->flags & 1))) {
			/* need to do it *now*, this *must* be the same prio as the signal watcher itself */
			ev_set_priority(w, EV_MAXPRI);
			w->rpid    = pid;
			w->rstatus = status;
			ev_feed_event(EV_A_ (W)w, EV_CHILD);
		}
	}
}

#ifndef WCONTINUED
	#define WCONTINUED 0
#endif

/* called on sigchld etc., calls waitpid */
static void childcb (EV_P_ ev_signal *sw, int revents)
{
	int pid, status;

	/* some systems define WCONTINUED but then fail to support it (linux 2.4) */
	if (0 >= (pid = waitpid (-1, &status, WNOHANG | WUNTRACED | WCONTINUED)))
		if (!WCONTINUED || errno != EINVAL || 0 >= (pid = waitpid (-1, &status, WNOHANG | WUNTRACED)))
			return;

	/*
	 * make sure we are called again until all children have been reaped
	 * we need to do it this way so that the callback gets called before we continue
	 */
	ev_feed_event(EV_A_ (W)sw, EV_SIGNAL);

	child_reap(EV_A_ pid, pid, status);
	if ((EV_PID_HASHSIZE) > 1)
		child_reap(EV_A_ 0, pid, status); /* this might trigger a watcher twice, but feed_event catches that */
}
#endif
/* }}} UUUUUUUUUUUUUUUUUUUUUUU, BEGIN ****************************************/

#include "evw_backend.c"
#include "evw_verify.c"

/* {{{ Core, BEGIN ***********************************************************/
#if EV_MULTIPLICITY
struct ev_loop * ecb_cold
#else
int
#endif
ev_default_loop(unsigned int flags) EV_THROW
{
	if (ev_default_loop_ptr == 0) {
#if EV_MULTIPLICITY
		EV_P = ev_default_loop_ptr = &default_loop_struct;
#else
		ev_default_loop_ptr = 1;
#endif

		loop_init(EV_A_ flags);

		if (ev_backend(EV_A)) {
#if EV_CHILD_ENABLE
			ev_signal_init(&childev, childcb, SIGCHLD);
			ev_set_priority(&childev, EV_MAXPRI);
			ev_signal_start(EV_A_ &childev);
			ev_unref(EV_A); /* child watcher should not keep loop alive */
#endif
		} else {
			ev_default_loop_ptr = 0;
		}
	}

	return ev_default_loop_ptr;
}

void ev_loop_fork(EV_P) EV_THROW
{
	postfork = 1;
}

void ev_invoke(EV_P_ void *w, int revents)
{
	EV_CB_INVOKE((W)w, revents);
}

unsigned int ev_pending_count(EV_P) EV_THROW
{
	int pri;
	unsigned int count = 0;

	for (pri = NUMPRI; pri > 0; --pri)
		count += pendingcnt [pri];

	return count;
}

void noinline ev_invoke_pending(EV_P)
{
	ANPENDING *p;

	pendingpri = NUMPRI;

	/* pendingpri possibly gets modified in the inner loop */
	while (pendingpri) {
		--pendingpri;

		while (pendingcnt[pendingpri]) {
			p = pendings[pendingpri] + --pendingcnt[pendingpri];

			p->w->pending = 0;
			EV_CB_INVOKE(p->w, p->events);
			EV_FREQUENT_CHECK;
		}
	}
}

#if 0
void walk_cb(EV_P_ int type, void *w) EV_THROW
{
	printf("===> %d %p\n", type, w);
}
#endif

int ev_run(EV_P_ int flags)
{
#if EV_FEATURE_API
	++loop_depth;
#endif

	/* libev: ev_loop recursion during release detected. */
	assert(loop_done != EVBREAK_RECURSE);

	loop_done = EVBREAK_CANCEL;

	/* in case we recurse, ensure ordering stays nice and clean */
	EV_INVOKE_PENDING;

	do {
#if EV_VERIFY >= 2
		ev_verify(EV_A);
#endif

#ifndef _WIN32
		/* penalise the forking check even more */
		if (expect_false(curpid)) {
			if (expect_false(getpid() != curpid)) {
				curpid = getpid();
				postfork = 1;
			}
		}
#endif

#if EV_FORK_ENABLE
		/* we might have forked, so queue fork handlers */
		if (expect_false(postfork)) {
			if (forkcnt) {
				queue_events(EV_A_ (W *)forks, forkcnt, EV_FORK);
				EV_INVOKE_PENDING;
			}
		}
#endif

#if EV_PREPARE_ENABLE
		/* queue prepare watchers (and execute them) */
		if (expect_false(preparecnt)) {
			queue_events(EV_A_ (W *)prepares, preparecnt, EV_PREPARE);
			EV_INVOKE_PENDING;
		}
#endif
		if (expect_false(loop_done))
			break;

		/* we might have forked, so reify kernel state if necessary */
		if (expect_false(postfork))
			loop_fork(EV_A);

		/* update fd-related kernel structures */
		fd_reify(EV_A);

		/* calculate blocking time ===== BEGIN ===== */
		ev_tstamp waittime  = 0.;
		ev_tstamp sleeptime = 0.;

		/* remember old timestamp for io_blocktime calculation */
		ev_tstamp prev_mn_now = mn_now;

		/* update time to cancel out callback processing overhead */
		time_update(EV_A_ 1e100);

		/* from now on, we want a pipe-wake-up */
		pipe_write_wanted = 1;

		ECB_MEMORY_FENCE; /* make sure pipe_write_wanted is visible before we check for potential skips */

		if (expect_true(!(flags & EVRUN_NOWAIT || idleall || !activecnt || pipe_write_skipped))) {
			waittime = MAX_BLOCKTIME;

			if (timercnt) {
				ev_tstamp to = ANHE_at(timers [HEAP0]) - mn_now;
				if (waittime > to)
					waittime = to;
			}

#if EV_PERIODIC_ENABLE
			if (periodiccnt) {
				ev_tstamp to = ANHE_at (periodics [HEAP0]) - ev_rt_now;
				if (waittime > to)
					waittime = to;
			}
#endif

			/* don't let timeouts decrease the waittime below timeout_blocktime */
			if (expect_false(waittime < timeout_blocktime))
				waittime = timeout_blocktime;

			/*
			 * at this point, we NEED to wait, so we have to ensure to pass a
			 * minimum nonzero value to the backend.
			 */
			if (expect_false(waittime < backend_mintime))
				waittime = backend_mintime;

			/* extra check because io_blocktime is commonly 0 */
			if (expect_false(io_blocktime)) {
				sleeptime = io_blocktime - (mn_now - prev_mn_now);

				if (sleeptime > waittime - backend_mintime)
					sleeptime = waittime - backend_mintime;

				if (expect_true(sleeptime > 0.)) {
					ev_sleep(sleeptime);
					waittime -= sleeptime;
				}
			}
		}

#if EV_FEATURE_API
		++loop_count;
#endif
		//assert ((loop_done = EVBREAK_RECURSE, 1)); /* assert for side effect */
		backend_poll(EV_A_ waittime);
		//assert ((loop_done = EVBREAK_CANCEL, 1)); /* assert for side effect */

		pipe_write_wanted = 0; /* just an optimisation, no fence needed */

		ECB_MEMORY_FENCE_ACQUIRE;
		if (pipe_write_skipped) {
			/* pipe_w not active, but pipe not written. */
			assert(ev_is_active(&pipe_w));
			ev_feed_event(EV_A_ &pipe_w, EV_CUSTOM);
		}

		/* update ev_rt_now, do magic */
		time_update(EV_A_ waittime + sleeptime);
		/* calculate blocking time ===== -END- ===== */

		/* queue pending timers and reschedule them */
		timers_reify(EV_A); /* relative timers called last */
#if EV_PERIODIC_ENABLE
		periodics_reify(EV_A); /* absolute timers called first */
#endif

#if EV_IDLE_ENABLE
		idle_reify(EV_A); /* queue idle watchers unless other events are pending */
#endif

#if EV_CHECK_ENABLE
		/* queue check watchers, to be executed first */
		if (expect_false(checkcnt))
			queue_events(EV_A_ (W *)checks, checkcnt, EV_CHECK);
#endif
		//ev_walk(EV_A_ EV_TIMER, walk_cb);
		//ev_walk(EV_A_ EV_IO, walk_cb);

		EV_INVOKE_PENDING;
	} while (expect_true(activecnt && !loop_done && !(flags & (EVRUN_ONCE | EVRUN_NOWAIT))));

	if (loop_done == EVBREAK_ONE)
		loop_done = EVBREAK_CANCEL;

#if EV_FEATURE_API
	--loop_depth;
#endif

	return activecnt;
}
/* }}} Core, -END- ***********************************************************/

#include "evw_common.c"
#include "evw_signal.c"
#include "evw_io.c"
#include "evw_timer.c"
#include "evw_periodic.c"
#include "evw_child.c"
#include "evw_once.c"
#include "evw_walk.c"

#if EV_MULTIPLICITY
	#include "ev_wrap.h"
#endif

// vim: foldmarker={{{,}}} foldlevel=0 foldmethod=marker
