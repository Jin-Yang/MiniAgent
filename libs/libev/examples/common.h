#ifndef COMMON_H_
#define COMMON_H_

#define HAVE_STRERROR_R   1

#include <stdlib.h>

char *sstrerror(int errnum, char *buf, size_t buflen);
void plugin_log(const char level, const char *format, ...);
#define ERROR(...)    plugin_log('E', __VA_ARGS__)
#define WARNING(...)  plugin_log('W', __VA_ARGS__)
#define NOTICE(...)   plugin_log('N', __VA_ARGS__)
#define INFO(...)     plugin_log('I', __VA_ARGS__)
#define DEBUG(...)    plugin_log('D', __VA_ARGS__)

#endif
