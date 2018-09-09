
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


int main(int argc, char *argv[])
{
	const char *pidfile = PROJECT_PID_FILE;
	int daemonize = 1, rc;
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

