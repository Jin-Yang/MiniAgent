
#if EV_PREPARE_ENABLE
void
ev_prepare_start (EV_P_ ev_prepare *w) EV_THROW
{
  if (expect_false (ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  ev_start (EV_A_ (W)w, ++preparecnt);
  array_needsize (ev_prepare *, prepares, preparemax, preparecnt, EMPTY2);
  prepares [preparecnt - 1] = w;

  EV_FREQUENT_CHECK;
}

void
ev_prepare_stop (EV_P_ ev_prepare *w) EV_THROW
{
  clear_pending (EV_A_ (W)w);
  if (expect_false (!ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  {
    int active = ev_active (w);

    prepares [active - 1] = prepares [--preparecnt];
    ev_active (prepares [active - 1]) = active;
  }

  ev_stop (EV_A_ (W)w);

  EV_FREQUENT_CHECK;
}
#endif

