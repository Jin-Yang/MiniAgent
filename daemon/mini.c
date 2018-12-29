
#include "mini.h"
#include "pidfile.h"

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <execinfo.h>

#include "bsock.h"
#include "libev/ev.h"
#include "liblog/log.h"

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

#if 0
static void mini_read(struct ev_bsock *bs, struct ev_buff *buf, void *arg)
{
	(void) bs;
	(void) arg;
	(void) buf;

	buff_seal(buf);
	log_info("===> %s", buff_string(buf));
	buff_restart(buf);
}

static void mini_write(struct ev_bsock *bs, void *arg)
{
	(void) bs;
	(void) arg;
	log_info("=== WRITE");
}

static void mini_connect(struct ev_bsock *bs, void *arg)
{
	(void) bs;
	(void) arg;
	log_info("connected to '%s:%s' on %d.",
			bs->addr, bs->port, bs->fd);
}

static void mini_close(struct ev_bsock *bs, void *arg)
{
	(void) bs;
	(void) arg;
	log_info("buffer socket('%s:%s') closed", bs->addr, bs->port);
}

static void mini_error(struct ev_bsock *bs, int err, void *arg)
{
	(void) bs;
	(void) err;
	(void) arg;
	log_error("buffer socket('%s:%s') %s: %s",
		bs->addr, bs->port, bs->errmsg, strerror(bs->errnum));
}
#endif

static void repeate_foobar(EV_P_ ev_timer *w, int revents)
{
	log_info("repeat event %d, %p", revents, w);
}

static void signal_handler_cb(EV_P_ ev_signal *w, int revents)
{
	(void) revents;

	switch (w->signum) {
	case SIGINT:
		log_error("got SIGINT, quit now.");
		break;
	}

	ev_break(EV_A_ EVBREAK_ALL);
}

int main(int argc, char *argv[])
{
	EV_P EV_DEFAULT; /* OR ev_default_loop(0) */
	int daemonize = 1, rc;
	const char *pidfile = PROJECT_PID_FILE;
	const char *logfile = PROJECT_LOG_FILE;
	char *loglevel = "info";
	pid_t pid;
	ev_timer wtimer;
	struct ev_signal wsigint;

	while (1) {
		rc = getopt(argc, argv, "hfP:");
		if (rc == -1)
			break;
		switch (rc) {
		case 'P':
			pidfile = optarg;
			break;
                case 'l':
                        loglevel = optarg;
                        break;
                case 'L':
                        logfile = optarg;
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

        signal(SIGSEGV, log_backtrace);

	rc = log_init(logfile, log_get_level(loglevel));
	if (rc < 0)
		return -1;
	log_info("=================== BEGIN >>>>>>>>>>>>>>>>>>>");

	if (pidfile_check(pidfile) != 0) {
		fprintf(stderr, "Check PIDFile '%s' failed, details check the log.\n", pidfile);
		goto leave;
	}

        if (daemonize) {
                pid = fork();
                if (pid < 0) { /* error */
                        log_error("fork failed, rc %d %s", pid, strerror(errno));
			exit(EXIT_FAILURE);
                } else if (pid != 0) {      /* parent */
                        fprintf(stdout, "Running (PID %i)\n", pid);
                        return 0;
                }
                setsid(); /* Detach from session */

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

	if (pidfile_update(pidfile) < 0) {
		log_error("Create pidfile '%s' failed.", pidfile);
		fprintf(stderr, "Create pidfile '%s' failed.\n", pidfile);
		goto leave;
	}

	ev_signal_init(&wsigint, signal_handler_cb, SIGINT);
	ev_signal_start(EV_A_ &wsigint);
	ev_timer_init(&wtimer, repeate_foobar, 5., 1.);
	ev_timer_start(EV_A_ &wtimer);

	ev_run(EV_A_ 0);

	pidfile_destory(pidfile);

leave:
	log_info("=================== -END- <<<<<<<<<<<<<<<<<<<");

        return 0;
}

