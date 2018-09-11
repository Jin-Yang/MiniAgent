
#if EV_CHECK_ENABLE
void
ev_check_start (EV_P_ ev_check *w) EV_THROW
{
  if (expect_false (ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  ev_start (EV_A_ (W)w, ++checkcnt);
  array_needsize (ev_check *, checks, checkmax, checkcnt, EMPTY2);
  checks [checkcnt - 1] = w;

  EV_FREQUENT_CHECK;
}

void
ev_check_stop (EV_P_ ev_check *w) EV_THROW
{
  clear_pending (EV_A_ (W)w);
  if (expect_false (!ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  {
    int active = ev_active (w);

    checks [active - 1] = checks [--checkcnt];
    ev_active (checks [active - 1]) = active;
  }

  ev_stop (EV_A_ (W)w);

  EV_FREQUENT_CHECK;
}
#endif

