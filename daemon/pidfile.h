#ifndef PIDFILE_H_
#define PIDFILE_H_

int pidfile_check(const char *file);
int pidfile_update(const char *file);
void pidfile_destory(const char *file);

#endif
