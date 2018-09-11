
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>

char *sstrncpy(char *dest, const char *src, size_t n)
{
        strncpy(dest, src, n);
        dest[n - 1] = '\0';
        return (dest);
}

void plugin_log(const char level, const char *format, ...)
{
        char msg[1024];
        va_list ap;

        va_start(ap, format);
        vsnprintf(msg, sizeof(msg), format, ap);
        msg[sizeof(msg) - 1] = '\0';
        va_end(ap);

        fprintf(stderr, "%c %s\n", level, msg);
        return;
}

#if !HAVE_STRERROR_R
static pthread_mutex_t strerror_r_lock = PTHREAD_MUTEX_INITIALIZER;
#endif
char *sstrerror(int errnum, char *buf, size_t buflen)
{
        buf[0] = '\0';
#if !HAVE_STRERROR_R
        char *temp;
        pthread_mutex_lock(&strerror_r_lock);
        temp = strerror(errnum);
        sstrncpy(buf, temp, buflen);
        pthread_mutex_unlock(&strerror_r_lock);
#elif STRERROR_R_CHAR_P
        char *temp;
        temp = strerror_r(errnum, buf, buflen);
        if (buf[0] == '\0') {
                if ((temp != NULL) && (temp != buf) && (temp[0] != '\0'))
                        sstrncpy(buf, temp, buflen);
                else
                        sstrncpy(buf, "strerror_r did not return an error message", buflen);
        }
#else
        if (strerror_r(errnum, buf, buflen) != 0) {
                ssnprintf(buf, buflen, "Error #%i; Additionally, strerror_r failed.", errnum);
        }
#endif
        return buf;
}
