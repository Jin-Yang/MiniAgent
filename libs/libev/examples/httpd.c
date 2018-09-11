
#include "libev/ev.h"
#include "common.h"

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>
#include <netinet/in.h>

#define USE_UNIX_SOCKET 0

#define SOCK_PATH       "/tmp/libev.sock"
#define LISTEN_PORT     8080
#define BUFFER_SIZE     4096

/* The connection states. */
#define CNST_FREE        0
#define CNST_READING     1
#define CNST_SENDING     2
#define CNST_PAUSING     3
#define CNST_LINGERING   4

/* Not all resposne codes are listed, as some of them may not have sense here. */
enum response_codes_e {
	HTTP_OK = 0,          /* 200 */
	HTTP_BAD_REQUEST,     /* 400 */
	HTTP_UNAUTHORIZED,    /* 401 */
	HTTP_FORBIDDEN,       /* 403 */
	HTTP_NOT_FOUND,       /* 404 */
	HTTP_INTERNAL_ERROR,  /* 500 */
	HTTP_NOT_IMPLEMENTED  /* 501 */
};

const char *CODES[] = {
	"200 OK",
	"400 BAD REQUEST",
	"401 UNAUTHORIZED",
	"403 FORBIDDEN",
	"404 NOT FOUND",
	"500 INTERNAL ERROR",
	"501 NOT IMPLEMENTED"
};

struct connection {
	char* read_buf;
	size_t read_size, read_idx, checked_idx;
	int state;
};


/* Total number of connected clients */
int total_clients = 0;

void httpd_realloc(char **dest, size_t *maxsize, size_t size)
{
	char *tmp;

	tmp = (char *)realloc(*dest, size);
	if (tmp == NULL)
		return;

	*dest = tmp;
	*maxsize = size;
}

static void finish_connection(struct ev_io *watcher)
{
	struct connection *conn = (struct connection *)watcher->data;

	/* write response if needed. */

	if (conn) {
		if (conn->read_buf)
			free(conn->read_buf);
		free(conn);
	}

	close(watcher->fd);
	ev_io_stop(EV_P watcher);
	free(watcher);
	total_clients --; /* Decrement total_clients count */
	printf("%d client(s) connected.\n", total_clients);
}


/* Write the requested buffer completely, accounting for interruptions. */
static int httpd_write_fully(int fd, const char* buf, size_t nbytes)
{
	size_t nwritten;

	nwritten = 0;
	while (nwritten < nbytes) {
		int r;

		r = write(fd, buf + nwritten, nbytes - nwritten);
		if (r < 0 && (errno == EINTR || errno == EAGAIN)) {
			usleep(3000);
			continue;
		} else if ( r < 0 ) {
			return r;
		} else if ( r == 0 ) {
			break;
		}
		nwritten += r;
	}

	return nwritten;
}

void httpd_send_err(int fd, int status, const char *msg)
{
        char buffer[BUFFER_SIZE];
	int nbytes;

	nbytes = snprintf(buffer, sizeof(buffer),
		"HTTP/1.0 %s\r\nContent-Type: text/html\r\n"
		"\r\n<html><title>%s</title><body><p>%s</p></body></html>\r\n",
		CODES[status], CODES[status], msg);
	if (nbytes < 0) {
		ERROR("Failed when try to format error message");
		return;
	} else if (nbytes > BUFFER_SIZE) {
		WARNING("Message is too long to format it");
		return;
	}

	httpd_write_fully(fd, buffer, nbytes);
}

