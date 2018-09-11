#if EV_IDLE_ENABLE
void ev_idle_start (EV_P_ ev_idle *w) EV_THROW
{
  if (expect_false (ev_is_active (w)))
    return;

  pri_adjust (EV_A_ (W)w);

  EV_FREQUENT_CHECK;

  {
    int active = ++idlecnt [ABSPRI (w)];

    ++idleall;
    ev_start (EV_A_ (W)w, active);

    array_needsize (ev_idle *, idles [ABSPRI (w)], idlemax [ABSPRI (w)], active, EMPTY2);
    idles [ABSPRI (w)][active - 1] = w;
  }

  EV_FREQUENT_CHECK;
}

void
ev_idle_stop (EV_P_ ev_idle *w) EV_THROW
{
  clear_pending (EV_A_ (W)w);
  if (expect_false (!ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  {
    int active = ev_active (w);

    idles [ABSPRI (w)][active - 1] = idles [ABSPRI (w)][--idlecnt [ABSPRI (w)]];
    ev_active (idles [ABSPRI (w)][active - 1]) = active;

    ev_stop (EV_A_ (W)w);
    --idleall;
  }

  EV_FREQUENT_CHECK;
}

/* make idle watchers pending. this handles the "call-idle */
/* only when higher priorities are idle" logic */
inline_size void idle_reify (EV_P)
{
	int pri;

	if (expect_false(idleall)) {
		for (pri = NUMPRI; pri--; ) {
			if (pendingcnt[pri])
				break;

			if (idlecnt[pri]) {
				queue_events(EV_A_ (W *)idles [pri], idlecnt [pri], EV_IDLE);
				break;
			}
		}
	}
}
#endif
