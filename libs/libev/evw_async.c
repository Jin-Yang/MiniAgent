
#if EV_ASYNC_ENABLE
void ev_async_start(EV_P_ ev_async *w) EV_THROW
{
	if (expect_false(ev_is_active(w)))
		return;

	w->sent = 0;

	evpipe_init(EV_A);

	EV_FREQUENT_CHECK;

	ev_start(EV_A_ (W)w, ++asynccnt);
	array_needsize(ev_async *, asyncs, asyncmax, asynccnt, EMPTY2);
	asyncs[asynccnt - 1] = w;

	EV_FREQUENT_CHECK;
}

void
ev_async_stop (EV_P_ ev_async *w) EV_THROW
{
  clear_pending (EV_A_ (W)w);
  if (expect_false (!ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  {
    int active = ev_active (w);

    asyncs [active - 1] = asyncs [--asynccnt];
    ev_active (asyncs [active - 1]) = active;
  }

  ev_stop (EV_A_ (W)w);

  EV_FREQUENT_CHECK;
}

void ev_async_send(EV_P_ ev_async *w) EV_THROW
{
	w->sent = 1;
	evpipe_write(EV_A_ &async_pending);
}
#endif

