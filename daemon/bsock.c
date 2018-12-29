
#include "bsock.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>

#define BS_SUPP_LIMIT         1

#define CLI_CONNECT_TIMEOUT   10

#define MOD "(sock) "

static const char *BS_STATE[BS_ST_MAX] = {
	"init",
	"connecting",
	"connected",
	"disconnected",
};

static const char *BSEV_STATE[BSEV_MAX] = {
	"timer",
	"create socket error",
	"connect error",
	"connected",
	"connect timeout",
	"write error",
	"read end of file",
};

const char *bsock_get_evstate(int event)
{
	if (event < 0 || event > BSEV_MAX)
		return "invalid event";

	return BSEV_STATE[event];
}


struct ev_buff *buff_create(int s)
{
	struct ev_buff *buff;

	buff = malloc(sizeof(struct ev_buff) + s);
	if (buff == NULL)
		return NULL;
	buff->data = (char *)buff + sizeof(struct ev_buff);
	buff->start = buff->data;
	buff->tail = buff->data;
	buff->size = s;
	buff->end = buff->data + s;

	return buff;
}

int buff_drain(struct ev_buff *buff, int len)
{
	if (len >= buff->tail - buff->start) {
		buff->start = buff->data;
		buff->tail = buff->data;
		return 0;
	} else {
		buff->start += len;
		assert(buff->start <= buff->end);
		return buff_left(buff);
	}

	return 0;
}

void bsock_free(struct ev_bsock *bs)
{
	if (bs == NULL)
		return;

#define bsock_sfree(x) do { \
	if (x) {            \
		free(x);    \
		(x) = NULL; \
	}                   \
} while(0)

	bsock_sfree(bs->wbuf);
	bsock_sfree(bs->rbuf);
}

static void bsock_limit_cb(EV_P_ ev_timer *w, int revents)
{
	(void) revents;
	struct ev_bsock *bs;

	bs = w->data;
	if (ev_is_active(&bs->rsock) == 0)
		ev_io_start(EV_A_ &bs->rsock);
}

static void bsock_timer_cb(EV_P_ ev_timer *w, int revents)
{
	(void) revents;
	struct ev_bsock *bs;

	bs = w->data;
	ev_timer_stop(EV_A_ &bs->wtimer);
	log_trace(MOD "timer callback, current state <%s>.",
			BS_STATE[bs->state]);

	switch (bs->state) {
	case BS_INIT:
	case BS_CONNECTED:
	case BS_DISCONNECTED:
		if (bs->event)
			(*bs->event)(bs, BSEV_TIMER, bs->arg);
		break;

	case BS_CONNECTING:
		ev_io_stop(EV_A_ &bs->wsock);
		bsock_close(bs);
		if (bs->event)
			(*bs->event)(bs, BSEV_CONN_TIMEOUT, bs->arg);
		break;
	}
}

void bsock_destroy(struct ev_bsock *bs)
{
	if (bs == NULL)
		return;

	if (bs->wbuf)
		free(bs->wbuf);
	if (bs->rbuf)
		free(bs->rbuf);

	free(bs);
}

struct ev_bsock *bsock_create(int wbsize, int rbsize)
{
	struct ev_bsock *bs;

	bs = calloc(1, sizeof(*bs));
	if (bs == NULL)
		return NULL;
	bs->state = BS_INIT;

	if (wbsize > 0) {
		bs->wbuf = buff_create(wbsize);
		if (bs->wbuf == NULL)
			goto nomem;
	}

	if (rbsize > 0) {
		bs->rbuf = buff_create(rbsize);
		if (bs->rbuf == NULL)
			goto nomem;
	}

	ev_init(&bs->wtimer, bsock_timer_cb);
	bs->wtimer.data = bs;
	ev_timer_set(&bs->wtimer, (double)CLI_CONNECT_TIMEOUT, 0.0);

#if BS_SUPP_LIMIT == 1
	ev_init(&bs->rtimer, bsock_limit_cb);
	bs->rtimer.data = bs;
	ev_timer_set(&bs->rtimer, bs->delay, 0.0);
#endif

	return bs;

nomem:
	bsock_free(bs);
	return NULL;
}

#define bsock_error(msg, err, num) do {               \
	strncpy(bs->errmsg, msg, sizeof(bs->errmsg)); \
	bs->errnum = err;                             \
	if (bs->error)                                \
		(*bs->error)(bs, num, bs->arg);       \
	bsock_close(bs);                              \
} while(0)


