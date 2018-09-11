
#if EV_EMBED_ENABLE
void noinline
ev_embed_sweep (EV_P_ ev_embed *w) EV_THROW
{
  ev_run (w->other, EVRUN_NOWAIT);
}

static void
embed_io_cb (EV_P_ ev_io *io, int revents)
{
  ev_embed *w = (ev_embed *)(((char *)io) - offsetof (ev_embed, io));

  if (ev_cb (w))
    ev_feed_event (EV_A_ (W)w, EV_EMBED);
  else
    ev_run (w->other, EVRUN_NOWAIT);
}

static void
embed_prepare_cb (EV_P_ ev_prepare *prepare, int revents)
{
  ev_embed *w = (ev_embed *)(((char *)prepare) - offsetof (ev_embed, prepare));

  {
    EV_P = w->other;

    while (fdchangecnt)
      {
	fd_reify (EV_A);
	ev_run (EV_A_ EVRUN_NOWAIT);
      }
  }
}

static void embed_fork_cb (EV_P_ ev_fork *fork_w, int revents)
{
  ev_embed *w = (ev_embed *)(((char *)fork_w) - offsetof (ev_embed, fork));

  ev_embed_stop (EV_A_ w);

  {
    EV_P = w->other;

    ev_loop_fork (EV_A);
    ev_run (EV_A_ EVRUN_NOWAIT);
  }

  ev_embed_start (EV_A_ w);
}

#if 0
static void
embed_idle_cb (EV_P_ ev_idle *idle, int revents)
{
  ev_idle_stop (EV_A_ idle);
}
#endif

void
ev_embed_start (EV_P_ ev_embed *w) EV_THROW
{
  if (expect_false (ev_is_active (w)))
    return;

  {
    EV_P = w->other;
    //assert (("libev: loop to be embedded is not embeddable", backend & ev_embeddable_backends ()));
    ev_io_init (&w->io, embed_io_cb, backend_fd, EV_READ);
  }

  EV_FREQUENT_CHECK;

  ev_set_priority (&w->io, ev_priority (w));
  ev_io_start (EV_A_ &w->io);

  ev_prepare_init (&w->prepare, embed_prepare_cb);
  ev_set_priority (&w->prepare, EV_MINPRI);
  ev_prepare_start (EV_A_ &w->prepare);

  ev_fork_init (&w->fork, embed_fork_cb);
  ev_fork_start (EV_A_ &w->fork);

  /*ev_idle_init (&w->idle, e,bed_idle_cb);*/

  ev_start (EV_A_ (W)w, 1);

  EV_FREQUENT_CHECK;
}

void
ev_embed_stop (EV_P_ ev_embed *w) EV_THROW
{
  clear_pending (EV_A_ (W)w);
  if (expect_false (!ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

  ev_io_stop	  (EV_A_ &w->io);
  ev_prepare_stop (EV_A_ &w->prepare);
  ev_fork_stop	  (EV_A_ &w->fork);

  ev_stop (EV_A_ (W)w);

  EV_FREQUENT_CHECK;
}
#endif
