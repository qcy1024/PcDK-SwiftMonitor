#include "DNSMonitor.h"

DNSMonitor* DNSMonitor::m_instance = nullptr;

std::ostream& operator << (std::ostream& os, const DNSQuery_t& dnsQuery)
{
    os << "DNS Name: " << dnsQuery.queryName << std::endl
       << "DNS Type: " << dnsQuery.queryType << std::endl
       << "DNS Class: " << dnsQuery.queryClass; 
    return os;
}

DNSMonitor::DNSMonitor() : CLowLayersUnlocker()
{

}

DNSMonitor::~DNSMonitor()
{

}

bool DNSMonitor::unlockDNSPacket(pcpp::Packet* dnsPacket,adwfOn_t* ad)
{
#ifdef DEBUG_DNS
    m_dnsPackNum ++;
#endif
    if( !dnsPacket->isPacketOfType(pcpp::DNS) )
    {
        return false;
    }
    unlockEth(dnsPacket, ad);
    unlockIPv4(dnsPacket, ad);
    unlockUdp(dnsPacket, ad);
    pcpp::DnsLayer* packDNSLayer = dnsPacket->getLayerOfType<pcpp::DnsLayer>();
    if( packDNSLayer == nullptr ) 
    {
        return false;
    }
    pcpp::dnshdr* packDNShdr = packDNSLayer->getDnsHeader();
    DNSMonitor* p_ins_DNSMonitor = DNSMonitor::getInstance();
    qcy::Vec<DNSQuery_t> dnsQueries;

    m_dnsTransactionID = std::to_string(packDNShdr->transactionID);

    size_t packQueryCount = packDNSLayer->getQueryCount();
    pcpp::DnsQuery* it = packDNSLayer->getFirstQuery();
    if( it == nullptr ) return false;

    while( packQueryCount-- )
    {
        pcpp::DnsQuery* nxt = packDNSLayer->getNextQuery(it);
        std::string curQueryType = p_ins_DNSMonitor->getStrDnsTypeFrompcpp_DnsType(it->getDnsType());
        std::string curQueryClass = p_ins_DNSMonitor->getStrDnsClassFrompcpp_DnsClass(it->getDnsClass());
        std::string curQueryName = it->getName();
        DNSQuery_t* newQuery = new DNSQuery_t(curQueryName, curQueryType, curQueryClass);
        m_dnsQueries.push_back(newQuery);
        it = nxt;
    }

    return true;
}

bool DNSMonitor::dnsLogic(adwfOn_t* ad)
{
#ifdef DEBUG_DNS
    std::string dnsInfo = std::string(std::to_string(m_dnsPackNum)+"th dns packet.\n");
#else
    std::string dnsInfo = std::string("");
#endif
    dnsInfo = dnsInfo + *getLowLayerInfo(ad) 
    + __SEG__LINE__ + "\n" + DNS_INFO_TITLE + "\n" + __SEG__LINE__ + "\n"
    + *getDnsLayerInfo(ad);
    IO_t* p_ins_IO_t = IO_t::getInstance();
    IOOp_t ioop(STDOUT, dnsInfo);
    p_ins_IO_t->putIOOpInQAndSignal(ioop);

    clear_m_dnsQueries();

#ifdef CLEAR_CHEAT
    cheaty_clearLowLayersInfo();
#else 
    clearLowLayersInfo();
#endif

    return true;
}

std::shared_ptr<std::string> DNSMonitor::getDnsLayerInfo(const adwfOn_t* ad)
{
    std::string ret(std::string(""));
    ret = ret + "Transaction ID: " + m_dnsTransactionID + "\n"
        + "Queries" + "\n";
    for( qcy::Vec<DNSQuery_t*>::iterator it = m_dnsQueries.begin(); it != m_dnsQueries.end(); ++it ) 
    {
        ret = ret + "  " + (*it)->queryName + ": type " + (*it)->queryType + ", class" + (*it)->queryClass + "\n";
    }
    return std::make_shared<std::string>(ret);
}

/* tool functions. */
void DNSMonitor::clear_m_dnsQueries()
{
    for( auto it = m_dnsQueries.begin(); it != m_dnsQueries.end(); ++it ) 
    {
        if( (*it) != nullptr ) 
        {
            delete (*it);
            (*it) = nullptr;
        }
    }
#ifdef CLEAR_CHEAT
    m_dnsQueries.cheatyClear();
#else 
    m_dnsQueries.clear();
#endif
}

std::string DNSMonitor::getStrDnsTypeFrompcpp_DnsType(pcpp::DnsType dnstype)
{
    switch(dnstype)
    {
        case pcpp::DNS_TYPE_A :               return "A"; break;
        case pcpp::DNS_TYPE_AAAA :            return "AAAA"; break;
        case pcpp::DNS_TYPE_NS :              return "NS"; break;
        default: break;
    }
    return "Other";
}

std::string DNSMonitor::getStrDnsClassFrompcpp_DnsClass(pcpp::DnsClass dnsclass)
{
    switch(dnsclass)
    {
        case pcpp::DNS_CLASS_IN :             return "IN"; break;
        case pcpp::DNS_CLASS_IN_QU :          return "IN_QU"; break;
        case pcpp::DNS_CLASS_CH :             return "CH"; break;
        case pcpp::DNS_CLASS_HS :             return "HS"; break;
        case pcpp::DNS_CLASS_ANY :            return "ANY"; break;
        default:                        break;
    }
    return "Other";
}
