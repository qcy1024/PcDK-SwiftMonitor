#include "ThreadPoolForIO.h"
#include <unistd.h>

std::atomic<int> ThreadPoolForIO::m_busyThreadNum(0);

ThreadPoolForIO::ThreadPoolForIO(CConfig* p_ins_config) : m_actThreadNum(0)
{
    m_threadNum = p_ins_config->getIntDefault("num_threads_proc_io",50);
    // pthread_mutex_init(&cntMutex, nullptr);
}

ThreadPoolForIO::~ThreadPoolForIO()
{

}

bool ThreadPoolForIO::createThreads()
{
    pthread_t this_tid = 0;
    for( int i=0; i<m_threadNum; i++ )
    {
        threadItem tmpThreadItem(this_tid,false);
        if( pthread_create(&this_tid, nullptr, threadFunction, nullptr) != 0 )
        {
            // error 
            return false;
        }
        tmpThreadItem.isRunning = true;
        m_threadList.push_back(tmpThreadItem);
        m_actThreadNum += 1;
    }

lblfor:
    for( auto iter = m_threadList.begin(); iter!=m_threadList.end(); ++iter )
    {
        if( iter->isRunning == false )
        {
            usleep(100*1000);
            goto lblfor;
        }
    }
    return true;
}

void* ThreadPoolForIO::threadFunction(void* data)
{
    IO_t* p_IOObj = IO_t::getInstance();
    while( true )
    {
        // std::cout << "threadFunction running " << std::endl;
        pthread_mutex_lock(&(p_IOObj->m_mtx_IOOpQueue));
        // obverously that g_IOOpQueue is empty when thread pool is initializing.
        while( p_IOObj->m_IOOpQueue.empty() )
        {
            pthread_cond_wait(&(p_IOObj->m_IOReqCond), &(p_IOObj->m_mtx_IOOpQueue));
        }
        ThreadPoolForIO::m_busyThreadNum ++;
        IOOp_t op = p_IOObj->m_IOOpQueue.front();
        p_IOObj->m_IOOpQueue.pop();
        pthread_mutex_unlock(&(p_IOObj->m_mtx_IOOpQueue));

        // process IO
        p_IOObj->procIOOp(op);
        ThreadPoolForIO::m_busyThreadNum --;
    }

    return nullptr;
}

