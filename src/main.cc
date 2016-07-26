#include <sys/time.h>
#include <signal.h>
#include <sys/select.h>
#include <fcntl.h>
#include <linux/input.h>

#include <iostream>

#include "timer.h"
#include "singleton.h"
#include "smart_pointer.h"


#define TIMER_RESOLUTION        1000


using e7::common::array;
using e7::common::smart_pointer;
using e7::common::raii;
using e7::common::fdraii;


namespace test {
class hello_on_timeout : public u2ush::i_on_timeout
{
public:
    virtual void do_trigger(void)
    {
        u2ush::timer_heap *th = reinterpret_cast<u2ush::timer_heap *>(
            u2ush::g_singleton_mng.get_instance("u2ush::timer_heap")
        );

        struct tm tmp_tm;
        time_t tmp_now = ::u2ush::g_now / 1000;
        ::gmtime_r(&tmp_now, &tmp_tm);
        ::fprintf(
            stderr, "\r%lu -- %d.%02d.%02d %02d:%02d:%02d",
            tmp_now, tmp_tm.tm_year + 1900, 1 + tmp_tm.tm_mon, tmp_tm.tm_mday,
            tmp_tm.tm_hour + 8, tmp_tm.tm_min, tmp_tm.tm_sec
        );

        th->heap.push(smart_pointer<u2ush::timer>(
            new u2ush::timer(u2ush::g_now + 1000, new hello_on_timeout)
        ));
    }
};
}
using test::hello_on_timeout;


namespace u2ush {
uint64_t g_now;
e7::common::singleton_mng g_singleton_mng;

static void sigalrm_handler(int signum)
{
    static_cast<void>(::__sync_add_and_fetch (&g_now, 1));

    return;
}


static int init_signal(void)
{
    struct sigaction sa;

    sa.sa_flags = 0;
    sa.sa_handler = sigalrm_handler;   //信号处理函数

    if (-1 == ::sigaction(SIGALRM, &sa, NULL)) {
        return -1;
    }

    return 0;
}


int main(int argc, char *argv[], char *env[])
{
    int rslt;
    struct timeval tv;
    struct itimerval itm;
    timer_heap *th = reinterpret_cast<timer_heap *>(
        g_singleton_mng.get_instance("u2ush::timer_heap")
    );

    // 初始化信号
    if (-1 == u2ush::init_signal()) {
        return EXIT_FAILURE;
    }

    // 初始化定时器
    itm.it_interval = (struct timeval){0, TIMER_RESOLUTION};
    itm.it_value = (struct timeval){0, 20 * 1000};
    rslt = ::setitimer(ITIMER_REAL, &itm, NULL);
    if (-1 == rslt) {
        return EXIT_FAILURE;
    }

    rslt = ::gettimeofday(&tv, NULL);
    if (-1 == rslt) {
        return EXIT_FAILURE;
    }
    g_now = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    // 时钟驱动循环
    int capture_count = 0;
    while (true) {
        rslt = ::select(0, NULL, NULL, NULL, NULL);

        while (! th->heap.empty()) {
            smart_pointer<u2ush::timer> top(th->heap.top());

            if (top->timeout > g_now) {
                break;
            }

            top->trigger->do_trigger();
            th->heap.pop();
        }

        if (th->heap.empty()) {
            th->heap.push(smart_pointer<u2ush::timer>(
                new u2ush::timer(g_now, new hello_on_timeout)
            ));
        }
    }

    return EXIT_SUCCESS;
}
}


using u2ush::timer_heap;
using u2ush::g_singleton_mng;
class Ta {public:~Ta() {std::cout << "~Ta" << std::endl;}};
class Tb:public Ta {};


void some_test(void)
{
    timer_heap *th = reinterpret_cast<timer_heap *>(
        g_singleton_mng.get_instance("u2ush::timer_heap")
    );
    smart_pointer<u2ush::timer> spt1(
        new u2ush::timer(1, new hello_on_timeout)
    );
    smart_pointer<u2ush::timer> spt3(
        new u2ush::timer(3, new hello_on_timeout)
    );
    smart_pointer<u2ush::timer> spt2(
        new u2ush::timer(2, new hello_on_timeout)
    );

    th->heap.push(spt1);
    th->heap.push(spt2);
    th->heap.push(spt3);
    smart_pointer<u2ush::timer> spm1(th->heap.top());
    std::cout << spm1->timeout << std::endl;
    th->heap.pop();
    smart_pointer<u2ush::timer> spm2(th->heap.top());
    std::cout << spm2->timeout << std::endl;
    th->heap.pop();
    smart_pointer<u2ush::timer> spm3(th->heap.top());
    std::cout << spm3->timeout << std::endl;
    th->heap.pop();

    Ta *ta = new Ta[10];
    smart_pointer< array<Ta> > spmta(
        new array<Ta>(ta, 10)
    );
    Ta *trr = spmta->at(0);

    return;
}


int main(int argc, char *argv[], char *env[])
{
    int rslt = 0;

    // 初始化全局单例
    u2ush::g_singleton_mng.append(
        "u2ush::timer_heap", new u2ush::timer_heap
    );

    some_test();

    rslt = u2ush::main(argc, argv, env);

    return rslt;
}
