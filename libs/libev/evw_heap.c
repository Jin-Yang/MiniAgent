
/*
 * the heap functions want a real array index. array index 0 is guaranteed to not
 * be in-use at any time. the first heap entry is at array [HEAP0]. DHEAP gives
 * the branching factor of the d-tree.
 *
 * at the moment we allow libev the luxury of two heaps,
 * a small-code-size 2-heap one and a ~1.5kb larger 4-heap
 * which is more cache-efficient.
 * the difference is about 5% with 50000+ watchers.
 */
#if EV_USE_4HEAP
#define DHEAP			4
#define HEAP0			(DHEAP - 1) /* index of first element in heap */
#define HPARENT(k) ((((k) - HEAP0 - 1) / DHEAP) + HEAP0)
#define UPHEAP_DONE(p,k) ((p) == (k))

/* away from the root */
inline_speed void downheap(ANHE *heap, int N, int k)
{
	ANHE he = heap[k];
	ANHE *E = heap + N + HEAP0;

	for (;;) {
		ev_tstamp minat;
		ANHE *minpos;
		ANHE *pos = heap + DHEAP * (k - HEAP0) + HEAP0 + 1;

		/* find minimum child */
		if (expect_true(pos + DHEAP - 1 < E)) {
			/* fast path */		       (minpos = pos + 0), (minat = ANHE_at (*minpos));
			if (ANHE_at (pos [1]) < minat) (minpos = pos + 1), (minat = ANHE_at (*minpos));
			if (ANHE_at (pos [2]) < minat) (minpos = pos + 2), (minat = ANHE_at (*minpos));
			if (ANHE_at (pos [3]) < minat) (minpos = pos + 3), (minat = ANHE_at (*minpos));
		} else if (pos < E) {
			/* slow path */				      (minpos = pos + 0), (minat = ANHE_at (*minpos));
			if (pos + 1 < E && ANHE_at (pos [1]) < minat) (minpos = pos + 1), (minat = ANHE_at (*minpos));
			if (pos + 2 < E && ANHE_at (pos [2]) < minat) (minpos = pos + 2), (minat = ANHE_at (*minpos));
			if (pos + 3 < E && ANHE_at (pos [3]) < minat) (minpos = pos + 3), (minat = ANHE_at (*minpos));
		} else {
			break;
		}

		if (ANHE_at (he) <= minat)
			break;

		heap[k] = *minpos;
		ev_active(ANHE_w(*minpos)) = k;

		k = minpos - heap;
	}

	heap[k] = he;
	ev_active(ANHE_w (he)) = k;
}
#else /* 4HEAP */

#define HEAP0			1
#define HPARENT(k)		((k) >> 1)
#define UPHEAP_DONE(p,k)	(!(p))

/* away from the root */
inline_speed void downheap(ANHE *heap, int N, int k)
{
	ANHE he = heap[k];

	for (;;) {
		int c = k << 1;

		if (c >= N + HEAP0)
			break;

		//c += c + 1 < N + HEAP0 && ANHE_at (heap [c]) > ANHE_at(heap[c + 1]) ? 1 : 0;
		c += ((c + 1) < (N + HEAP0)) && ANHE_at(heap[c]) > ANHE_at(heap[c + 1]) ? 1 : 0;

		if (ANHE_at(he) <= ANHE_at(heap[c]))
			break;

		heap[k] = heap[c];
		ev_active(ANHE_w(heap[k])) = k;

		k = c;
	}

	heap[k] = he;
	ev_active(ANHE_w(he)) = k;
}
#endif

/* towards the root */
inline_speed void upheap(ANHE *heap, int k)
{
	ANHE he = heap[k];

	for (;;) {
		int p = HPARENT(k);

		if (UPHEAP_DONE(p, k) || ANHE_at(heap[p]) <= ANHE_at(he))
			break;

		heap[k] = heap[p];
		ev_active(ANHE_w(heap[k])) = k;

		k = p;
	}

	heap [k] = he;
	ev_active (ANHE_w (he)) = k;
}

/* move an element suitably so it is in a correct place */
inline_size void adjustheap (ANHE *heap, int N, int k)
{
	if (k > HEAP0 && ANHE_at (heap [k]) <= ANHE_at (heap [HPARENT (k)]))
		upheap (heap, k);
	else
		downheap (heap, N, k);
}

/* rebuild the heap: this function is used only once and executed rarely */
inline_size void reheap (ANHE *heap, int N)
{
	int i;

	/* we don't use floyds algorithm, upheap is simpler and is more cache-efficient
	 * also, this is easy to implement and correct for both 2-heaps and 4-heaps.
	 */
	for (i = 0; i < N; ++i)
		upheap (heap, i + HEAP0);
}

/* make timers pending */
inline_size void timers_reify (EV_P)
{
	EV_FREQUENT_CHECK;

	if (timercnt && ANHE_at(timers[HEAP0]) < mn_now) {
		do {
			ev_timer *w = (ev_timer *)ANHE_w(timers[HEAP0]);

			assert(ev_is_active(w)); /* inactive timer on timer heap detected. */

			if (w->repeat) { /* first reschedule or stop timer */
				ev_at(w) += w->repeat;
				if (ev_at(w) < mn_now)
					ev_at(w) = mn_now;
				/* negative ev_timer repeat value found while processing timers. */
				assert(w->repeat > 0.);

				ANHE_at_cache(timers[HEAP0]);
				downheap(timers, timercnt, HEAP0);
			} else {
				ev_timer_stop(EV_A_ w); /* nonrepeating: stop timer */
			}

			EV_FREQUENT_CHECK;
			feed_reverse(EV_A_ (W)w);
		} while (timercnt && ANHE_at(timers[HEAP0]) < mn_now);

		feed_reverse_done(EV_A_ EV_TIMER);
	}
}

