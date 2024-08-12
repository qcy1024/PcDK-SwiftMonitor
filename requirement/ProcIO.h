#ifndef __PROCIO__H__
#define __PROCIO__H__

#include <iostream>
#include <atomic>
#include <queue>
#include "GUnlockPack.h"

#define DEBUG_IOOP_STAT

#define TYPE_IOMSG std::string

class ThreadPoolForIO;

enum TYPE_IOOP 
{
    STDOUT, STDIN,
    FILE_W, FILE_R
};

typedef struct IOOp_t 
{

    TYPE_IOMSG msg;
    TYPE_IOOP opType;

    IOOp_t() : msg(""), opType(STDOUT) {};
    IOOp_t(TYPE_IOOP opType, TYPE_IOMSG msg) : msg(msg), opType(opType) {};

}IOOp_t, *p_IOOp_t;

// The instance of this class must be used wherever IO operations are needed in packet capturing thread.
typedef class IO_t
{
    friend class ThreadPoolForIO;

private:
    static IO_t* m_instance;
    // mutex for multi-thread access of m_instance
    static pthread_mutex_t m_mtx_ins;
    IO_t();

public:

    class Garbo 
    {
    public:
        ~Garbo() 
        {
            if( IO_t::m_instance != nullptr )
            {
                delete IO_t::m_instance;
                IO_t::m_instance = nullptr;
            }
        }
    };

    static IO_t* getInstance() 
    {
        if( m_instance == nullptr )
        {
            pthread_mutex_lock(&IO_t::m_mtx_ins);
            if( m_instance == nullptr )
            {
                m_instance = new IO_t;
            }
            pthread_mutex_unlock(&IO_t::m_mtx_ins);
        }
        return m_instance;
    }

#ifdef DEBUG_IOOP_STAT

    // process a IO operation.
    bool procIOOp(IOOp_t op);
    // place a IO operation into m_IOOpQueue.
    bool putIOOpInQAndSignal(IOOp_t op);
#else 
    bool procIOOp(IOOp_t op) const;
    bool putIOOpInQAndSignal(IOOp_t op) const;
#endif

private:
    Garbo garbo;
    std::queue<IOOp_t> m_IOOpQueue;
    pthread_mutex_t m_mtx_IOOpQueue;
    pthread_cond_t m_IOReqCond;

// for debug 
public:
    // the number of IO ops that placed in m_IOOpQueue by putIOOpInQAndSignal() 
    std::atomic<int> m_ioPutcnt;
    // the number of IO ops that processed by procIOOp()
    std::atomic<int> m_processedIOcnt;
} IO_t, *p_IO_t;


#endif 
