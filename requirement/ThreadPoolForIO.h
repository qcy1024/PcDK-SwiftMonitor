#ifndef __THREADPOOL_FOR_IO__H__
#define __THREADPOOL_FOR_IO__H__

#include "GUnlockPack.h"
#include "ProcIO.h"
#include <queue>
#include "CConfig.h"
#include <atomic>

class HttpMonitor;

class ThreadPoolForIO
{
    friend bool IO_t::putIOOpInQAndSignal(IOOp_t op);
    friend class HttpMonitor;
    struct threadItem 
    {
        pthread_t tid;
        bool isRunning;
        threadItem(pthread_t tid, bool isRunning) : tid(tid), isRunning(isRunning) {};
    };
public:
    ThreadPoolForIO(CConfig* p_ins_config);
    ~ThreadPoolForIO();
    bool createThreads();

private:
    int m_threadNum;
    int m_actThreadNum;
    static std::atomic<int> m_busyThreadNum;

    static void* threadFunction(void* data);
    std::vector<threadItem> m_threadList;

};



#endif

