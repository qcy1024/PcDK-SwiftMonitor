#ifndef __DNS_STATISTICS_COLLECTOR_H__
#define __DNS_STATISTICS_COLLECTOR_H__

#include "StatisticsCollector.h"

class DNSStatisticCollector : public StatisticCollector
{
public:
    static DNSStatisticCollector* getInstance()
    {
        if( m_instance == nullptr )
        {
            pthread_mutex_lock(&DNSStatisticCollector::m_mtx_ins);
            if( m_instance == nullptr )
            {
                m_instance = new DNSStatisticCollector;
            }
            pthread_mutex_unlock(&DNSStatisticCollector::m_mtx_ins);
        }
        return m_instance;
    }

    bool collectDns(adwfOn_t* ad,pcpp::Packet udpPacket);

private:
    DNSStatisticCollector();
    virtual ~DNSStatisticCollector();
    struct Garbo
    {
        ~Garbo() 
        {
            if( DNSStatisticCollector::m_instance != nullptr )
            {
                delete DNSStatisticCollector::m_instance;
                DNSStatisticCollector::m_instance == nullptr;
            }
        }
    };
    Garbo garbo;
    static DNSStatisticCollector* m_instance;
    static pthread_mutex_t m_mtx_ins;
};

#endif
