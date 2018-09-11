
#include "mini.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <execinfo.h>

#include "libev/ev.h"

static void exit_usage(int status)
{
        printf("Usage: booter [OPTIONS]\n\n"
		"Available Options:\n"
		"    -f                Don't fork to the background.\n"
		"    -P <file>         PID file path.\n"
		"             builtin: " PROJECT_PID_FILE "\n"
		"    -h                Display help (this message).\n\n"
		"Builtin Variables:\n"
		"Version: " PROJECT_VERSION "\n"
		"Compile: " __DATE__ " "  __TIME__ "\n");

	exit(status);
}

static int check_pidfile(const char *file)
{
	int fd, rc, pid;
	char buffer[128];

	fd = open(file, O_CLOEXEC | O_RDONLY, 0);
	if (fd >= 0) { /* file already exists */
		log_debug("PIDFile '%s' already exists.", file);
		rc = read(fd, buffer, sizeof(buffer) - 1);
		if (rc < 0) {
			log_error("read from PIDFile '%s' failed", file);
			close(fd);
			return -1;
		}
		buffer[rc] = 0;

		pid = atoi(buffer);
		if (pid <= 1) {
			log_error("invalid content from PIDFile '%s': %s.",
					file, buffer);
			close(fd);
			return -1;
		}
		close(fd);

		/* check if the proess is really exists. */
		rc = snprintf(buffer, sizeof(buffer), "/proc/%d/comm", pid);
		if (rc < 0 || rc >= (int)sizeof(buffer)) {
			log_error("try to format '/proc/%d/comm' failed, %s.",
					pid, strerror(errno));
			return -1;
		}

		fd = open(buffer, O_CLOEXEC | O_RDONLY, 0);
		if (fd > 0) { /* file already exists, check it's command */
			rc = read(fd, buffer, sizeof(buffer) - 1);
			if (rc < 0) {
				log_error("read from '/proc/%d/comm' failed, %s.",
						pid, strerror(errno));
				close(fd);
				return -1;
			}
			buffer[rc - 1] = 0; /* skip '\n' */

			if (strcmp(PROJECT_NAME, buffer) == 0) {
				log_error("process(%d) already exists.", pid);
				close(fd);
				return -1;
			}
			log_info("file '/proc/%d/comm' exists, (" PROJECT_NAME ") != (%s).",
					pid, buffer);
			close(fd);
		} else if (fd < 0 && errno == ENOENT) {
			log_info("PIDFile exists, but '/proc/%d/comm' doesn't.", pid);
		} else if (fd < 0) {
			log_info("open(/proc/%d/comm) failed, %s.", pid, strerror(errno));
			return -1;
		}
	}

	fd = open(file, O_CLOEXEC | O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		log_error("open PIDFile '%s' failed. %s", file, strerror(errno));
		return -1;
	}

	rc = snprintf(buffer, sizeof(buffer), "%i\n", getpid());
	if (rc > (int)sizeof(buffer) - 1)
		rc = sizeof(buffer) - 1;
	buffer[rc] = 0;

        rc = write(fd, buffer, rc);
	if (rc < 0) {
		log_error("write PIDFile '%s' failed. %s", file, strerror(errno));
		close(fd);
		return -1;
	}

        close(fd);

	return 0;
}

void log_backtrace(int signal)
{
        int i, frames;
        void *callstack[128];
        char **strs;

	log_error("got SEGV(%d) signal.", signal);
        frames = backtrace(callstack, 128);
        strs = backtrace_symbols(callstack, frames);
        if (strs == NULL) {
                log_error("backtrace_symbols failed, %s", strerror(errno));
                exit(1);
        }

        for (i = 0; i < frames; ++i)
                log_error("    [%d]: %s", i, strs[i]);
        free(strs);
        exit(1);
}

int bar(void)
{
        int *addr = NULL;

        *addr = 3;
        return 0;
}

int foo(void)
{
        bar();
        return 0;
}


struct ev_buff {
	char *start, *tail, *end;
	int size;
};

