
#include <libev/ev.h>       // a single header file is required
#include <time.h>     // a single header file is required
#include <stdio.h>    // for puts

ev_periodic periodic_timer;

void periodic_action(EV_P_ ev_periodic *w, int revents)
{
        (void) w;
        (void) revents;
        time_t now = time(NULL);
        printf("current time is %s", ctime(&now));
}

static ev_tstamp rescheduler(ev_periodic *w, ev_tstamp now)
{
        (void) w;
        return now + 60;
}

int main(void)
{
        time_t now = time(NULL);
        EV_P EV_DEFAULT; /* OR ev_default_loop(0) */

        // 调用rescheduler()返回下次执行的时间，如果存在回调函数，则会忽略其它参数
        // 包括offset+interval，其输出示例如下：
        //     begin time is Fri Apr 14 21:51:47 2016
        //   current time is Fri Apr 14 21:52:47 2016
        //   current time is Fri Apr 14 21:53:47 2016
        //   current time is Fri Apr 14 21:54:47 2016
        ev_periodic_init(&periodic_timer, periodic_action, 0, 1, rescheduler);

        // 一般offset在[0, insterval]范围内，如下，也就是在最近的一个5秒整触发第一
        // 次回调函数，其输出示例如下：
        //     begin time is Fri Apr 21 23:24:18 2016
        //   current time is Fri Apr 21 23:24:25 2016
        //   current time is Fri Apr 21 23:24:35 2016
        //   current time is Fri Apr 21 23:24:45 2016
        //ev_periodic_init(&periodic_timer, periodic_action, 5, 10, NULL);

        // 如下只执行一次，也就是在20秒后触发
        //     begin time is Fri Apr 21 23:27:04 2016
        //   current time is Fri Apr 21 23:27:24 2016
        //ev_periodic_init(&periodic_timer, periodic_action, now+20, 0, NULL);      //3

        ev_periodic_start(EV_A_ &periodic_timer);

        printf("  begin time is %s", ctime(&now));

        ev_run (EV_A_ 0);

        return 0;
}
