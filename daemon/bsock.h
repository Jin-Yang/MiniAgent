#ifndef BSOCK_H_
#define BSOCK_H_

#include "config.h"
#include "libev/ev.h"
#include "liblog/log.h"

#include <netdb.h>
#include <arpa/inet.h>

#define INET_PORT_LEN    8
struct ev_buff {
	char *data, *start, *tail, *end;
	int size;
};

enum ev_bsock_states {
	BS_INIT,
	BS_CONNECTING,
	BS_CONNECTED,
	BS_DISCONNECTED,
	BS_ST_MAX
};

enum {
	BSEV_TIMER,
	BSEV_SOCKET,
	BSEV_ERRCONN, /* connect failed */
	BSEV_CONNECT, /* connected */
	BSEV_CONN_TIMEOUT, /* connect time out */
	BSEV_WRITE, /* write error */
	BSEV_READEOF, /* read end of file */
	BS_ERR_READ,
	BS_ERR_CONNECT,
	BSEV_MAX,
};

struct ev_bsock {
	char *addr, *port;
	int state;
	int fd;

	struct addrinfo info;
	struct sockaddr sock;

	double delay;

	char ipaddr[INET6_ADDRSTRLEN];
	char ipport[INET_PORT_LEN]; /* max 65536 */



	struct ev_io wsock, rsock;
	struct ev_timer rtimer, wtimer;

	struct ev_buff *wbuf, *rbuf;

#if 0
    struct ev_io read_ev;
    struct ev_io write_ev;
    struct Buffer *read_buf;
    struct Buffer *write_buf;
    struct ev_timer read_bytes_timer_ev;
    size_t read_bytes_n;
    void (*read_bytes_callback)(struct BufferedSocket *buffsock, void *arg);
    void *read_bytes_arg;
    struct ev_loop *loop;
#endif

	void (*read)(struct ev_bsock *bs, struct ev_buff *buf, void *arg);
	void (*write)(struct ev_bsock *bs, void *arg);
	void (*event)(struct ev_bsock *bs, int event, void *arg);

	void (*connect)(struct ev_bsock *bs, void *arg);
	void (*close)(struct ev_bsock *bs, void *arg);
	void (*error)(struct ev_bsock *bs, int err, void *arg);
	void *arg;
	int errnum;
	char errmsg[128];
};

#define bsock_timer_set(bs, after_, repeat_) do {                           \
	ev_timer_set(&(bs)->wtimer, (double)(after_), (double)(repeat_));   \
        ev_timer_start(EV_A_ &(bs)->wtimer);                                \
} while(0)

#define bsock_set_arg(bs, arg_)     (bs)->arg = (arg_)
#define bsock_set_read(bs, cb)     (bs)->read = (cb)
#define bsock_set_write(bs, cb)    (bs)->write = (cb)
#define bsock_set_event(bs, cb)    (bs)->event = (cb)

#define bsock_set_connect(bs, cb)  (bs)->connect = cb
#define bsock_set_close(bs, cb)    (bs)->close = cb
#define bsock_set_error(bs, cb)    (bs)->error = cb

#define bsock_get_timer(bs)        (bs)->wtimer

#define buff_left(buf)     ((buf)->end - (buf)->tail)
#define buff_length(buf)   ((buf)->tail - (buf)->start)
#define buff_restart(buf)  (buf)->tail = (buf)->start;
#define buff_string(buf)   ((buf)->start)
#define buff_seal(buf)                          \
	do {                                    \
		if ((buf)->end == (buf)->tail)  \
			*((buf)->tail - 1) = 0; \
		else                            \
			*(buf)->tail = 0;       \
} while(0)
int buff_drain(struct ev_buff *buff, int len);

struct ev_bsock *bsock_create(int wbsize, int rbsize);
int bsock_connect(struct ev_bsock *bs);
int bsock_set_sock(struct ev_bsock *bs, struct addrinfo *addr);
void bsock_close(struct ev_bsock *bs);
int bsock_write(struct ev_bsock *bs, const char *data, int len);
const char *bsock_get_evstate(int event);
void bsock_destroy(struct ev_bsock *bs);

#endif
