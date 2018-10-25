

#if EV_VERIFY
static void noinline ecb_cold verify_watcher(EV_P_ W w)
{
	/* watcher has invalid priority */
	assert(ABSPRI(w) >= 0 && ABSPRI(w) < NUMPRI);

	/* pending watcher not on pending queue */
	if (w->pending)
		assert(pendings[ABSPRI(w)][w->pending - 1].w == w);
}

static void noinline ecb_cold verify_heap(EV_P_ ANHE *heap, int N)
{
	int i;

	for (i = HEAP0; i < N + HEAP0; ++i) {
		/* active index mismatch in heap */
		assert(ev_active(ANHE_w(heap[i])) == i);
		/* heap condition violated */
		assert(i == HEAP0 || ANHE_at(heap[HPARENT(i)]) <= ANHE_at(heap [i]));
		/* heap at cache mismatch */
		assert(ANHE_at(heap[i]) == ev_at(ANHE_w(heap[i])));

		verify_watcher(EV_A_ (W)ANHE_w(heap[i]));
	}
}
#if 0
static void noinline ecb_cold array_verify(EV_P_ W *ws, int cnt)
{
	while (cnt--) {
		/* active index mismatch */
		assert(ev_active(ws[cnt]) == cnt + 1);
		verify_watcher(EV_A_ ws[cnt]);
	}
}
#endif
#endif

#if EV_FEATURE_API
void ecb_cold ev_verify (EV_P) EV_THROW
{
#if EV_VERIFY
	int i;
	struct ev_watcher_list *w, *w2;

	assert(activecnt >= -1);

	assert(fdchangemax >= fdchangecnt);

#ifndef NDEBUG
	for (i = 0; i < fdchangecnt; ++i)
		assert(fdchanges[i] >= 0); /* negative fd in fdchanges */
#endif

	assert(anfdmax >= 0);
	for (i = 0; i < anfdmax; ++i) {
		int j = 0;

		for (w = w2 = anfds[i].head; w; w = w->next) {
			verify_watcher(EV_A_ (W)w);

			if (j++ & 1) {
				assert(w != w2); /* io watcher list contains a loop */
				w2 = w2->next;
			}

			/* inactive fd watcher on anfd list */
			assert(ev_active (w) == 1);
			/* fd mismatch between watcher and anfd */
			assert(((ev_io *)w)->fd == i);
		}
	}

	assert(timermax >= timercnt);
	verify_heap(EV_A_ timers, timercnt);

#if EV_PERIODIC_ENABLE
	assert(periodicmax >= periodiccnt);
	verify_heap(EV_A_ periodics, periodiccnt);
#endif

	for (i = NUMPRI; i--; ) {
		assert(pendingmax [i] >= pendingcnt [i]);
#if EV_IDLE_ENABLE
		assert(idleall >= 0);
		assert(idlemax[i] >= idlecnt[i]);
		array_verify(EV_A_ (W *)idles[i], idlecnt[i]);
#endif
	}

#if EV_FORK_ENABLE
	assert(forkmax >= forkcnt);
	array_verify(EV_A_ (W *)forks, forkcnt);
#endif

#if EV_CLEANUP_ENABLE
	assert(cleanupmax >= cleanupcnt);
	array_verify(EV_A_ (W *)cleanups, cleanupcnt);
#endif

#if EV_ASYNC_ENABLE
	assert (asyncmax >= asynccnt);
	array_verify (EV_A_ (W *)asyncs, asynccnt);
#endif

#if EV_PREPARE_ENABLE
	assert(preparemax >= preparecnt);
	array_verify(EV_A_ (W *)prepares, preparecnt);
#endif

#if EV_CHECK_ENABLE
	assert(checkmax >= checkcnt);
	array_verify(EV_A_ (W *)checks, checkcnt);
#endif

# if 0
#if EV_CHILD_ENABLE
	for (w = (ev_child *)childs [chain & ((EV_PID_HASHSIZE) - 1)]; w; w = (ev_child *)((WL)w)->next)
	for (signum = EV_NSIG; signum--; ) if (signals [signum].pending)
#endif
# endif
#endif
}
#endif
