/* catch a SIGINT signal, ctrl-c */
#include "../ev.h"
#include <stdio.h>
#include <signal.h>

ev_signal signal_watcher;

static void sigint_cb(EV_P_ ev_signal *w, int revents)
{
	(void) w;
	(void) revents;

	puts("catch SIGINT");
	ev_break(EV_A_ EVBREAK_ALL);
}

int main (void)
{
	// use the default event loop unless you have special needs
        EV_P EV_DEFAULT; /* OR ev_default_loop(0) */

	ev_signal_init(&signal_watcher, sigint_cb, SIGINT);
	ev_signal_start(EV_A_ &signal_watcher);

	// now wait for events to arrive
	ev_run(EV_A_ 0);

	// break was called, so exit
	return 0;
}

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <ev.h>

struct ev_loop *loop = NULL;
static ev_async async_watcher;

static void async_cb(EV_P_ ev_async *w, int revents)
{
        static int cb_count = 0;
        printf("async_cb() call, cb_count = %d\n", cb_count++);
}

void *ev_create(void *p)
{
        printf("ev_create() call, start!\n");
        loop = ev_loop_new(EVFLAG_AUTO);
        ev_async_init(&async_watcher, async_cb);
        ev_async_start(loop, &async_watcher);
        ev_run(loop, 0); // 如果不在回调中调用stop或者break方法，ev_run后面的代码就永远不会被执行。
        printf("ev_create() call, ev_run end!\n");
}

int main()
{
        pthread_t tid;
        pthread_create(&tid, NULL, ev_create, NULL);
        sleep(1); // 要等子线程先注册完事件之后，才可以调用下面的ev_async_send函数，否则你懂的。
        while(1)
        {
                static int num = 0;
                printf("num = %d\n", num);
                ev_async_send(loop, &async_watcher); // 既然可以在这里ev_async_send，那么也可以在其他需要的地方ev_async_send
                sleep(2);
                num++;
        }
        return 0;
}

