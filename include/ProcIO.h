#ifndef __PROCIO__H__
#define __PROCIO__H__

#include <iostream>
#include <atomic>
#include <queue>
#include "GUnlockPack.h"

#define DEBUG_IOOP_STAT

#define TYPE_IOMSG std::string

// debug
// extern std::atomic<int> cnt;
// extern std::atomic<int> cntin;
// extern pthread_mutex_t iomtx;

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
        // std::cout << "*********入**********" << std::endl;
        if( m_instance == nullptr )
        {
            // pthread_mutex_lock(&iomtx);
            // std::cout << "此时m_instance == nullptr" << std::endl;
            // pthread_mutex_unlock(&iomtx);
            pthread_mutex_lock(&IO_t::m_mtx_ins);
            // pthread_mutex_lock(&iomtx);
            // std::cout << "一个线程拿到了锁。"  << std::endl;
            // pthread_mutex_unlock(&iomtx);
            if( m_instance == nullptr )
            {
                m_instance = new IO_t;
            }
            pthread_mutex_unlock(&IO_t::m_mtx_ins);
            // pthread_mutex_lock(&iomtx);
            // std::cout << "一个线程放开了锁。"  << std::endl;
            // pthread_mutex_unlock(&iomtx);
        }
        // pthread_mutex_lock(&iomtx);
        // std::cout << "一个线程成功拿到了m_instance 返回了。" << std::endl;
        // pthread_mutex_unlock(&iomtx);
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
    static IO_t* m_instance;
    // mutex for multi-thread access of m_instance
    static pthread_mutex_t m_mtx_ins;
    IO_t();

// for debug 
public:
    // the number of IO ops that placed in m_IOOpQueue by putIOOpInQAndSignal() 
    std::atomic<int> m_ioPutcnt;
    // the number of IO ops that processed by procIOOp()
    std::atomic<int> m_processedIOcnt;
} IO_t, *p_IO_t;


#endif 
