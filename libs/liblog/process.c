
#include "log.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#include "rotater.h"

#define LOG_FILE_MODE      (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define LOG_FILE_FLAG      (O_CLOEXEC | O_WRONLY | O_APPEND | O_CREAT)
#define PRO_FSYNC_PERIOD   20

static char filename[MAX_PATH_LEN];
static int log_level = LOG_DEF_LEVEL;

int log_init_process(const char *file, int level)
{
	int fd, rc;

	if (strlen(file) > MAX_PATH_LEN - 1) {
		fprintf(stderr, "ERROR: Length of file '%s' larger than %d bytes\n",
				file, MAX_PATH_LEN);
		return -1;
	}
	memcpy(filename, file, sizeof(filename) - 1);
	filename[MAX_PATH_LEN - 1] = 0;

	/* try to open the logfile */
	fd = open(filename, LOG_FILE_FLAG, LOG_FILE_MODE);
	if (fd < 0) {
		fprintf(stderr, "ERROR: Open log file '%s' failed: %s\n",
				filename, strerror(errno));
		return -1;
	}
	close(fd);
	if (level < 0)
		log_level = LOG_INFO;
	else
		log_level = level;

	rc = rotater_init();
	if (rc < 0)
		return -1;
	rotater_rotate(filename, MAX_LOG_FILE_SIZE); /* try to rotate */

	return 0;
}

void log_it_process(int severity, const char * const fmt, ...)
{
	int rc, fd;
	va_list ap;
        char buffer[LOG_BUFFER_SIZE];

        assert(fmt);
        assert(severity < LOG_LEVEL_NUM);

        if (severity > log_level)
                return;

	fd = open(filename, LOG_FILE_FLAG, LOG_FILE_MODE);
	if (fd < 0) { /* this should not happen, checked when log_init() */
		_error("open(%s) fail, errno %d", filename, errno);
		return;
	}

        va_start(ap, fmt);
	rc = log_buffer_format(buffer, severity, fmt, ap);
        va_end(ap);

        write(fd, buffer, rc);
	close(fd);

	static int fsync_count = 0;
	if (++fsync_count > PRO_FSYNC_PERIOD) {
		struct stat info;

		fsync_count = 0;
		if (stat(filename, &info)) {
			rc = -1;
			_error("stat(%s) fail, errno %d", filename, errno);
			return;
		}
		if (info.st_size <= MAX_LOG_FILE_SIZE) {
			_debug("file size %d is smaller than %d, just ignore", info.st_size, MAX_LOG_FILE_SIZE);
			return;
		}
		rotater_rotate(filename, MAX_LOG_FILE_SIZE);
	}
}