enum ev_bsock_states {
	BS_INIT,
	BS_CONNECTING,
	BS_CONNECTED,
	BS_DISCONNECTED
};

struct ev_bsock {
	char *addr, *port;
	int state;
	int fd;

	struct ev_io wsock, rsock;
	struct ev_timer wtimer;

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
	void (*connect)(struct ev_bsock *bs, void *arg);
	void (*close)(struct ev_bsock *bs, void *arg);
	void (*error)(struct ev_bsock *bs, void *arg);
	void *arg;
};


#define bsock_set_read(bs, cb)     (bs)->read = cb;
#define bsock_set_write(bs, cb)    (bs)->write = cb;
#define bsock_set_connect(bs, cb)  (bs)->connect = cb;
#define bsock_set_close(bs, cb)    (bs)->close = cb;
#define bsock_set_error(bs, cb)    (bs)->error = cb;

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

struct ev_buff *buff_create(int s)
{
	struct ev_buff *buff;

	buff = malloc(sizeof(struct ev_buff) + s);
	if (buff == NULL)
		return NULL;
	buff->start = (char *)buff + sizeof(struct ev_buff);
	buff->size = s;
	buff->tail = buff->start;
	buff->end = buff->start + s;

	return buff;
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

	bsock_sfree(bs->addr);
	bsock_sfree(bs->wbuf);
	bsock_sfree(bs->rbuf);
}

struct ev_bsock *bsock_create(const char *addr, int wbsize, int rbsize)
{
	struct ev_bsock *bs;

	bs = calloc(1, sizeof(*bs));
	if (bs == NULL)
		return NULL;
	bs->addr = strdup(addr);
	if (bs->addr == NULL) {
		free(bs);
		return NULL;
	}
	bs->port = strchr(bs->addr, ':');
	if (bs->port == NULL) {
		bs->port = "80";
	} else {
		*bs->port = 0;
		bs->port++;
	}
	bs->state = BS_INIT;

	bs->wbuf = buff_create(wbsize);
	bs->rbuf = buff_create(rbsize);
	if (bs->wbuf == NULL || bs->rbuf == NULL)
		goto nomem;

	return bs;

nomem:
	bsock_free(bs);
	return NULL;
}

#include <netdb.h>
#include <sys/socket.h>

void bsock_close(struct ev_bsock *bs)
{
	if (bs->state == BS_DISCONNECTED || bs->state == BS_INIT)
		return;

	log_debug("closing '%s:%s' on %d", bs->addr, bs->port, bs->fd);

	if (bs->fd != -1) {
		close(bs->fd);
		bs->fd = -1;
	}

	//ev_timer_stop(EV_A_ &bs->wtimer);
	//ev_io_stop(EV_A_ &bs->wsock);

	if (bs->state == BS_CONNECTED) {
		ev_io_stop(EV_A_ &bs->wsock);
		ev_io_stop(EV_A_ &bs->rsock);
	}
	bs->state = BS_DISCONNECTED;

#if 0
    ev_timer_stop(buffsock->loop, &buffsock->read_bytes_timer_ev);
#endif

}

static void bsock_read_cb(EV_P_ ev_io *w, int revents)
{
	(void) revents;
	int rc;
	struct ev_bsock *bs;

	bs = w->data;
	log_debug("read callback for '%s:%s' on %d",
				bs->addr, bs->port, bs->fd);

	/* need more buffer */
	rc = buff_left(bs->rbuf);
	if (rc <= 0 && bs->read)
		(*bs->read)(bs, bs->rbuf, bs->arg);

	rc = read(w->fd, bs->rbuf->tail, buff_left(bs->rbuf));
	if (rc == -1) {
		if (errno == EAGAIN || errno == EINTR)
			return;
		goto error;
	} else if (rc == 0) {
		bsock_close(bs);
		if (bs->close)
			(*bs->close)(bs, bs->arg);
		return;
	}
	bs->rbuf->tail += rc;

	if (bs->read)
		(*bs->read)(bs, bs->rbuf, bs->arg);

	return;

error:
	bsock_close(bs);
	if (bs->error)
		(*bs->error)(bs, bs->arg);
}