static void bsock_read_cb(EV_P_ ev_io *w, int revents)
{
	(void) revents;
	int rc;
	struct ev_bsock *bs;

	bs = w->data;
	log_debug0(MOD "read callback for '%s:%s' on #%d.",
				bs->ipaddr, bs->ipport, bs->fd);

	/* need more buffer */
	rc = buff_left(bs->rbuf);
	if (rc <= 0 && bs->read) {
		log_debug0(MOD "no more space for read socket.");
		(*bs->read)(bs, bs->rbuf, bs->arg);
	}

	rc = read(w->fd, bs->rbuf->tail, buff_left(bs->rbuf));
	if (rc == -1) {
		if (errno == EAGAIN || errno == EINTR)
			return;
		goto error;
	} else if (rc == 0) {
		bsock_close(bs);
		ev_timer_stop(EV_A_ &bs->wtimer);
		if (bs->event)
			(*bs->event)(bs, BSEV_READEOF, bs->arg);
		return;
	}
	bs->rbuf->tail += rc;
	log_debug(MOD "read %d bytes.", rc);

	if (bs->read)
		(*bs->read)(bs, bs->rbuf, bs->arg);

#if BS_SUPP_LIMIT == 1
	if (bs->delay > 0) {
		ev_io_stop(EV_A_ &bs->rsock);
		ev_timer_set(&bs->rtimer, bs->delay, 0.0);
		ev_timer_start(EV_A_ &bs->rtimer);
	}
#endif

	return;

error:
	bsock_error("read failed", errno, BS_ERR_READ);
}

static void bsock_write_cb(EV_P_ ev_io *w, int revents)
{
	(void) revents;
	struct ev_bsock *bs;
	int rc, len;

	bs = w->data;
	len = buff_length(bs->wbuf);
	log_debug(MOD "write callback for '%s:%s' on %d, length %d bytes.",
			bs->ipaddr, bs->ipport, bs->fd, len);

	rc = write(bs->fd, bs->wbuf->start, len);
	if (rc < 0) {
		if (errno == EINTR)
			return;
		ev_io_start(EV_A_ &bs->wsock);
		if (bs->event)
			(*bs->event)(bs, BSEV_WRITE, bs->arg);
		bsock_close(bs);
		return;
	} else if (rc > 0) {
		log_debug(MOD "write %d bytes done.", rc);
		if (buff_drain(bs->wbuf, rc) == 0) {
			log_debug0(MOD "no more data to write.");
			ev_io_stop(EV_A_ &bs->wsock);
		}
		return;
	} else {
		ev_io_stop(EV_A_ &bs->wsock);
	}
}

static inline void bsock_connected(struct ev_bsock *bs)
{
	bs->state = BS_CONNECTED;

	ev_init(&bs->wsock, bsock_write_cb);
	bs->wsock.data = bs;
	ev_io_set(&bs->wsock, bs->fd, EV_WRITE);

	ev_init(&bs->rsock, bsock_read_cb);
	bs->rsock.data = bs;
	ev_io_set(&bs->rsock, bs->fd, EV_READ);

#if BS_SUPP_LIMIT == 0
	ev_io_start(EV_A_ &bs->rsock);
#else
	if (bs->delay > 0) {
		ev_timer_start(EV_A_ &bs->rtimer);
	} else {
		ev_io_start(EV_A_ &bs->rsock);
	}
#endif

	if (bs->event)
		(*bs->event)(bs, BSEV_CONNECT, bs->arg);


}

static void bsock_connect_cb(EV_P_ ev_io *w, int revents)
{
	(void) revents;
	int rc, error;
	struct ev_bsock *bs;
	socklen_t errsz = sizeof(error);

	bs = w->data;
	log_debug0(MOD "connect to '%s:%s' callback.", bs->ipaddr, bs->ipport);

	ev_timer_stop(EV_A_ &bs->wtimer);
	ev_io_stop(EV_A_ &bs->wsock);

	rc = getsockopt(bs->fd, SOL_SOCKET, SO_ERROR, (void *)&error, &errsz);
	if (rc == -1) {
		bsock_error("getsockopt failed", errno, BSEV_ERRCONN);
		return;
	}

	if (error) {
		log_error(MOD "connect to '%s:%s' failed, %s.",
				bs->ipaddr, bs->ipport, strerror(error));
		bsock_close(bs);
		if (bs->event)
			(*bs->event)(bs, BSEV_ERRCONN, bs->arg);
		return;
	}

	bsock_connected(bs);
}