/* adjust all timers by a given offset */
static void noinline ecb_cold
timers_reschedule (EV_P_ ev_tstamp adjust)
{
  int i;

  for (i = 0; i < timercnt; ++i)
    {
      ANHE *he = timers + i + HEAP0;
      ANHE_w (*he)->at += adjust;
      ANHE_at_cache (*he);
    }
}

#if EV_PERIODIC_ENABLE
static void noinline periodic_recalc(EV_P_ ev_periodic *w)
{
	ev_tstamp interval = w->interval > MIN_INTERVAL ? w->interval : MIN_INTERVAL;
	ev_tstamp at = w->offset + interval * ev_floor ((ev_rt_now - w->offset) / interval);

	/* the above almost always errs on the low side */
	while (at <= ev_rt_now) {
		ev_tstamp nat = at + w->interval;

		/* when resolution fails us, we use ev_rt_now */
		if (expect_false (nat == at)) {
			at = ev_rt_now;
			break;
		}

		at = nat;
	}

	ev_at (w) = at;
}

/* make periodics pending */
inline_size void periodics_reify(EV_P)
{
	EV_FREQUENT_CHECK;

	while (periodiccnt && ANHE_at(periodics[HEAP0]) < ev_rt_now) {
		do {
			ev_periodic *w = (ev_periodic *)ANHE_w(periodics[HEAP0]);

			assert(ev_is_active (w)); /* inactive timer on periodic heap detected. */

			/* first reschedule or stop timer */
			if (w->reschedule_cb) {
				ev_at (w) = w->reschedule_cb (w, ev_rt_now);

				/* ev_periodic reschedule callback returned time in the past */
				assert(ev_at (w) >= ev_rt_now);

				ANHE_at_cache(periodics[HEAP0]);
				downheap(periodics, periodiccnt, HEAP0);
			} else if (w->interval) {
				periodic_recalc (EV_A_ w);
				ANHE_at_cache (periodics [HEAP0]);
				downheap (periodics, periodiccnt, HEAP0);
			} else {
				ev_periodic_stop (EV_A_ w); /* nonrepeating: stop timer */
			}

			EV_FREQUENT_CHECK;
			feed_reverse (EV_A_ (W)w);
		} while (periodiccnt && ANHE_at (periodics [HEAP0]) < ev_rt_now);

		feed_reverse_done (EV_A_ EV_PERIODIC);
	}
}

/*
 * simply recalculate all periodics.
 * TODO: maybe ensure that at least one event happens when jumping forward?
 */
static void noinline ecb_cold periodics_reschedule (EV_P)
{
	int i;

	/* adjust periodics after time jump */
	for (i = HEAP0; i < periodiccnt + HEAP0; ++i) {
		ev_periodic *w = (ev_periodic *)ANHE_w (periodics [i]);

		if (w->reschedule_cb)
			ev_at (w) = w->reschedule_cb (w, ev_rt_now);
		else if (w->interval)
			periodic_recalc (EV_A_ w);

		ANHE_at_cache (periodics [i]);
	}

	reheap (periodics, periodiccnt);
}

/*
 * fetch new monotonic and realtime times from the kernel
 * also detect if there was a timejump, and act accordingly.
 */
inline_speed void time_update(EV_P_ ev_tstamp max_block)
{
#if EV_USE_MONOTONIC
	if (expect_true(have_monotonic)) {
		int i;
		ev_tstamp odiff = rtmn_diff;

		mn_now = get_clock();

		/*
		 * only fetch the realtime clock every 0.5*MIN_TIMEJUMP seconds
		 * interpolate in the meantime.
		 */
		if (expect_true (mn_now - now_floor < MIN_TIMEJUMP * .5)) {
			ev_rt_now = rtmn_diff + mn_now;
			return;
		}

		now_floor = mn_now;
		ev_rt_now = ev_time ();

		/*
		 * loop a few times, before making important decisions.
		 * on the choice of "4": one iteration isn't enough,
		 * in case we get preempted during the calls to
		 * ev_time and get_clock. a second call is almost guaranteed
		 * to succeed in that case, though. and looping a few more times
		 * doesn't hurt either as we only do this on time-jumps or
		 * in the unlikely event of having been preempted here.
		 */
		for (i = 4; --i; ) {
			ev_tstamp diff;
			rtmn_diff = ev_rt_now - mn_now;

			diff = odiff - rtmn_diff;

			if (expect_true ((diff < 0. ? -diff : diff) < MIN_TIMEJUMP))
				return; /* all is well */

			ev_rt_now = ev_time();
			mn_now	  = get_clock();
			now_floor = mn_now;
		}

		/*
		 * no timer adjustment, as the monotonic clock doesn't jump
		 * timers_reschedule (EV_A_ rtmn_diff - odiff)
		 */
# if EV_PERIODIC_ENABLE
		periodics_reschedule (EV_A);
# endif
	} else
#endif
	{
		ev_rt_now = ev_time ();

		if (expect_false (mn_now > ev_rt_now || ev_rt_now > mn_now + max_block + MIN_TIMEJUMP)) {
			/* adjust timers. this is easy, as the offset is the same for all of them */
			timers_reschedule (EV_A_ ev_rt_now - mn_now);
#if EV_PERIODIC_ENABLE
			periodics_reschedule (EV_A);
#endif
		}

		mn_now = ev_rt_now;
	}
}
#endif
