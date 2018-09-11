#ifndef LIBNEV_SIGNAL_
#define LIBNEV_SIGNAL_

#ifndef SA_RESTART
	#define SA_RESTART 0
#endif

/* try to deduce the maximum number of signals on this platform */
#if defined EV_NSIG
	/* use what's provided */
#elif defined NSIG
#	define EV_NSIG (NSIG)
#elif defined _NSIG
#	define EV_NSIG (_NSIG)
#elif defined SIGMAX
#	define EV_NSIG (SIGMAX+1)
#elif defined SIG_MAX
#	define EV_NSIG (SIG_MAX+1)
#elif defined _SIG_MAX
#	define EV_NSIG (_SIG_MAX+1)
#elif defined MAXSIG
#	define EV_NSIG (MAXSIG+1)
#elif defined MAX_SIG
#	define EV_NSIG (MAX_SIG+1)
#elif defined SIGARRAYSIZE
#	define EV_NSIG (SIGARRAYSIZE) /* Assume ary[SIGARRAYSIZE] */
#elif defined _sys_nsig
#	define EV_NSIG (_sys_nsig)    /* Solaris 2.5 */
#else
#	define EV_NSIG (8 * sizeof (sigset_t) + 1)
#endif

#endif