static void bsock_write_cb(EV_P_ ev_io *w, int revents)
{
	(void) revents;
	struct ev_bsock *bs;

	bs = w->data;
	log_debug("read callback for '%s:%s' on %d",
				bs->addr, bs->port, bs->fd);
}

static void bsock_connect_cb(EV_P_ ev_io *w, int revents)
{
	(void) revents;
	int rc, error;
	struct ev_bsock *bs;
	socklen_t errsz = sizeof(error);

	bs = w->data;
	ev_timer_stop(EV_A_ &bs->wtimer);
	ev_io_stop(EV_A_ &bs->wsock);

	rc = getsockopt(bs->fd, SOL_SOCKET, SO_ERROR, (void *)&error, &errsz);
	if (rc == -1) {
		log_error("getsockopt failed for '%s:%s' on %d",
				bs->addr, bs->port, bs->fd);
		bsock_close(bs);
		return;
	}

	if (error) {
		log_error("connect failed for '%s:%s' on %d, %s",
				bs->addr, bs->port, bs->fd, strerror(error));
		bsock_close(bs);
		return;
	}

	log_info("connected to '%s:%s' on %d.", bs->addr, bs->port, bs->fd);
	bs->state = BS_CONNECTED;

	/* setup the write io watcher */
	bs->wsock.data = bs;
	ev_init(&bs->wsock, bsock_write_cb);
	ev_io_set(&bs->wsock, bs->fd, EV_WRITE);

	/* setup the read io watcher */
	bs->rsock.data = bs;
	ev_init(&bs->rsock, bsock_read_cb);
	ev_io_set(&bs->rsock, bs->fd, EV_READ);

	/* kick off the read events */
	ev_io_start(EV_A_ &bs->rsock);

	if (bs->connect)
		(*bs->connect)(bs, bs->arg);
}

static void bsock_connect_timeout_cb(EV_P_ ev_timer *w, int revents)
{
	(void) revents;
	struct ev_bsock *bs;

	bs = w->data;
        log_error("connection timeout for '%s:%s' on %d",
			bs->addr, bs->port, bs->fd);
	bsock_close(bs);
}

int bsock_connect(struct ev_bsock *bs)
{
	int rc;
	struct addrinfo ai, *res;

	if (bs->state == BS_CONNECTED || bs->state == BS_CONNECTING)
		return 0;

	memset(&ai, 0, sizeof(struct addrinfo));
	ai.ai_family = AF_INET;
	ai.ai_socktype = SOCK_STREAM;

	rc = getaddrinfo(bs->addr, bs->port, &ai, &res);
	if (rc != 0) {
		log_error("getaddrinfo('%s:%s') failed, %s", bs->addr,
				bs->port, strerror(errno));
		return -1;
	}

	rc = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (rc == -1) {
		log_error("create socket('%s:%s') failed, %s", bs->addr,
				bs->port, strerror(errno));
		freeaddrinfo(res);
		return -1;
	}
	bs->fd = rc;

	rc = connect(bs->fd, res->ai_addr, res->ai_addrlen);
	if (rc == -1 && errno != EINPROGRESS) {
		close(bs->fd);
		freeaddrinfo(res);
		bs->fd = -1;
		log_error("connect('%s:%s') failed, %s", bs->addr,
				bs->port, strerror(errno));
		return 0;
	}

	freeaddrinfo(res);

	ev_init(&bs->wsock, bsock_connect_cb);
	ev_io_set(&bs->wsock, bs->fd, EV_WRITE);
	bs->wsock.data = bs;
	ev_io_start(EV_A_ &bs->wsock);

	ev_init(&bs->wtimer, bsock_connect_timeout_cb);
	ev_timer_set(&bs->wtimer, 2.0, 0.0);
	bs->wtimer.data = bs;
	ev_timer_start(EV_A_ &bs->wtimer);

	bs->state = BS_CONNECTING;

	return bs->fd;
}

