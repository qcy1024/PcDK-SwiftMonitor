#ifndef __THREADPOOL_FOR_IO__H__
#define __THREADPOOL_FOR_IO__H__

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
        ThreadPoolForIO* BelongTo;
        threadItem(ThreadPoolForIO* p_thPoolIO, bool isRunning) : BelongTo(p_thPoolIO), isRunning(isRunning) {};
    };
public:
    ThreadPoolForIO(const CConfig* p_ins_config);
    ~ThreadPoolForIO();
    bool createThreads();

private:
    int m_threadNum;
    int m_actThreadNum;
    static std::atomic<int> m_busyThreadNum;

    static void* threadFunction(void* data);
    std::vector<threadItem*> m_threadList;

};



#endif

