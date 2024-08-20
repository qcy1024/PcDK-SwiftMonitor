#include "DNSStatisticsCollector.h"

DNSStatisticCollector* DNSStatisticCollector::m_instance = nullptr;
pthread_mutex_t DNSStatisticCollector::m_mtx_ins = PTHREAD_MUTEX_INITIALIZER;

DNSStatisticCollector::DNSStatisticCollector() : StatisticCollector()
{

}

DNSStatisticCollector::~DNSStatisticCollector()
{
    
}

bool DNSStatisticCollector::collectDns(adwfOn_t* ad,pcpp::Packet udpPacket)
{

    return true;
}
