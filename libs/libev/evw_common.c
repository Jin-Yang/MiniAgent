
#if EV_MULTIPLICITY
struct ev_loop * ecb_cold ev_loop_new(unsigned int flags) EV_THROW
{
	EV_P;

	EV_A = (struct ev_loop *)ev_malloc(sizeof(struct ev_loop));
	if (EV_A == NULL)
		return NULL;

	memset(EV_A, 0, sizeof(struct ev_loop));
	loop_init(EV_A_ flags);

	if (ev_backend(EV_A))
		return EV_A;

	ev_free(EV_A);

	return NULL;
}
#endif /* multiplicity */

unsigned int ev_backend(EV_P) EV_THROW
{
	return backend;
}

#if EV_FEATURE_API
unsigned int ev_iteration(EV_P) EV_THROW
{
	return loop_count;
}

unsigned int ev_depth(EV_P) EV_THROW
{
	return loop_depth;
}

void ev_set_io_collect_interval(EV_P_ ev_tstamp interval) EV_THROW
{
	io_blocktime = interval;
}

void ev_set_timeout_collect_interval(EV_P_ ev_tstamp interval) EV_THROW
{
	timeout_blocktime = interval;
}

void ev_set_invoke_pending_cb(EV_P_ ev_loop_callback invoke_pending_cb) EV_THROW
{
	invoke_cb = invoke_pending_cb;
}

void ev_set_loop_release_cb(EV_P_ void (*release)(EV_P) EV_THROW, void (*acquire)(EV_P) EV_THROW) EV_THROW
{
	release_cb = release;
	acquire_cb = acquire;
}
#endif
void ev_set_userdata (EV_P_ void *data) EV_THROW
{
	userdata = data;
}

void *ev_userdata (EV_P) EV_THROW
{
	return userdata;
}

int ecb_cold ev_version_major(void) EV_THROW
{
	return EV_VERSION_MAJOR;
}

int ecb_cold ev_version_minor(void) EV_THROW
{
	return EV_VERSION_MINOR;
}


void ev_break(EV_P_ int how) EV_THROW
{
	loop_done = how;
}

void ev_ref(EV_P) EV_THROW
{
	++activecnt;
}

void ev_unref(EV_P) EV_THROW
{
	--activecnt;
}

/* singly-linked list management, used when the expected list length is short */
inline_size void wlist_add(WL *head, WL elem)
{
	elem->next = *head;
	*head = elem;
}

inline_size void wlist_del(WL *head, WL elem)
{
	while (*head) {
		if (expect_true(*head == elem)) {
			*head = elem->next;
			break;
		}

		head = &(*head)->next;
	}
}

/* internal, faster, version of ev_clear_pending */
inline_speed void clear_pending (EV_P_ W w)
{
	if (w->pending) {
		pendings[ABSPRI(w)][w->pending - 1].w = (W)&pending_w;
		w->pending = 0;
	}
}

int ev_clear_pending (EV_P_ void *w) EV_THROW
{
	W w_ = (W)w;
	int pending = w_->pending;

	if (expect_true (pending)) {
		ANPENDING *p = pendings [ABSPRI (w_)] + pending - 1;
		p->w = (W)&pending_w;
		w_->pending = 0;
		return p->events;
	} else
		return 0;
}

inline_size void pri_adjust(EV_P_ W w)
{
	int pri = ev_priority(w);
	pri = pri < EV_MINPRI ? EV_MINPRI : pri;
	pri = pri > EV_MAXPRI ? EV_MAXPRI : pri;
	ev_set_priority(w, pri);
}

inline_speed void ev_start(EV_P_ W w, int active)
{
	pri_adjust(EV_A_ w);
	w->active = active;
	ev_ref(EV_A);
}

inline_size void ev_stop(EV_P_ W w)
{
	ev_unref (EV_A);
	w->active = 0;
}

void ev_now_update(EV_P) EV_THROW
{
	time_update(EV_A_ 1e100);
}

void ev_suspend(EV_P) EV_THROW
{
	ev_now_update(EV_A);
}

void ev_resume(EV_P) EV_THROW
{
	ev_tstamp mn_prev = mn_now;

	ev_now_update(EV_A);
	timers_reschedule(EV_A_ mn_now - mn_prev);
#if EV_PERIODIC_ENABLE
	/* TODO: really do this? */
	periodics_reschedule(EV_A);
#endif
}
