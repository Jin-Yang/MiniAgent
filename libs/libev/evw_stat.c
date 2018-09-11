
#if EV_STAT_ENABLE

# ifdef _WIN32
#  undef lstat
#  define lstat(a,b) _stati64 (a,b)
# endif

#define DEF_STAT_INTERVAL  5.0074891
#define NFS_STAT_INTERVAL 30.1074891 /* for filesystems potentially failing inotify */
#define MIN_STAT_INTERVAL  0.1074891

static void noinline stat_timer_cb (EV_P_ ev_timer *w_, int revents);

#if EV_USE_INOTIFY

/* the * 2 is to allow for alignment padding, which for some reason is >> 8 */
# define EV_INOTIFY_BUFSIZE (sizeof (struct inotify_event) * 2 + NAME_MAX)

static void noinline
infy_add (EV_P_ ev_stat *w)
{
  w->wd = inotify_add_watch (fs_fd, w->path,
			     IN_ATTRIB | IN_DELETE_SELF | IN_MOVE_SELF | IN_MODIFY
			     | IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO
			     | IN_DONT_FOLLOW | IN_MASK_ADD);

  if (w->wd >= 0)
    {
      struct statfs sfs;

      /* now local changes will be tracked by inotify, but remote changes won't */
      /* unless the filesystem is known to be local, we therefore still poll */
      /* also do poll on <2.6.25, but with normal frequency */

      if (!fs_2625)
	w->timer.repeat = w->interval ? w->interval : DEF_STAT_INTERVAL;
      else if (!statfs (w->path, &sfs)
	       && (sfs.f_type == 0x1373 /* devfs */
		   || sfs.f_type == 0x4006 /* fat */
		   || sfs.f_type == 0x4d44 /* msdos */
		   || sfs.f_type == 0xEF53 /* ext2/3 */
		   || sfs.f_type == 0x72b6 /* jffs2 */
		   || sfs.f_type == 0x858458f6 /* ramfs */
		   || sfs.f_type == 0x5346544e /* ntfs */
		   || sfs.f_type == 0x3153464a /* jfs */
		   || sfs.f_type == 0x9123683e /* btrfs */
		   || sfs.f_type == 0x52654973 /* reiser3 */
		   || sfs.f_type == 0x01021994 /* tmpfs */
		   || sfs.f_type == 0x58465342 /* xfs */))
	w->timer.repeat = 0.; /* filesystem is local, kernel new enough */
      else
	w->timer.repeat = w->interval ? w->interval : NFS_STAT_INTERVAL; /* remote, use reduced frequency */
    }
  else
    {
      /* can't use inotify, continue to stat */
      w->timer.repeat = w->interval ? w->interval : DEF_STAT_INTERVAL;

      /* if path is not there, monitor some parent directory for speedup hints */
      /* note that exceeding the hardcoded path limit is not a correctness issue, */
      /* but an efficiency issue only */
      if ((errno == ENOENT || errno == EACCES) && strlen (w->path) < 4096)
	{
	  char path [4096];
	  strcpy (path, w->path);

	  do
	    {
	      int mask = IN_MASK_ADD | IN_DELETE_SELF | IN_MOVE_SELF
		       | (errno == EACCES ? IN_ATTRIB : IN_CREATE | IN_MOVED_TO);

	      char *pend = strrchr (path, '/');

	      if (!pend || pend == path)
		break;

	      *pend = 0;
	      w->wd = inotify_add_watch (fs_fd, path, mask);
	    }
	  while (w->wd < 0 && (errno == ENOENT || errno == EACCES));
	}
    }

  if (w->wd >= 0)
    wlist_add (&fs_hash [w->wd & ((EV_INOTIFY_HASHSIZE) - 1)].head, (WL)w);

  /* now re-arm timer, if required */
  if (ev_is_active (&w->timer)) ev_ref (EV_A);
  ev_timer_again (EV_A_ &w->timer);
  if (ev_is_active (&w->timer)) ev_unref (EV_A);
}

