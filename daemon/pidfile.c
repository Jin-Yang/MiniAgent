
#include "mini.h"
#include "liblog/log.h"

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>


int pidfile_check(const char *file)
{
	int fd, rc, pid;
	char buffer[128];

	fd = open(file, O_CLOEXEC | O_RDONLY, 0);
	if (fd < 0) {
		if (errno == ENOENT)
			return 0;
		log_error("open PIDFile %s failed, %s.", file, strerror(errno));
		return -1;
	}

	/* file already exists */
	log_warning("PIDFile '%s' already exists.", file);
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
	if (fd < 0 && errno == ENOENT) {
		log_info("PIDFile exists, but '/proc/%d/comm' doesn't.", pid);
		return 0;
	} else if (fd < 0) {
		log_info("open(/proc/%d/comm) failed, %s.", pid, strerror(errno));
		return -1;
	}

	/* file already exists, check it's command */
	rc = read(fd, buffer, sizeof(buffer) - 1);
	if (rc < 0) {
		log_error("read from '/proc/%d/comm' failed, %s.", pid, strerror(errno));
		close(fd);
		return -1;
	}
	buffer[rc - 1] = 0; /* skip '\n' */

	if (strcmp(PROJECT_NAME, buffer) == 0) {
		log_error("process(%d) already exists.", pid);
		close(fd);
		return -1;
	}
	log_warning("file '/proc/%d/comm' exists, but (" PROJECT_NAME ") != (%s).",
			pid, buffer);
	close(fd);

	return 0;
}


int pidfile_update(const char *file)
{
	int fd, rc;
	char buffer[128];

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

void pidfile_destory(const char *file)
{
	if (remove(file) < 0)
		log_warning("remove PIDFile '%s' failed, %s.", file, strerror(errno));
}

