#ifndef __DNSMONITOR__H__
#define __DNSMONITOR__H__

#include <string>
#include <iostream>
#include <atomic>

#include "UdpLayer.h"
#include "DnsLayer.h"

#include  "ThreadPoolForIO.h"

#define DEBUG_DNS

#define DNS_INFO_TITLE         "  |-- DNS INFO --|  "

typedef struct DNSQuery_t
{
    std::string queryName;
    std::string queryType;
    std::string queryClass;
    DNSQuery_t(std::string queryName, std::string queryType, std::string queryClass) : queryName(queryName), queryType(queryType), queryClass(queryClass) {};
}DNSQuery, *p_DNSQuery_t;

std::ostream& operator << (std::ostream& os, const DNSQuery_t& dnsQuery);

class DNSMonitor : public CLowLayersUnlocker
{
private:
    static DNSMonitor* m_instance;
    DNSMonitor();
    virtual ~DNSMonitor();

    class Garbo 
    {
    public:
        ~Garbo()
        {
            if( DNSMonitor::m_instance != nullptr )
            {
                delete m_instance;
                m_instance = nullptr;
            }
        }
    };
    static Garbo garbo;
    
public:
    // inherited from CLowLayersUnlocker
    // bool unlockLowLayers(pcpp::Packet* parsedPacket, p_adwfOn_t ad);
    // std::shared_ptr<std::string> getLowLayerInfo(p_adwfOn_t ad);

    // We make sure that getInstance() is called only by packet capturing thread.
    static DNSMonitor* getInstance() 
    {
        if( m_instance == nullptr )
        {
            m_instance = new DNSMonitor;
        }
        return m_instance;
    }

    bool unlockDNSPacket(pcpp::Packet* dnsPacket,adwfOn_t* ad);
    bool dnsLogic(adwfOn_t* ad);
    std::shared_ptr<std::string> getDnsLayerInfo(const adwfOn_t* ad);


// Ought to be private. public for debug.
public:
    std::string                             m_dnsTransactionID;
    qcy::Vec<DNSQuery_t*>                   m_dnsQueries;
    std::atomic<int>                        m_dnsPackNum;

    void clear_m_dnsQueries();

    std::string getStrDnsTypeFrompcpp_DnsType(pcpp::DnsType dnstype);
    std::string getStrDnsClassFrompcpp_DnsClass(pcpp::DnsClass dnsclass);
};


#endif