static void noinline
infy_del (EV_P_ ev_stat *w)
{
  int slot;
  int wd = w->wd;

  if (wd < 0)
    return;

  w->wd = -2;
  slot = wd & ((EV_INOTIFY_HASHSIZE) - 1);
  wlist_del (&fs_hash [slot].head, (WL)w);

  /* remove this watcher, if others are watching it, they will rearm */
  inotify_rm_watch (fs_fd, wd);
}

static void noinline
infy_wd (EV_P_ int slot, int wd, struct inotify_event *ev)
{
  if (slot < 0)
    /* overflow, need to check for all hash slots */
    for (slot = 0; slot < (EV_INOTIFY_HASHSIZE); ++slot)
      infy_wd (EV_A_ slot, wd, ev);
  else
    {
      WL w_;

      for (w_ = fs_hash [slot & ((EV_INOTIFY_HASHSIZE) - 1)].head; w_; )
	{
	  ev_stat *w = (ev_stat *)w_;
	  w_ = w_->next; /* lets us remove this watcher and all before it */

	  if (w->wd == wd || wd == -1)
	    {
	      if (ev->mask & (IN_IGNORED | IN_UNMOUNT | IN_DELETE_SELF))
		{
		  wlist_del (&fs_hash [slot & ((EV_INOTIFY_HASHSIZE) - 1)].head, (WL)w);
		  w->wd = -1;
		  infy_add (EV_A_ w); /* re-add, no matter what */
		}

	      stat_timer_cb (EV_A_ &w->timer, 0);
	    }
	}
    }
}

static void
infy_cb (EV_P_ ev_io *w, int revents)
{
  char buf [EV_INOTIFY_BUFSIZE];
  int ofs;
  int len = read (fs_fd, buf, sizeof (buf));

  for (ofs = 0; ofs < len; )
    {
      struct inotify_event *ev = (struct inotify_event *)(buf + ofs);
      infy_wd (EV_A_ ev->wd, ev->wd, ev);
      ofs += sizeof (struct inotify_event) + ev->len;
    }
}

inline_size void ecb_cold
ev_check_2625 (EV_P)
{
  /* kernels < 2.6.25 are borked
   * http://www.ussg.indiana.edu/hypermail/linux/kernel/0711.3/1208.html
   */
  if (ev_linux_version () < 0x020619)
    return;

  fs_2625 = 1;
}

inline_size int
infy_newfd (void)
{
#if defined IN_CLOEXEC && defined IN_NONBLOCK
  int fd = inotify_init1 (IN_CLOEXEC | IN_NONBLOCK);
  if (fd >= 0)
    return fd;
#endif
  return inotify_init ();
}

inline_size void
infy_init (EV_P)
{
  if (fs_fd != -2)
    return;

  fs_fd = -1;

  ev_check_2625 (EV_A);

  fs_fd = infy_newfd ();

  if (fs_fd >= 0)
    {
      fd_intern (fs_fd);
      ev_io_init (&fs_w, infy_cb, fs_fd, EV_READ);
      ev_set_priority (&fs_w, EV_MAXPRI);
      ev_io_start (EV_A_ &fs_w);
      ev_unref (EV_A);
    }
}

inline_size void
infy_fork (EV_P)
{
  int slot;

  if (fs_fd < 0)
    return;

  ev_ref (EV_A);
  ev_io_stop (EV_A_ &fs_w);
  close (fs_fd);
  fs_fd = infy_newfd ();

  if (fs_fd >= 0)
    {
      fd_intern (fs_fd);
      ev_io_set (&fs_w, fs_fd, EV_READ);
      ev_io_start (EV_A_ &fs_w);
      ev_unref (EV_A);
    }

  for (slot = 0; slot < (EV_INOTIFY_HASHSIZE); ++slot)
    {
      WL w_ = fs_hash [slot].head;
      fs_hash [slot].head = 0;

      while (w_)
	{
	  ev_stat *w = (ev_stat *)w_;
	  w_ = w_->next; /* lets us add this watcher */

	  w->wd = -1;

	  if (fs_fd >= 0)
	    infy_add (EV_A_ w); /* re-add, no matter what */
	  else
	    {
	      w->timer.repeat = w->interval ? w->interval : DEF_STAT_INTERVAL;
	      if (ev_is_active (&w->timer)) ev_ref (EV_A);
	      ev_timer_again (EV_A_ &w->timer);
	      if (ev_is_active (&w->timer)) ev_unref (EV_A);
	    }
	}
    }
}