int bsock_set_sock(struct ev_bsock *bs, struct addrinfo *addr)
{
	if (addr == NULL || bs == NULL)
		return -1;
	if (addr->ai_family != AF_INET && addr->ai_family != AF_INET6)
		return -1;

	memcpy(&bs->info, addr, sizeof(struct addrinfo));
	memcpy(&bs->sock, addr->ai_addr, sizeof(struct sockaddr));
	bs->info.ai_addr = NULL;
	bs->info.ai_next = NULL;

	if (bs->info.ai_family == AF_INET) {         /* IPv4 */
		struct sockaddr_in *ipv4;

		ipv4 = (struct sockaddr_in *)&bs->sock;
		inet_ntop(AF_INET, &ipv4->sin_addr, bs->ipaddr, sizeof(bs->ipaddr));
		snprintf(bs->ipport, sizeof(bs->ipport), "%d", ntohs(ipv4->sin_port));
	} else if (bs->info.ai_family == AF_INET6) { /* IPv6 */
		struct sockaddr_in6 *ipv6;

		ipv6 = (struct sockaddr_in6 *)&bs->sock;
		inet_ntop(AF_INET, &ipv6->sin6_addr, bs->ipaddr, sizeof(bs->ipaddr));
		snprintf(bs->ipport, sizeof(bs->ipport), "%d", ntohs(ipv6->sin6_port));
	}

	log_info(MOD "set sock address to '%s:%s'.", bs->ipaddr, bs->ipport);

	return 0;
}

void bsock_close(struct ev_bsock *bs)
{
	log_debug(MOD "closing '%s:%s'(%s) on fd #%d.",
			bs->ipaddr, bs->ipport, BS_STATE[bs->state], bs->fd);

	if (bs->state == BS_DISCONNECTED || bs->state == BS_INIT)
		return;

	if (bs->fd != -1) {
		close(bs->fd);
		bs->fd = -1;
	}

	//ev_timer_stop(EV_A_ &bs->wtimer);
	if (bs->state == BS_CONNECTED) {
		ev_io_stop(EV_A_ &bs->wsock);
		ev_io_stop(EV_A_ &bs->rsock);
	}
	bs->state = BS_DISCONNECTED;
}

int bsock_connect(struct ev_bsock *bs)
{
	int fd, rc;

	if (bs->state == BS_CONNECTED || bs->state == BS_CONNECTING) {
		log_warning(MOD "server '%s:%s' state <%s>.",
			bs->ipaddr, bs->ipport, BS_STATE[bs->state]);
		return 0;
	}
	log_info(MOD "connecting to '%s:%s', state <%s>.", bs->ipaddr, bs->ipport,
			BS_STATE[bs->state]);

	fd = socket(bs->info.ai_family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, bs->info.ai_protocol);
	if (fd == -1) {
		log_error(MOD "create socket failed, %s.", strerror(errno));
		return -1;
	}
	bs->fd = fd;

	rc = connect(fd, (struct sockaddr *)&bs->sock, sizeof(struct sockaddr));
	if (rc == -1 && errno != EINPROGRESS) {
		log_error(MOD "connect to server failed, %s.", strerror(errno));
		close(fd);
		return -1;
	} else if (rc == 0) {
		log_info(MOD "connect to server #%d success immediately.", fd);
		bsock_connected(bs);
		return fd;
	}
	bs->state = BS_CONNECTING;

	ev_init(&bs->wsock, bsock_connect_cb);
	ev_io_set(&bs->wsock, bs->fd, EV_WRITE);
	bs->wsock.data = bs;
	ev_io_start(EV_A_ &bs->wsock);

	bsock_timer_set(bs, CLI_CONNECT_TIMEOUT, 0.0);

	return bs->fd;
}

#define MIN(a, b) if ((a) > (b) ?

int bsock_write(struct ev_bsock *bs, const char *data, int len)
{
	int left, written;
	struct ev_buff *buff;

	if (bs->state != BS_CONNECTED)
		return -1;
	buff = bs->wbuf;
	left = buff_left(buff);
	log_debug(MOD "writing %d bytes starting at %p, left %d.",
			len, data, left);
	if (left == 0)
		return 0;

	written = left > len ? len : left;
	memcpy(buff->tail, data, written);
	buff->tail += written;

	if (ev_is_active(&bs->wsock) == 0) {
		ev_init(&bs->wsock, bsock_write_cb);
		ev_io_set(&bs->wsock, bs->fd, EV_WRITE);
		bs->wsock.data = bs;
		ev_io_start(EV_A_ &bs->wsock);
	}

	return written;
}

