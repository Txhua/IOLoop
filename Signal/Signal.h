#ifndef _IOEVENT_SIGNAL_H
#define _IOEVENT_SIGNAL_H

#include <boost/noncopyable.hpp>
#include <stdint.h>
#include <vector>
#include <signal.h>


namespace IOEvent
{

enum Signo
{
    SIGNO_PIPE = SIGPIPE,
    SIGNO_HUP = SIGHUP,
    SIGNO_HLD = SIGCHLD,
    SIGNO_IO = SIGIO,
    SIGNO_SR1 = SIGUSR1,
    SIGNO_SR2 = SIGUSR2,
    SIGNO_TERM = SIGTERM,
};

struct SignalData;

class Signal final : boost::noncopyable
{
public:
    typedef void (*SignalHandler)(int);
public:
    Signal();
    ~Signal();
public:
    SignalHandler signalSet(int32_t _signo, SignalHandler _callBack);
    SignalHandler signalSet(int32_t _signo, SignalHandler _callBack,  int32_t _mask);
    SignalHandler signalGetHandler(int32_t _signo);
    void signalClear(void);
    // 如果当前的线程不想要被信号中断,那么就可以使用signalNone函数屏蔽所有的信号,
    // 这样该进程所有的函数都不会被信号中断，编写函数的时候就不用考虑被信号打断的情况。
    // 值得注意的是处理的信号 SIGKILL 和 SIGSTOP 无法被阻塞。
    void signalNone(void);
    static int32_t kill(pid_t _pid, int32_t _sig);
private:
    static void signalAsyncHandler(int32_t _signo);
    static void signalCallback(int32_t _signo);
private:
    static std::vector<SignalData> signals_;
    static int32_t lock_;
};


}










#endif // !_IOEVENT_SIGNAL_H