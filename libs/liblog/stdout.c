
#include "log.h"

#include <unistd.h>

static int log_level = LOG_INFO;

void log_it_stdout(int severity, const char *const fmt, ...)
{
	int rc;
        char buffer[LOG_BUFFER_SIZE];
	va_list ap;

        if (severity > log_level)
                return;

        va_start(ap, fmt);
	rc = log_buffer_format(buffer, severity, fmt, ap);
        va_end(ap);

        write(STDOUT_FILENO, buffer, rc);
}

int log_init_stdout(const char *file, int level)
{
	(void) file;
	if (level < 0)
		log_level = LOG_INFO;
	else
		log_level = level;
	return 0;
}

/* more logs */
int log_dec_stdout(void)
{
	if (log_level < LOG_LEVEL_MAX)
		log_level++;
	return log_level;
}

int log_inc_stdout(void)
{
	if (log_level > LOG_LEVEL_MIN)
		log_level--;
	return log_level;
}