#if 0
#define MIN(a, b) if ((a) > (b) ?

int bsock_write(struct ev_bsock *bs, void *data, int len)
{
	int n;
	struct ev_buff *buff;

	if (bs->state != BS_CONNECTED)
		return 0;
	log_debug("writing %d bytes starting at %p", len, data);
	buff = bs->wbuf;

	n = MIN(buff_left(buff), len);
	memcpy(buff->tail, data, n);



	buffer_add(buffsock->write_buf, data, len);
    ev_io_start(buffsock->loop, &buffsock->write_ev);

	n = buff_length(bs->wbuf);
	if (n > 0) {
        buffered_socket_write(buffsock, buf->data, n);
        buffer_reset(buf);
	}
}
#endif

void mini_read(struct ev_bsock *bs, struct ev_buff *buf, void *arg)
{
	(void) bs;
	(void) arg;
	(void) buf;

	buff_seal(buf);
	log_info("===> %s", buff_string(buf));
	buff_restart(buf);
}

void mini_write(struct ev_bsock *bs, void *arg)
{
	(void) bs;
	(void) arg;
}

void mini_connect(struct ev_bsock *bs, void *arg)
{
	(void) bs;
	(void) arg;
	log_info("=== CONNECT");
}

void mini_close(struct ev_bsock *bs, void *arg)
{
	(void) bs;
	(void) arg;
}

void mini_error(struct ev_bsock *bs, void *arg)
{
	(void) bs;
	(void) arg;
}

int main(int argc, char *argv[])
{
	EV_P EV_DEFAULT; /* OR ev_default_loop(0) */
	int daemonize = 1, rc;
	struct ev_bsock *bs;

	bs = bsock_create("127.0.0.1:8090", 4096, 4096);
	if (bs == NULL) {
		log_error("create buffered socket failed.");
		return -1;
	}

	bsock_set_read(bs, mini_read);
	bsock_set_write(bs, mini_write);
	bsock_set_connect(bs, mini_connect);
	bsock_set_close(bs, mini_close);
	bsock_set_error(bs, mini_error);

	rc = bsock_connect(bs);


	ev_run(EV_A_ 0);

	return -1;


















	const char *pidfile = PROJECT_PID_FILE;
	pid_t pid;

        signal(SIGSEGV, log_backtrace);
        //foo();


	while (1) {
		rc = getopt(argc, argv, "hfP:");
		if (rc == -1)
			break;
		switch (rc) {
		case 'P':
			pidfile = optarg;
			break;
		case 'f':
			daemonize = 0;
			break;
		case 'h':
			exit_usage(EXIT_SUCCESS);
		default:
			exit_usage(EXIT_FAILURE);
		}
	}
	if (optind < argc)
		exit_usage(EXIT_FAILURE);

        if (daemonize) {
                pid = fork();
                if (pid < 0) { /* error */
                        log_error("fork failed, %s", strerror(errno));
			exit(EXIT_FAILURE);
                } else if (pid != 0) {      /* parent */
                        log_info("Running (PID %i)", pid);
                        return 0;
                }
                setsid(); /* Detach from session */

                if (check_pidfile(pidfile) < 0) /* Write pidfile */
                        exit(EXIT_FAILURE);

                /* close standard descriptors */
                close(2);
                close(1);
                close(0);

                rc = open("/dev/null", O_RDWR);
                if (rc != 0) {
                        log_error("redirect 'STDIN' to '/dev/null' failed, rc %d", rc);
                        exit(EXIT_FAILURE);
                }

                rc = dup(0);
                if (rc != 1) {
                        log_error("redirect `STDOUT' to '/dev/null' failed, rc %d", rc);
                        exit(EXIT_FAILURE);
                }

                rc = dup(0);
                if (rc != 2) {
                        log_error("redirect 'STDERR' to '/dev/null' failed, rc %d", rc);
                        exit(EXIT_FAILURE);
                }
	}

	while (1) {
		sleep(1);
		log_info("sleep");
	};


        return 0;
}

