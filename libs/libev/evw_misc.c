/* {{{ Timing stuff, BEGIN ***************************************************/
# if HAVE_CLOCK_SYSCALL
#	ifndef EV_USE_CLOCK_SYSCALL
#		define EV_USE_CLOCK_SYSCALL 1
#		ifndef EV_USE_REALTIME
#			define EV_USE_REALTIME	0
#		endif
#		ifndef EV_USE_MONOTONIC
#			define EV_USE_MONOTONIC 1
#		endif
#	endif
# elif !defined EV_USE_CLOCK_SYSCALL
#	define EV_USE_CLOCK_SYSCALL 0
# endif

/*
 * On linux, we can use a (slow) syscall to avoid a dependency on pthread,
 * which makes programs even slower. might work on other unices, too.
 */
#if EV_USE_CLOCK_SYSCALL
#	include <sys/syscall.h>
#	ifdef SYS_clock_gettime
#		define clock_gettime(id, ts) syscall (SYS_clock_gettime, (id), (ts))
#		undef EV_USE_MONOTONIC
#		define EV_USE_MONOTONIC 1
#	else
#		undef EV_USE_CLOCK_SYSCALL
#		define EV_USE_CLOCK_SYSCALL 0
#	endif
#endif

# if HAVE_CLOCK_GETTIME
#	ifndef EV_USE_MONOTONIC
#		define EV_USE_MONOTONIC 1
#	endif
#	ifndef EV_USE_REALTIME
#		define EV_USE_REALTIME	0
#	endif
# else
#	ifndef EV_USE_MONOTONIC
#		define EV_USE_MONOTONIC 0
#	endif
#	ifndef EV_USE_REALTIME
#		define EV_USE_REALTIME	0
#	endif
# endif

/* this block fixes any misconfiguration where we know we run into trouble otherwise */
#ifndef CLOCK_MONOTONIC
#	undef EV_USE_MONOTONIC
#	define EV_USE_MONOTONIC 0
#endif
#ifndef CLOCK_REALTIME
#	undef EV_USE_REALTIME
#	define EV_USE_REALTIME 0
#endif

// nanosleep setting.
# if HAVE_NANOSLEEP
#	ifndef EV_USE_NANOSLEEP
#		define EV_USE_NANOSLEEP EV_FEATURE_OS
#	endif
# else
#	undef EV_USE_NANOSLEEP
#	define EV_USE_NANOSLEEP 0
# endif

# ifndef EV_USE_NANOSLEEP
#	if _POSIX_C_SOURCE >= 199309L
#		define EV_USE_NANOSLEEP EV_FEATURE_OS
#	else
#		define EV_USE_NANOSLEEP 0
#	endif
# endif

# if !EV_USE_NANOSLEEP
/* hp-ux has it in sys/time.h, which we unconditionally include above */
#	if !defined _WIN32 && !defined __hpux
#		include <sys/select.h>
#	endif
# endif

#ifndef EV_HAVE_EV_TIME
ev_tstamp ev_time(void) EV_THROW
{
#if EV_USE_REALTIME
	if (expect_true(have_realtime)) {
		struct timespec ts;

		clock_gettime(CLOCK_REALTIME, &ts);
		return ts.tv_sec + ts.tv_nsec * 1e-9;
	}
#endif

	struct timeval tv;

	gettimeofday(&tv, 0);
	return tv.tv_sec + tv.tv_usec * 1e-6;
}
#endif

inline_size ev_tstamp get_clock(void)
{
#if EV_USE_MONOTONIC
	if (expect_true(have_monotonic)) {
		struct timespec ts;

		clock_gettime(CLOCK_MONOTONIC, &ts);
		return ts.tv_sec + ts.tv_nsec * 1e-9;
	}
#endif

	return ev_time ();
}

#if EV_MULTIPLICITY
ev_tstamp ev_now(EV_P) EV_THROW
{
	return ev_rt_now;
}
#endif

void ev_sleep(ev_tstamp delay) EV_THROW
{
	if (delay > 0.) {
#if EV_USE_NANOSLEEP
		struct timespec ts;

		EV_TS_SET(ts, delay);
		nanosleep(&ts, 0);
#elif defined _WIN32
		Sleep((unsigned long)(delay * 1e3));
#else
		/*
		 * here we rely on sys/time.h + sys/types.h + unistd.h providing select something
		 * not guaranteed by newer posix versions, but guaranteed by older ones
		 */
		struct timeval tv;

		EV_TV_SET(tv, delay);
		select(0, 0, 0, 0, &tv);
#endif
	}
}
/* }}} Timing stuff, -END- ***************************************************/


# if HAVE_FLOOR
#	ifndef EV_USE_FLOOR
#		define EV_USE_FLOOR 1
#	endif
# endif

#ifndef EV_USE_FLOOR
	#define EV_USE_FLOOR 0
#endif

/* define a suitable floor function (only used by periodics atm) */
#if EV_USE_FLOOR
#	include <math.h>
#	define ev_floor(v) floor(v)
#else
#include <float.h>
/* a floor() replacement function, should be independent of ev_tstamp type */
static ev_tstamp noinline ev_floor(ev_tstamp v)
{
	/* the choice of shift factor is not terribly important */
#if FLT_RADIX != 2 /* assume FLT_RADIX == 10 */
	const ev_tstamp shift = sizeof (unsigned long) >= 8 ? 10000000000000000000. : 1000000000.;
#else
	const ev_tstamp shift = sizeof (unsigned long) >= 8 ? 18446744073709551616. : 4294967296.;
#endif

	/* argument too large for an unsigned long? */
	if (expect_false (v >= shift)) {
		ev_tstamp f;
		if (v == v - 1.)
			return v; /* very large number */

		f = shift * ev_floor(v * (1. / shift));
		return f + ev_floor(v - f);
	}

	/* special treatment for negative args? */
	if (expect_false(v < 0.)) {
		ev_tstamp f = -ev_floor(-v);

		return f - (f == v ? 0 : 1);
	}

	return (unsigned long)v;  /* fits into an unsigned long */
}
#endif

