
#if EV_WALK_ENABLE
void ecb_cold ev_walk(EV_P_ int types, void (*cb)(EV_P_ int type, void *w)) EV_THROW
{
	int i, j;
	struct ev_watcher_list *wl, *wn;

	if (types & (EV_IO | EV_EMBED))
		for (i = 0; i < anfdmax; ++i)
			for (wl = anfds[i].head; wl; ) {
				wn = wl->next;

#if EV_EMBED_ENABLE
				if (ev_cb((ev_io *)wl) == embed_io_cb) {
					if (types & EV_EMBED)
		cb (EV_A_ EV_EMBED, ((char *)wl) - offsetof (struct ev_embed, io));
	    }
	  else
#endif
#if EV_USE_INOTIFY
	  if (ev_cb ((ev_io *)wl) == infy_cb)
	    ;
	  else
#endif
	  if ((ev_io *)wl != &pipe_w)
	    if (types & EV_IO)
	      cb (EV_A_ EV_IO, wl);

	  wl = wn;
	}

	if (types & (EV_TIMER | EV_STAT)) {
		for (i = timercnt + HEAP0; i-- > HEAP0; ) {
#if EV_STAT_ENABLE
			/*TODO: timer is not always active*/
			if (ev_cb((ev_timer *)ANHE_w(timers[i])) == stat_timer_cb)
				if (types & EV_STAT)
					cb (EV_A_ EV_STAT, ((char *)ANHE_w (timers [i])) - offsetof (struct ev_stat, timer));
			else
#endif
			if (types & EV_TIMER)
				cb(EV_A_ EV_TIMER, ANHE_w(timers [i]));
		}
	}

#if EV_PERIODIC_ENABLE
	if (types & EV_PERIODIC)
		for (i = periodiccnt + HEAP0; i-- > HEAP0; )
			cb(EV_A_ EV_PERIODIC, ANHE_w(periodics[i]));
#endif

#if EV_IDLE_ENABLE
	if (types & EV_IDLE)
		for (j = NUMPRI; j--; )
			for (i = idlecnt[j]; i--; )
				cb (EV_A_ EV_IDLE, idles[j][i]);
#endif

#if EV_FORK_ENABLE
	if (types & EV_FORK)
		for (i = forkcnt; i--; )
			if (ev_cb(forks [i]) != embed_fork_cb)
				cb (EV_A_ EV_FORK, forks[i]);
#endif

#if EV_ASYNC_ENABLE
	if (types & EV_ASYNC)
		for (i = asynccnt; i--; )
			cb (EV_A_ EV_ASYNC, asyncs[i]);
#endif

#if EV_PREPARE_ENABLE
	if (types & EV_PREPARE)
		for (i = preparecnt; i--; )
# if EV_EMBED_ENABLE
			if (ev_cb(prepares[i]) != embed_prepare_cb)
# endif
				cb(EV_A_ EV_PREPARE, prepares[i]);
#endif

#if EV_CHECK_ENABLE
	if (types & EV_CHECK)
		for (i = checkcnt; i--; )
			cb(EV_A_ EV_CHECK, checks[i]);
#endif

#if EV_SIGNAL_ENABLE
	if (types & EV_SIGNAL) {
		for (i = 0; i < EV_NSIG - 1; ++i) {
			for (wl = signals[i].head; wl; ) {
				wn = wl->next;
				cb(EV_A_ EV_SIGNAL, wl);
				wl = wn;
			}
		}
	}
#endif

#if EV_CHILD_ENABLE
	if (types & EV_CHILD) {
		for (i = (EV_PID_HASHSIZE); i--; ) {
			for (wl = childs [i]; wl; ) {
				wn = wl->next;
				cb(EV_A_ EV_CHILD, wl);
				wl = wn;
			}
		}
	}
#endif
}
#endif

/* }}} Child Watcher, -END- **************************************************/
