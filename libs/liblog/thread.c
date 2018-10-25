
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "log.h"
#include "rotater.h"

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

#define sstrerror          strerror_r

#define LOG_BUFFER_SIZE    4096
#define HTD_FSYNC_PERIOD   20

static char             filename[MAX_PATH_LEN];
static int		logfile        = -1;
static pthread_key_t    log_thread_key = -1;
static int              log_level      = LOG_DEBUG0;

/* If no memory, just ignore it */
void log_it_thread(int severity, const char * const fmt, ...)
{
        va_list ap;
        int rc;
	char *buffer;

        assert(fmt);
        assert(severity < LOG_LEVEL_NUM);

        if (severity > log_level)
                return;

	buffer = (char *)pthread_getspecific(log_thread_key);
        if (buffer == NULL) {
		if ((buffer = calloc(1, LOG_BUFFER_SIZE)) == NULL) {
			_error("out of memory");
			return;
		}

                rc = pthread_setspecific(log_thread_key, buffer);
                if (rc) {
			_error("set specific fail, errno %d", errno);
			free(buffer);
			buffer = NULL;
                        return;
                }
        }
	assert(buffer);

        va_start(ap, fmt);
        rc = log_buffer_format(buffer, severity, fmt, ap);
        va_end(ap);
	assert(rc > 0);

        write(logfile, buffer, rc);

	/* not so thread safe here, as multiple thread may ++fsync_count at the same time */
	static int fsync_count = 0;
	if (++fsync_count > HTD_FSYNC_PERIOD) {
		fsync_count = 0;
		fsync(logfile);

		if (logfile < 0) {
			logfile = open(filename, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
			_warn("logfile < 0, try to re-open(%s), rc = %d", filename, logfile);
		}

		struct stat info;

		if (stat(filename, &info)) {
			rc = -1;
			_error("stat(%s) fail, errno %d", filename, errno);
			return;
		}

		if (info.st_size <= MAX_LOG_FILE_SIZE) {
			_debug("file size %d is smaller than %d, just ignore", info.st_size, MAX_LOG_FILE_SIZE);
			return;
		}

		close(logfile);
		logfile = -1;

		rotater_rotate(filename, MAX_LOG_FILE_SIZE);

		/* ignore errors, retry it later */
		logfile = open(filename, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
		if (logfile < 0) {
			char errbuf[ERRBUF_SIZE];
			_error("open(%s) failed: %s", filename, sstrerror(errno, errbuf, sizeof(errbuf)));
		}

	}
}

static void thread_key_destroy(char *buffer)
{
        if (buffer == NULL)
                return;
        free(buffer);
}

static void log_destory(void)
{
	char *buffer;

        if (logfile > 0) {
                fsync(logfile);
                close(logfile);
                logfile = -1;
        }

        buffer = (char *)pthread_getspecific(log_thread_key);
        if (buffer != NULL)
                thread_key_destroy(buffer);
}

int log_init_thread(const char *file, int level)
{
        int rc;

        rc = pthread_key_create(&log_thread_key, (void(*)(void *))thread_key_destroy);
        if (rc) {
                char errbuf[ERRBUF_SIZE];
                fprintf(stderr, "ERROR: Create pthread key failed: %s.\n",
                                sstrerror(errno, errbuf, sizeof(errbuf)));
                return -1;
        }

	if (strlen(file) > MAX_PATH_LEN - 1) {
		fprintf(stderr, "ERROR: Length of file '%s' larger than %d bytes",
				file, MAX_PATH_LEN);
		return -1;
	}
	memcpy(filename, file, sizeof(filename) - 1);
	filename[MAX_PATH_LEN - 1] = 0;

        /* Register log_destory to be called at program termination */
        atexit(log_destory);

	rotater_rotate(filename, MAX_LOG_FILE_SIZE); /* try to rotate */

	/* try to open the logfile, flag O_CLOEXEC */
	logfile = open(filename, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
	if (logfile < 0) {
		char errbuf[ERRBUF_SIZE];
		fprintf(stderr, "ERROR: Open log file '%s' failed: %s\n",
				filename, sstrerror(errno, errbuf, sizeof(errbuf)));
		return -1;
	}
	if (level < 0)
		log_level = LOG_INFO;
	else
		log_level = level;

	rc = rotater_init();
	if (rc < 0)
		return -1;

	//log_it_thread(LOG_INFO, LOGC_INTERN, "Start thread logging mode");

        return 0;
}

