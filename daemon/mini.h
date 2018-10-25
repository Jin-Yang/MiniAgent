#ifndef MINIC_H_
#define MINIC_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "config.h"

#include <stdio.h>

#ifndef PROJECT_NAME
#define PROJECT_NAME      "MiniAgent"
#endif

#ifndef PROJECT_VERSION
#define PROJECT_VERSION   "0.1.0"
#endif

#ifndef PROJECT_PID_FILE
#define PROJECT_PID_FILE  "MiniAgent.pid"
#endif

#ifndef PROJECT_LOG_FILE
#define PROJECT_LOG_FILE  "MiniAgent.log"
#endif

/*
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
	#define log_debug(...)           do { printf("debug: " __VA_ARGS__); putchar('\n'); } while(0);
	#define log_info(...)            do { printf("info : " __VA_ARGS__); putchar('\n'); } while(0);
	#define log_warn(...)            do { printf("warn : " __VA_ARGS__); putchar('\n'); } while(0);
	#define log_error(...)           do { printf("error: " __VA_ARGS__); putchar('\n'); } while(0);
#elif defined __GNUC__
	#define log_debug(fmt, args...)  do { printf("debug: " fmt, ## args); putchar('\n'); } while(0);
	#define log_info(fmt, args...)   do { printf("info : " fmt, ## args); putchar('\n'); } while(0);
	#define log_warn(fmt, args...)   do { printf("warn : " fmt, ## args); putchar('\n'); } while(0);
	#define log_error(fmt, args...)  do { printf("error: " fmt, ## args); putchar('\n'); } while(0);
#endif
*/


#endif
