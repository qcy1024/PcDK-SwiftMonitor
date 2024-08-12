#ifndef __RAW_PACKET_MANAGER__H__
#define __RAW_PACKET_MANAGER__H__

#include "ProcIO.h"
#include <pthread.h>
#include "RawPacket.h"
#include <string>

class CRawPacketManager
{
private:
    CRawPacketManager(){};
    static CRawPacketManager* m_instance;
    static pthread_mutex_t m_mtx_instance;

    class Garbo
    {
    public:
        Garbo(){};
        ~Garbo()
        {
            if( CRawPacketManager::m_instance != nullptr )
            {
                delete m_instance;
                m_instance = nullptr;
            }
        };

    };
    Garbo garbo;

public:
    static CRawPacketManager* getInstance()
    {
        if( m_instance == nullptr )
        {
            pthread_mutex_lock(&CRawPacketManager::m_mtx_instance);
            if( m_instance == nullptr )
            {
                m_instance = new CRawPacketManager;
            }
            pthread_mutex_unlock(&CRawPacketManager::m_mtx_instance);
        }
        return m_instance;
    }

    // interfaces
    bool printRawPacketData(const pcpp::RawPacket* rawPacket) const;

};




#endif

