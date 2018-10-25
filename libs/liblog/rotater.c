
#include "log.h"

#include <glob.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef FEATURE_LOG_PROCESS
static char *lock_file = "/tmp/log.lock";
static int lock_fd = -1;
#else
#include <pthread.h>
static pthread_mutex_t rotater_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

static int rotater_trylock(void)
{
#ifdef FEATURE_LOG_PROCESS
	struct flock fl = {
		.l_type   = F_WRLCK,
		.l_start  = 0,
		.l_whence = SEEK_SET,
		.l_len    = 0
	};

	if (fcntl(lock_fd, F_SETLK, &fl)) {
		/* lock by other process, EAGAIN(linux) EACCES(AIX) */
		if (errno == EAGAIN || errno == EACCES)
			_warn("fcntl lock fail, as file is lock by other process");
		else
			_error("lock fd(%d) fail, errno %d", lock_fd, errno);
		return -1;
	}
#else
	int rc;

	rc = pthread_mutex_trylock(&rotater_lock);
	if (rc == EBUSY) {
		_warn("pthread_mutex_trylock fail, locked by other threads");
		return -1;
	} else if (rc != 0) {
		_error("pthread_mutex_trylock fail, rc %d", rc);
		return -1;
	}
#endif

	return 0;
}

static int rotater_unlock(void)
{
	int rc = 0;

#ifdef FEATURE_LOG_PROCESS
	struct flock fl = {
		.l_type = F_UNLCK,
		.l_start = 0,
		.l_whence = SEEK_SET,
		.l_len = 0
	};

	if (fcntl(lock_fd, F_SETLK, &fl)) {
		rc = -1;
		_error("unlock fd(%d) fail, errno %d", lock_fd, errno);
	}
#else
	if (pthread_mutex_unlock(&rotater_lock)) {
		rc = -1;
		_error("pthread_mutext_unlock fail, errno[%d]", errno);
	}
#endif

	return rc;
}

void rotater_destory(void)
{
#ifdef FEATURE_LOG_PROCESS
	if (lock_fd > 0)
		close(lock_fd);
#endif

	return;
}

int rotater_init(void)
{
#ifdef FEATURE_LOG_PROCESS
	int fd = 0;

	fd = open(lock_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		_error("Failed to open(%s), errno %d", lock_file, errno);
		return -1;
	}

	lock_fd = fd;
#endif

	return 0;
}

int rotater_lsrm(const char *file)
{
	glob_t globbuf;
	struct stat statbuf;
	int rc;
	size_t pathc;
	char **pathv;

	char file_pattern[MAX_PATH_LEN];
	int nwrite;

	nwrite = snprintf(file_pattern, sizeof(file_pattern), "%s.[0-9]*-[0-9]*", file);
	if (nwrite < 0 || nwrite >= (int)sizeof(file_pattern)) {
		_error("snprintf() return %d, overflow or errno %d", nwrite, errno);
		return -1;
	}

	/*
	 * scan file which likes the following, and should sorted.
	 *   file.log.0506-221224
	 *   file.log.0506-221225
	 *   file.log.0506-221226
	 *   file.log.0506-221227
	 */
	rc = glob(file_pattern, GLOB_ERR | GLOB_MARK, NULL, &globbuf);
	if (rc == GLOB_NOMATCH) {
		_error("no match for glob(%s)", file_pattern);
		goto exit;
	} else if (rc) {
		_error("glob(%s) error, rc %d errno %d", file_pattern, rc, errno);
		return -1;
	}
	/* TODO: ensure the files sorted by time */

	pathv = globbuf.gl_pathv;
	pathc = globbuf.gl_pathc;

	for (; pathc-- > MAX_FILE_NUMS; pathv++) {
		rc = lstat(*pathv, &statbuf);
		if (rc < 0) {
			_error("lstat(%s) error, rc %d errono %", *pathv, rc, errno);
			continue;
		}

		if (S_ISREG(statbuf.st_mode)) {
			rc = remove(*pathv);
			if (rc < 0) {
				_error("remove(%s) error, rc %d errono %", *pathv, rc, errno);
				continue;
			}
		}
	}

exit:
	globfree(&globbuf);
	return 0;
}

int rotater_rotate(const char *file, long file_size)
{
	int rc = 0;
	struct stat info;
	char new_path[MAX_PATH_LEN - 1];

	assert(file);

	if (rotater_trylock()) {
		_warn("rotater trylock fail, maybe lock by other process or threads");
		return 0;
	}

	if (stat(file, &info)) {
		rc = -1;
		_error("stat(%s) fail, errno %d", file, errno);
		goto exit;
	}

	/* recheck it, not large enough, may rotate by others */
	if (info.st_size <= file_size) {
		rc = 0;
		_error("file size %d is smaller than %d", info.st_size, file_size);
		goto exit;
	}

	/* 1203-123535, 12-03 12:35:35 */
	char		  timestr[16];
	time_t		  tt;
	struct tm	  timenow;

	time(&tt);
	localtime_r(&tt, &timenow);
	rc = strftime(timestr, sizeof(timestr), "%m%d-%H%M%S", &timenow);
	assert(rc < sizeof(timestr) && rc != 0);

	/* do the base_path mv  */
	memset(new_path, 0, sizeof(new_path));
	rc = snprintf(new_path, sizeof(new_path), "%s.%s", file, timestr);
	if (rc < 0 || rc >= (int)sizeof(new_path)) {
		_error("new log file, nwirte %d, overflow or errno %d", rc, errno);
		return -1;
	}

	if (rename(file, new_path)) {
		_error("rename(%s)->(%s) fail, errno %d", file, new_path, errno);
		return -1;
	}

	/* begin list and move files */
	rc = rotater_lsrm(file);
	if (rc) {
		rc = -1;
		_error("rotater lsrm (%s) fail, rc %d", file, rc);
	}

exit:
	/* unlock file */
	if (rotater_unlock())
		_error("rotater_unlock fail");

	return rc;
}

