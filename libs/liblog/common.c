
#include "log.h"

#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <sys/syscall.h>


int log_buffer_format(char *buffer, int severity, const char *fmt, va_list ap)
{
        static char _log[LOG_LEVEL_NUM] = { 'E', 'A', 'C', 'E', 'W', 'N', 'I', 'D', '0' };
	int offset, rc;

#if 0
	struct   timeval   curt;
	gettimeofday(&curt, 0);
	rc = snprintf(buffer + offset, sizeof(buffer) - offset, "%ld.%03ld ",
		       	curt.tv_sec, curt.tv_usec / 1000);
	assert(rc < sizeof(buffer) - offset);
#else
        time_t     tt;
        struct tm  timenow;
        time(&tt);
        localtime_r(&tt, &timenow);
        rc = strftime(buffer, LOG_BUFFER_SIZE, "%m-%d %T ", &timenow);
	assert(rc < LOG_BUFFER_SIZE && rc != 0);
#endif
	offset = rc;

        rc = snprintf(buffer + offset, LOG_BUFFER_SIZE - offset, "%c (%d) ",
			_log[severity], (pid_t)syscall(SYS_gettid));
	assert(rc < LOG_BUFFER_SIZE - offset);
	offset += rc;

        rc = vsnprintf(buffer + offset, LOG_BUFFER_SIZE - offset, fmt, ap);
	if (rc >= LOG_BUFFER_SIZE - offset) {
                char msg[] = "...";
		memcpy(buffer + LOG_BUFFER_SIZE - sizeof(msg), msg, sizeof(msg) - 1);
		offset = LOG_BUFFER_SIZE - 1;
	} else if (rc < 0) {
                char msg[] = "log vsnprintf failed";
		memcpy(buffer + offset, msg, sizeof(msg) - 1);
		offset += (sizeof(msg) - 1);
	} else {
		offset += rc;
        }
	buffer[offset] = '\n';

        return offset + 1;
}

int log_get_level(const char *level)
{
	if (level == NULL)
		return -1;
	else if (strcasecmp(level, "emerg") == 0)
		return LOG_EMERG;
	else if (strcasecmp(level, "alert") == 0)
		return LOG_ALERT;
	else if (strcasecmp(level, "error") == 0)
		return LOG_ERR;
	else if (strcasecmp(level, "warning") == 0)
		return LOG_WARNING;
	else if (strcasecmp(level, "info") == 0)
		return LOG_INFO;
	else if (strcasecmp(level, "debug") == 0)
		return LOG_DEBUG;
	else if (strcasecmp(level, "debug0") == 0)
		return LOG_DEBUG0;
	else if (strcasecmp(level, "trace") == 0)
		return LOG_TRACE;

	return -1;
}

const char *log_get_name(const int level)
{
	static const char *lvlname[] = {
		"emerg", "alert", "critical", "error",
		"warning", "notice", "info", "debug", "trace"
	};

	if (level < LOG_LEVEL_MIN || level > LOG_LEVEL_MAX)
		return "invalid";

	return lvlname[level];
}

void log_inner(int severity, const char *fmt, ...)
{
	va_list args;
	char time_str[64];
	FILE *fp = NULL;
	time_t tt;
	struct tm local_time;
	char *log_profile, *log_level = "info";
	int level = LOG_INFO;

	static char *_log[] = { "DEBUG", "WARN", "ERROR" };

        assert(severity < LOG_LEVEL_NUM);

	log_profile = getenv("LOG_PROFILE");
	if (log_profile == NULL)
		return;

	log_level = getenv("LOG_PROFILE_LEVEL");
	if (log_level)
		level = log_get_level(log_level);
        if (severity > level)
                return;

	fp = fopen(log_profile, "a");
	if (fp == NULL)
		return;

	time(&tt);
	localtime_r(&tt, &local_time);
	strftime(time_str, sizeof(time_str), "%m-%d %T", &local_time);
	fprintf(fp, "%s %s (%d) ", time_str, _log[severity],
				(pid_t)syscall(SYS_gettid));

	va_start(args, fmt);
	vfprintf(fp, fmt, args);
	va_end(args);
	fprintf(fp, "\n");

	fclose(fp);

	return;
}

