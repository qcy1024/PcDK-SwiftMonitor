#include "ProcIO.h"
#include <iostream>

pthread_mutex_t IO_t::m_mtx_ins;
IO_t* IO_t::m_instance = nullptr;

IO_t::IO_t() : m_ioPutcnt(0), m_processedIOcnt(0)
{ 
    pthread_mutex_init(&IO_t::m_mtx_ins, nullptr);  
    pthread_mutex_init(&m_mtx_IOOpQueue, nullptr);
}

// process a IO operation.
bool IO_t::procIOOp(IOOp_t op)
{
    // std::cout << "procIOOp running" << std::endl;
    switch( op.opType )
    {
        case STDOUT : 
        {
            std::cout << op.msg << std::endl;
            break;
        }

        default: break;
    }
    m_processedIOcnt++;
    return true;
}

// place a IO operation into m_IOOpQueue.
bool IO_t::putIOOpInQAndSignal(IOOp_t op)
{
    pthread_mutex_lock(&m_mtx_IOOpQueue);
    m_IOOpQueue.push(op);
    pthread_mutex_unlock(&m_mtx_IOOpQueue);
    m_ioPutcnt++;
    pthread_cond_signal(&m_IOReqCond);
    // std::cout << "put in IOq scceed." << std::endl;
    return true;
}