#ifdef __linux
# include <sys/utsname.h>
#endif

static unsigned int noinline ecb_cold ev_linux_version(void)
{
#ifdef __linux
	unsigned int v = 0;
	struct utsname buf;
	int i;
	char *p = buf.release;

	if (uname(&buf))
		return 0;

	for (i = 3+1; --i; ) {
		unsigned int c = 0;

		for (;;) {
			if (*p >= '0' && *p <= '9') {
				c = c * 10 + *p++ - '0';
			} else {
				p += *p == '.';
				break;
			}
		}
		v = (v << 8) | c;
	}

	return v;
#else
	return 0;
#endif
}

#if EV_AVOID_STDIO
static void noinline ecb_cold ev_printerr (const char *msg)
{
	write(STDERR_FILENO, msg, strlen (msg));
}
#endif

static void (*syserr_cb)(const char *msg) EV_THROW;

void ecb_cold ev_set_syserr_cb (void (*cb)(const char *msg) EV_THROW) EV_THROW
{
	syserr_cb = cb;
}

static void noinline ecb_cold ev_syserr (const char *msg)
{
	if (!msg)
		msg = "(libev) system error";

	if (syserr_cb) {
		syserr_cb (msg);
	} else {
#if EV_AVOID_STDIO
		ev_printerr (msg);
		ev_printerr (": ");
		ev_printerr (strerror (errno));
		ev_printerr ("\n");
#else
		perror (msg);
#endif
		abort ();
	}
}
/* }}} Miscellanea, -END- ****************************************************/

/* {{{ Memory related, BEGIN *************************************************/
static void *ev_realloc_emul(void *ptr, long size) EV_THROW
{
	/*
	 * some systems, notably openbsd and darwin, fail to properly
	 * implement realloc (x, 0) (as required by both ansi c-89 and
	 * the single unix specification, so work around them here.
	 * recently, also (at least) fedora and debian started breaking it,
	 * despite documenting it otherwise.
	 */
	if (size)
		return realloc(ptr, size);

	free(ptr);
	return 0;
}

static void *(*alloc)(void *ptr, long size) EV_THROW = ev_realloc_emul;

void ecb_cold ev_set_allocator(void *(*cb)(void *ptr, long size)) EV_THROW
{
	alloc = cb;
}

inline_speed void *ev_realloc(void *ptr, long size)
{
	ptr = alloc(ptr, size);

	if (ptr == NULL && size) {
#if EV_AVOID_STDIO
		ev_printerr("(libev) memory allocation failed, aborting.");
#else
		fprintf(stderr, "(libev) cannot allocate %ld bytes, aborting.\n", size);
#endif
		abort();
	}

	return ptr;
}

#define ev_malloc(size) ev_realloc (0, (size))
#define ev_free(ptr)	ev_realloc ((ptr), 0)

/* prefer to allocate in chunks of this size, must be 2**n and >> 4 longs */
#define MALLOC_ROUND 4096
/*
 * find a suitable new size for the given array,
 * hopefully by rounding to a nice-to-malloc size.
 */
inline_size int array_nextsize(int elem, int cur, int cnt)
{
	int ncur = cur + 1;

	do
		ncur <<= 1;
	while (cnt > ncur);

	/* if size is large, round to MALLOC_ROUND - 4 * longs to accommodate malloc overhead */
	if (elem * ncur > (int)(MALLOC_ROUND - sizeof (void *) * 4)) {
		ncur *= elem;
		ncur = (ncur + elem + (MALLOC_ROUND - 1) + sizeof (void *) * 4) & ~(MALLOC_ROUND - 1);
		ncur = ncur - sizeof (void *) * 4;
		ncur /= elem;
	}

	return ncur;
}

static void * noinline ecb_cold array_realloc (int elem, void *base, int *cur, int cnt)
{
	*cur = array_nextsize(elem, *cur, cnt);
	return ev_realloc(base, elem ** cur);
}

#define array_init_zero(base,count)	\
	memset((void *)(base), 0, sizeof(*(base)) * (count))

#define array_needsize(type,base,cur,cnt,init)					       \
	if (expect_false((cnt) > (cur))) {					       \
		int cbt_unused ocur_ = (cur);					       \
		(base) = (type *)array_realloc(sizeof(type), (base), &(cur), (cnt));   \
		init((base) + (ocur_), (cur) - ocur_);				       \
	}

#if 0
#define array_slim(type,stem)							       \
	if (stem ## max < array_roundsize (stem ## cnt >> 2)) {			       \
		stem ## max = array_roundsize (stem ## cnt >> 1);		       \
		base = (type *)ev_realloc (base, sizeof (type) * (stem ## max));       \
		fprintf (stderr, "slimmed down " # stem " to %d\n", stem ## max);/*D*/ \
	}
#endif

#define array_free(stem, idx) \
	ev_free(stem ## s idx); stem ## cnt idx = stem ## max idx = 0; stem ## s idx = 0
/* }}} Memory related, -END- *************************************************/