#endif

#ifdef _WIN32
# define EV_LSTAT(p,b) _stati64 (p, b)
#else
# define EV_LSTAT(p,b) lstat (p, b)
#endif

void
ev_stat_stat (EV_P_ ev_stat *w) EV_THROW
{
  if (lstat (w->path, &w->attr) < 0)
    w->attr.st_nlink = 0;
  else if (!w->attr.st_nlink)
    w->attr.st_nlink = 1;
}

static void noinline stat_timer_cb (EV_P_ ev_timer *w_, int revents)
{
	(void)revents;
  ev_stat *w = (ev_stat *)(((char *)w_) - offsetof (ev_stat, timer));

  ev_statdata prev = w->attr;
  ev_stat_stat (EV_A_ w);

  /* memcmp doesn't work on netbsd, they.... do stuff to their struct stat */
  if (
    prev.st_dev      != w->attr.st_dev
    || prev.st_ino   != w->attr.st_ino
    || prev.st_mode  != w->attr.st_mode
    || prev.st_nlink != w->attr.st_nlink
    || prev.st_uid   != w->attr.st_uid
    || prev.st_gid   != w->attr.st_gid
    || prev.st_rdev  != w->attr.st_rdev
    || prev.st_size  != w->attr.st_size
    || prev.st_atime != w->attr.st_atime
    || prev.st_mtime != w->attr.st_mtime
    || prev.st_ctime != w->attr.st_ctime
  ) {
      /* we only update w->prev on actual differences */
      /* in case we test more often than invoke the callback, */
      /* to ensure that prev is always different to attr */
      w->prev = prev;

      #if EV_USE_INOTIFY
	if (fs_fd >= 0)
	  {
	    infy_del (EV_A_ w);
	    infy_add (EV_A_ w);
	    ev_stat_stat (EV_A_ w); /* avoid race... */
	  }
      #endif

      ev_feed_event (EV_A_ w, EV_STAT);
    }
}

void
ev_stat_start (EV_P_ ev_stat *w) EV_THROW
{
  if (expect_false (ev_is_active (w)))
    return;

  ev_stat_stat (EV_A_ w);

  if (w->interval < MIN_STAT_INTERVAL && w->interval)
    w->interval = MIN_STAT_INTERVAL;

  ev_timer_init (&w->timer, stat_timer_cb, 0., w->interval ? w->interval : DEF_STAT_INTERVAL);
  ev_set_priority (&w->timer, ev_priority (w));

#if EV_USE_INOTIFY
  infy_init (EV_A);

  if (fs_fd >= 0)
    infy_add (EV_A_ w);
  else
#endif
    {
      ev_timer_again (EV_A_ &w->timer);
      ev_unref (EV_A);
    }

  ev_start (EV_A_ (W)w, 1);

  EV_FREQUENT_CHECK;
}

void
ev_stat_stop (EV_P_ ev_stat *w) EV_THROW
{
  clear_pending (EV_A_ (W)w);
  if (expect_false (!ev_is_active (w)))
    return;

  EV_FREQUENT_CHECK;

#if EV_USE_INOTIFY
  infy_del (EV_A_ w);
#endif

  if (ev_is_active (&w->timer))
    {
      ev_ref (EV_A);
      ev_timer_stop (EV_A_ &w->timer);
    }

  ev_stop (EV_A_ (W)w);

  EV_FREQUENT_CHECK;
}
#endif