void read_cb(EV_P struct ev_io *w, int revents)
{
        ssize_t read;
        char *buffer;
	struct connection *c = (struct connection *)w->data;

	assert(c);

        if (EV_ERROR & revents) {
                ERROR("Got invalid event");
                return;
        }

	/* Is there room in our buffer to read more bytes? */
	if (c->read_idx >= c->read_size) {
		if (c->read_size > 4096) {
			httpd_send_err(w->fd, 400, "Request body is too large");
			finish_connection(w);
			return;
		}
		httpd_realloc(&c->read_buf, &c->read_size, c->read_size + 512);
	}

	buffer = c->read_buf;
	if (buffer == NULL) {
		ERROR("Get internal error when using buffer");
		finish_connection(w);
		return;
	}
	buffer = c->read_buf + c->read_idx;

        /* Receive message from client socket */
        read = recv(w->fd, buffer, c->read_size - c->read_idx, 0);
        if (read < 0) {
                char errbuf[1024];
                ERROR("Failed to receive data through socket: %s", sstrerror(errno, errbuf, sizeof(errbuf)));
                return;
        } else if (read == 0) {
                /* Stop and free watcher if client socket is closing */
		finish_connection(w);
                printf("Peer might closing\n");
                return;
        } else {
                printf("Message: %s\n", buffer);
        }

	httpd_send_err(w->fd, HTTP_OK, buffer);

	finish_connection(w);
}

/* Accept client requests */
void accept_cb(EV_P_ struct ev_io *watcher, int revents)
{
#if USE_UNIX_SOCKET
        int sockfd;
        struct ev_io *w_client;

        if (EV_ERROR & revents) {
                ERROR("Got invalid event in accept callback routine");
                return;
        }

        sockfd = accept(watcher->fd, NULL, NULL);
        if (sockfd < 0) {
                char errbuf[1024];
                if (errno == EINTR)
                        return;
                ERROR("accept failed: %s", sstrerror(errno, errbuf, sizeof(errbuf)));
                close(watcher->fd);
        }

        total_clients ++;
        printf("Successfully connected with client.\n");
        printf("%d client(s) connected.\n", total_clients);

        // Initialize and start watcher to read client requests
        w_client = (struct ev_io*)malloc(sizeof(struct ev_io));
        ev_io_init(w_client, read_cb, sockfd, EV_READ);
        ev_io_start(EV_P w_client);
#else
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);
        int sockfd;
        struct ev_io *w_client = NULL;
	struct connection *conn = NULL;

        if (EV_ERROR & revents) {
                ERROR("Got invalid event in accept callback routine");
                return;
        }

        /* Accept client request */
        sockfd = accept(watcher->fd, (struct sockaddr *)&cliaddr, &clilen);
        if (sockfd < 0) {
                char errbuf[1024];
                ERROR("Failed to accept a new connection: %s", sstrerror(errno, errbuf, sizeof(errbuf)));
                return;
        }

        // Initialize and start watcher to read client requests
        w_client = (struct ev_io*)calloc(1, sizeof(struct ev_io));
        conn = (struct connection*)calloc(1, sizeof(struct connection));
	if (w_client == NULL || conn == NULL) {
		ERROR("Failed to allocate connection structure");
		free(w_client);
		free(conn);
	}
	w_client->data = conn;
	conn->state = CNST_READING;

        total_clients ++;
        printf("Successfully connected with client.\n");
        printf("%d client(s) connected.\n", total_clients);

        ev_io_init(w_client, read_cb, sockfd, EV_READ);
        ev_io_start(EV_P w_client);
#endif
}

