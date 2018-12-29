
#ifndef LIBLOG_LOG_H_
#define LIBLOG_LOG_H_ 1
//S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

#ifndef _GNU_SOURCE
/* support O_CLOEXEC */
#define _GNU_SOURCE
#endif

#include <syslog.h>
#include <stdarg.h>

#define MAX_PATH_LEN            1024
#define MAX_FILE_NUMS           4
#define MAX_LOG_FILE_SIZE       1024 * 1024 * 50
#define ERRBUF_SIZE             1024
#define LOG_BUFFER_SIZE         4096

#ifndef LOG_EMERG
        /* NOTE: Keep the following same with <syslog.h> file. */
        #define LOG_EMERG       0       /* system is unusable */
        #define LOG_ALERT       1       /* (used)action must be taken immediately, such as no memory */
        #define LOG_CRIT        2       /* critical conditions */
        #define LOG_ERR         3       /* (used)error conditions */
        #define LOG_WARNING     4       /* (used)warning conditions */
        #define LOG_NOTICE      5       /* normal but significant condition */
        #define LOG_INFO        6       /* (used)informational */
        #define LOG_DEBUG       7       /* (used)debug-level messages */
#endif
#define LOG_DEBUG0              8       /* (used)more details for debug-level */
#define LOG_TRACE               8       /* (used)more details for debug-level */
#define LOG_LEVEL_NUM           9
#define LOG_LEVEL_MAX           8
#define LOG_LEVEL_MIN           0
#define LOG_DEF_LEVEL           LOG_INFO

#ifdef FEATURE_LOG_PROCESS
#define log_init                log_init_process
#define log_it                  log_it_process
#define log_level_inc           log_inc_process
#define log_level_dec           log_dec_process
#elif defined FEATURE_LOG_STDOUT
#define log_init                log_init_stdout
#define log_it                  log_it_stdout
#define log_level_inc           log_inc_stdout
#define log_level_dec           log_dec_stdout
#elif defined FEATURE_LOG_THREAD
#define log_init                log_init_thread
#define log_it                  log_it_thread
#define log_level_inc           log_inc_thread
#define log_level_dec           log_dec_thread
#else
#include <stdio.h>
#define log_init(f, l)
#define _debug(...)            do { log_inner(LOG_DEBUG,   __VA_ARGS__); } while(0)
#define _warn(...)             do { log_inner(LOG_WARNING, __VA_ARGS__); } while(0)
#define _error(...)            do { log_inner(LOG_ERR,     __VA_ARGS__); } while(0)
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
	#define log_debug0(...)           do { printf("debug: " __VA_ARGS__); putchar('\n'); } while(0)
	#define log_debug(...)            do { printf("debug: " __VA_ARGS__); putchar('\n'); } while(0)
	#define log_info(...)             do { printf("info : " __VA_ARGS__); putchar('\n'); } while(0)
	#define log_warning(...)          do { printf("warn : " __VA_ARGS__); putchar('\n'); } while(0)
	#define log_error(...)            do { printf("error: " __VA_ARGS__); putchar('\n'); } while(0)
#elif defined __GNUC__
	#define log_debug0(fmt, args...)  do { printf("debug: " fmt, ## args); putchar('\n'); } while(0)
	#define log_debug(fmt, args...)   do { printf("debug: " fmt, ## args); putchar('\n'); } while(0)
	#define log_info(fmt, args...)    do { printf("info : " fmt, ## args); putchar('\n'); } while(0)
	#define log_warning(fmt, args...) do { printf("warn : " fmt, ## args); putchar('\n'); } while(0)
	#define log_error(fmt, args...)   do { printf("error: " fmt, ## args); putchar('\n'); } while(0)
#endif
#define LOG_MACRO
#endif

const char *log_get_name(const int level);
int log_get_level(const char *level);
void log_inner(int severity, const char *fmt, ...);
int log_buffer_format(char *buffer, int severity, const char *fmt, va_list ap);

#ifndef LOG_MACRO
int log_init_thread(const char *file, int level);
int log_init_process(const char *file, int level);
int log_init_stdout(const char *file, int loglevel);

void log_it_thread(int severity, const char * const fmt, ...);
void log_it_process(int severity, const char * const fmt, ...);
void log_it_stdout(int severity, const char * const fmt, ...);

/* more logs */
int log_dec_thread(void);
int log_dec_process(void);
int log_dec_stdout(void);

int log_inc_thread(void);
int log_inc_process(void);
int log_inc_stdout(void);

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
	#define _debug(...)            do { log_inner(LOG_DEBUG,   __VA_ARGS__); } while(0)
	#define _warn(...)             do { log_inner(LOG_WARNING, __VA_ARGS__); } while(0)
	#define _error(...)            do { log_inner(LOG_ERR,     __VA_ARGS__); } while(0)

	#define log_fatal(...)         do { log_it(LOG_CRIT,    __VA_ARGS__); } while(0)
	#define log_error(...)         do { log_it(LOG_ERR,     __VA_ARGS__); } while(0)
	#define log_warning(...)       do { log_it(LOG_WARNING, __VA_ARGS__); } while(0)
	#define log_notice(...)        do { log_it(LOG_NOTICE,  __VA_ARGS__); } while(0)
	#define log_info(...)          do { log_it(LOG_INFO,    __VA_ARGS__); } while(0)
	#define log_debug(...)         do { log_it(LOG_DEBUG,   __VA_ARGS__); } while(0)
	#define log_debug0(...)        do { log_it(LOG_DEBUG0,  __VA_ARGS__); } while(0)
	#define log_trace(...)         do { log_it(LOG_TRACE,   __VA_ARGS__); } while(0)
#elif defined __GNUC__
	#define _debug(fmt, args...)   do { log_inner(LOG_DEBUG,   fmt, ## args); } while(0)
	#define _warn(fmt, args...)    do { log_inner(LOG_WARNING, fmt, ## args); } while(0)
	#define _error(fmt, args...)   do { log_inner(LOG_ERR,     fmt, ## args); } while(0)

	#define log_fatal(...)         do { log_it(LOG_CRIT,    fmt, ## args); } while(0)
	#define log_error(...)         do { log_it(LOG_ERR,     fmt, ## args); } while(0)
	#define log_warning(...)       do { log_it(LOG_WARNING, fmt, ## args); } while(0)
	#define log_notice(...)        do { log_it(LOG_NOTICE,  fmt, ## args); } while(0)
	#define log_info(...)          do { log_it(LOG_INFO,    fmt, ## args); } while(0)
	#define log_debug(...)         do { log_it(LOG_DEBUG,   fmt, ## args); } while(0)
	#define log_debug0(...)        do { log_it(LOG_DEBUG0,  fmt, ## args); } while(0)
	#define log_trace(...)         do { log_it(LOG_TRACE,   fmt, ## args); } while(0)
#endif
#endif

#endif
