
#if EV_CLEANUP_ENABLE
void ev_cleanup_start (EV_P_ ev_cleanup *w) EV_THROW
{
  if (expect_false (ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  ev_start (EV_A_ (W)w, ++cleanupcnt);
  array_needsize (ev_cleanup *, cleanups, cleanupmax, cleanupcnt, EMPTY2);
  cleanups [cleanupcnt - 1] = w;

  /* cleanup watchers should never keep a refcount on the loop */
  ev_unref (EV_A);
  EV_FREQUENT_CHECK;
}

void
ev_cleanup_stop (EV_P_ ev_cleanup *w) EV_THROW
{
  clear_pending (EV_A_ (W)w);
  if (expect_false (!ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;
  ev_ref (EV_A);

  {
    int active = ev_active (w);

    cleanups [active - 1] = cleanups [--cleanupcnt];
    ev_active (cleanups [active - 1]) = active;
  }

  ev_stop (EV_A_ (W)w);

  EV_FREQUENT_CHECK;
}
#endif
