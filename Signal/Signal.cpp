#include "Signal.h"
#include <glog/logging.h>
#include <sys/signalfd.h>
#define SIGNO_MAX 128

namespace IOEvent
{
struct SignalData
{
    Signal::SignalHandler signalCallback;
    uint16_t signo;
    bool activated;
};

std::vector<SignalData> Signal::signals_;
int32_t Signal::lock_ = 0;
Signal::Signal()
    :signalFd_(0)
{
    signals_.reserve(SIGNO_MAX);
}

Signal::~Signal()
{

}

Signal::SignalHandler Signal::signalSet(int32_t _signo, SignalHandler _callBack)
{
    signals_[_signo].signalCallback = _callBack;
    signals_[_signo].activated = true;
    signals_[_signo].signo = _signo;
    signalSet(_signo, signalAsyncHandler, 0);
}


Signal::SignalHandler Signal::signalSet(int32_t _signo, SignalHandler _callBack,  int32_t _mask)
{
    if(_callBack == NULL)
    {
        // ignore
        _callBack = SIG_IGN;
    }
    else if((long)_callBack == -1)
    {
        // clear
        _callBack = SIG_DFL; 
    }

    struct sigaction act{};
    struct sigaction oact{};
    act.sa_handler = _callBack;
    if(_mask)
    {
        sigfillset(&act.sa_mask);
    }
    else
    {
        sigemptyset(&act.sa_mask);
    }
    act.sa_flags = 0;
    if(sigaction(_signo, &act, &oact) < 0)
    {
        return nullptr;
    }
    return oact.sa_handler;
}

Signal::SignalHandler Signal::signalGetHandler(int32_t _signo)
{
    if(_signo > SIGNO_MAX)
    {
        LOG(WARNING) << "signal numberis invalid signo: " << _signo;
        return nullptr;
    }
    return signals_[_signo].signalCallback;
}

void Signal::signalClear(void)
{  
    for (auto i = 0; i < SIGNO_MAX; i++) {
        if (signals_[i].activated) {  
            signalSet(signals_[i].signo, (SignalHandler) -1, 0);    
        }
    }
    signals_.clear();
}

void Signal::signalNone(void)
{
    sigset_t mask;
    sigfillset(&mask);
    int ret = pthread_sigmask(SIG_BLOCK, &mask, nullptr);
    if (ret < 0)
    {
        LOG(WARNING) << "pthread_sigmask() failed";
    }
}

int32_t Signal::kill(pid_t _pid, int32_t _sig) 
{
    if (_pid <= 0) 
    {
        return -1;
    }
    return kill(_pid, _sig);
}

void Signal::signalAsyncHandler(int32_t _signo)
{
    if(lock_)
    {
        //discard signal
        return;
    }
    lock_ = 1;
    signalCallback(_signo);
    lock_ = 0;
}

void Signal::signalCallback(int32_t _signo)
{
    if (_signo >= SIGNO_MAX) {
        LOG(WARNING) << "signal numberis invalid signo: " << _signo;
        return;
    }
    SignalHandler callback = signals_[_signo].signalCallback;
    if (!callback) {
        LOG(ERROR) << "signal callback nullptr signo : " << _signo;
        return;
    }
    callback(_signo);
}


}