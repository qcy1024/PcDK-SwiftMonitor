#include "ThreadPoolForIO.h"
#include <unistd.h>

std::atomic<int> ThreadPoolForIO::m_busyThreadNum(0);

// debug
// std::atomic<int> cnt(0);
// std::atomic<int> cntin(0);
// pthread_mutex_t iomtx = PTHREAD_MUTEX_INITIALIZER;

ThreadPoolForIO::ThreadPoolForIO(const CConfig* p_ins_config) : m_actThreadNum(0)
{
    m_threadNum = p_ins_config->getIntDefault("num_threads_proc_io",30);
}

ThreadPoolForIO::~ThreadPoolForIO()
{

}

bool ThreadPoolForIO::createThreads()
{
    pthread_t this_tid = 0;
    threadItem* tmpThreadItem;
    for( int i=0; i<m_threadNum; i++ )
    {
        tmpThreadItem = new threadItem(this,false);
        m_threadList.push_back(tmpThreadItem);
        if( pthread_create(&tmpThreadItem->tid, nullptr, threadFunction, tmpThreadItem) != 0 )
        {
            // error 
            return false;
        }
        
        m_actThreadNum += 1;
    }

lblfor:
    for( auto iter = m_threadList.begin(); iter!=m_threadList.end(); ++iter )
    {
        if( (*iter)->isRunning == false )
        {
            // sleep for 100*1000us.
            usleep(100*1000);
            goto lblfor;
        }
    }
    return true;
}


void* ThreadPoolForIO::threadFunction(void* data)
{
    // cntin++;
    // pthread_mutex_lock(&iomtx);
    // 下面这句话输出了20次，20
    // std::cout << "cntin = " << cntin << std::endl;
    // pthread_mutex_unlock(&iomtx);

    IO_t* p_IOObj = IO_t::getInstance();                            // 问题出在这句话。

    // pthread_mutex_lock(&iomtx);
    // 下面这句话只输出了18次，19
    // std::cout << "cntin2 = " << cntin << std::endl;
    // pthread_mutex_unlock(&iomtx);

    threadItem* tmpThreadItem = static_cast<threadItem*>(data);     
    int err = 0;

    while( true )
    {
        // std::cout << "threadFunction running " << std::endl;
        err = pthread_mutex_lock(&(p_IOObj->m_mtx_IOOpQueue));
        if( err != 0 ) 
        {
            std::cerr << "pthread_mutex_lock failed in threadFunction()" << std::endl;
            return nullptr;
        }
        // obverously that g_IOOpQueue is empty when thread pool is initializing.
        while( p_IOObj->m_IOOpQueue.empty() )
        {
            int t = tmpThreadItem->tid;
            // cnt++;
            // 下面这句话总是能够输出和cntin2同样的次数。
            // pthread_mutex_lock(&iomtx);
            // std::cout << "cnt = " << cnt << std::endl;
            // pthread_mutex_unlock(&iomtx);
            tmpThreadItem->isRunning = true;
            pthread_cond_wait(&(p_IOObj->m_IOReqCond), &(p_IOObj->m_mtx_IOOpQueue));
        }
        ThreadPoolForIO::m_busyThreadNum ++;
        IOOp_t op = p_IOObj->m_IOOpQueue.front();
        p_IOObj->m_IOOpQueue.pop();
        err = pthread_mutex_unlock(&(p_IOObj->m_mtx_IOOpQueue));
        if( err != 0 )
        {
            std::cerr << "pthread_mutex_lock failed in threadFunction()" << std::endl;
            return nullptr;
        }

        // process IO
        p_IOObj->procIOOp(op);
        ThreadPoolForIO::m_busyThreadNum --;
    }

    return nullptr;
}