static int initialize_listen_socket(void)
{
#if USE_UNIX_SOCKET
        int                     ret, flags;
        int                     listen_fd;
        struct sockaddr_un      servaddr;

        listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);   // Create server socket.
        if (listen_fd < 0) {
                char errbuf[1024];
                ERROR("Failed to create listen socket: %s", sstrerror(errno, errbuf, sizeof(errbuf)));
                return -1;
        }

        // Set the listen file descriptor to no-delay/non-blocking mode.
        flags = fcntl(listen_fd, F_GETFL, 0);
        if (flags < 0) {
                char errbuf[1024];
                ERROR("Failed to get listen fd flags: %s", sstrerror(errno, errbuf, sizeof(errbuf)));
                close( listen_fd );
                return -1;
	}
        if (fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK) < 0 ) {
                char errbuf[1024];
                ERROR("Failed to set 'O_NONBLOCK' flag: %s", sstrerror(errno, errbuf, sizeof(errbuf)));
                close( listen_fd );
                return -1;
	}

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sun_family      = AF_UNIX;
        strncpy(servaddr.sun_path, SOCK_PATH, sizeof(servaddr.sun_path));

        errno = 0;
        ret = unlink(SOCK_PATH);
        if ((ret != 0) && (errno != ENOENT)) {
                char errbuf[1024];
                WARNING("Deleting socket file \"%s\" failed: %s",
                        servaddr.sun_path, sstrerror(errno, errbuf, sizeof(errbuf)));
        } else if (ret == 0) {
                INFO("Successfully deleted socket file \"%s\".", servaddr.sun_path);
        }

        // Bind socket to address.
        ret = bind(listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
        if (ret < 0) {
                char errbuf[1024];
                ERROR("Failed to bind address: %s", sstrerror(errno, errbuf, sizeof(errbuf)));
	        close(listen_fd);
                return ret;
        }

        // Start a listen going.
        if (listen(listen_fd, 1024) < 0) {
                char errbuf[1024];
                ERROR("Failed to start a listen socket: %s", sstrerror(errno, errbuf, sizeof(errbuf)));
                close(listen_fd);
                return -1;
	}
#else
        int                     ret, flags, opt;
        int                     listen_fd;
        struct sockaddr_in      servaddr;

        listen_fd = socket(PF_INET, SOCK_STREAM, 0);   // Create server socket.
        if (listen_fd < 0) {
                char errbuf[1024];
                ERROR("Failed to create listen socket: %s", sstrerror(errno, errbuf, sizeof(errbuf)));
                return -1;
        }

        fcntl(listen_fd, F_SETFD, 1);           // Open close-on-exec flag.

        // Set the listen file descriptor to no-delay/non-blocking mode.
        flags = fcntl(listen_fd, F_GETFL, 0);
        if (flags < 0) {
                char errbuf[1024];
                ERROR("Failed to get listen fd flags: %s", sstrerror(errno, errbuf, sizeof(errbuf)));
                close( listen_fd );
                return -1;
	}
        if (fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK) < 0 ) {
                char errbuf[1024];
                ERROR("Failed to set 'O_NONBLOCK' flag: %s", sstrerror(errno, errbuf, sizeof(errbuf)));
                close( listen_fd );
                return -1;
	}

        // Allow reuse of local addresses.
        opt = 1;
        ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
        if (ret < 0) {
                char errbuf[1024];
                ERROR("Failed to set socket options 'SO_REUSEADDR': %s", sstrerror(errno, errbuf, sizeof(errbuf)));
	        close(listen_fd);
                return ret;
        }

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family      = AF_INET;           // AF-address family. PF-protocol family.
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // "0.0.0.0"
        servaddr.sin_port        = htons(LISTEN_PORT);

        // Bind socket to address.
        ret = bind(listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
        if (ret < 0) {
                char errbuf[1024];
                ERROR("Failed to bind address: %s", sstrerror(errno, errbuf, sizeof(errbuf)));
	        close(listen_fd);
                return ret;
        }

	/* dynamically allocating a port
	if (LISTEN_PORT[0] == 0) {
		int len = sizeof(servaddr);
		ret = getsockname(listen_fd, (struct sockaddr *)&servaddr, &len);
		if (ret < 0) {
			char errbuf[1024];
			ERROR("Failed to bind address: %s", sstrerror(errno, errbuf, sizeof(errbuf)));
			close(listen_fd);
			return ret;
		}
		// *port = ntohs(name.sin_port);
	}
	 */

        // Start a listen going.
        if (listen(listen_fd, 1024) < 0) {
                char errbuf[1024];
                ERROR("Failed to start a listen socket: %s", sstrerror(errno, errbuf, sizeof(errbuf)));
                close(listen_fd);
                return -1;
	}
#endif

        return listen_fd;
}

int main(void)
{
	EV_P EV_DEFAULT;
        int listen_fd;
        struct ev_io socket_accept;

        listen_fd = initialize_listen_socket();

        // Initialize and start a watcher to accepts client requests
        ev_io_init(&socket_accept, accept_cb, listen_fd, EV_READ);
        ev_io_start(EV_A_ &socket_accept);

        puts("Socket server starting ...");

        ev_loop(EV_A_ 0);

        close(listen_fd);

        return 0;
}

