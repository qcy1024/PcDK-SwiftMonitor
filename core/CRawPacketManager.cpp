#include "CRawPacketManager.h"

CRawPacketManager* CRawPacketManager::m_instance = nullptr;
pthread_mutex_t CRawPacketManager::m_mtx_instance = PTHREAD_MUTEX_INITIALIZER;

bool CRawPacketManager::printRawPacketData(const pcpp::RawPacket* rawPacket) const
{
    const unsigned char* packBeg = rawPacket->getRawData();
    int packRawDataLen = rawPacket->getRawDataLen();
    std::string rawData(std::string(""));
    for( int i=1; i<=packRawDataLen; i++ )
    {
        rawData += *(packBeg+i);
    }
    IO_t* IO_instance = IO_t::getInstance();
    IOOp_t tmpIOOp(STDOUT, rawData);
    IO_instance->putIOOpInQAndSignal(tmpIOOp);
    return true;
}

bool CRawPacketManager::procTcpPacket(pcpp::Packet* tcpPacket, pcpp::PcapLiveDevice* dev, adwfOn_t* ad) const
{
    // verify packet is TCP.
    if ( !tcpPacket->isPacketOfType(pcpp::TCP) )
        return false;
    IO_t* p_ins_IO_t = IO_t::getInstance();
    pcpp::TcpLayer* tcpLayer = tcpPacket->getLayerOfType<pcpp::TcpLayer>();
    // useFilter put on in CapWorker is just for debug, it ought to be in cmd line parameters or config file.
    if( CapWorker::useFilter && !checkTcpHttp(tcpLayer) && !checkTcpHttps(tcpLayer) 
                             && !checkTcpMysql(tcpLayer) && !checkTcpDNS(tcpLayer) )
    {
        // I/O will executed in a new thread.
        std::cout << "Received an unexpected TCP packet after filtering." << std::endl;
        return false;
    }

    HttpMonitor* p_ins_HttpMonitor = HttpMonitor::getInstance();
    StatisticCollector* statCollector = StatisticCollector::getInstance();
    
    /* Unlock pack and do logic. */
    if( tcpPacket->isPacketOfType(pcpp::HTTPRequest) )
    {
        // debug.
        std::string hreqprcvd = std::string("") + __SEG__LINE__ + "\n" + HTTP_REQUEST_PACKET_RECEIVED + __SEG__LINE__ + "\n";
        std::cout << hreqprcvd << std::endl;

        if( !p_ins_HttpMonitor->unlockHTTPRequestPacket(tcpPacket,ad) )
        {
            std::cout << "Analyze http request packet failed, something went wrong." << std::endl;  
        }
        p_ins_HttpMonitor->httpRequestLogic(ad);
        statCollector->collectHttpRequest(ad, *tcpPacket);
    }

    else if( tcpPacket->isPacketOfType(pcpp::HTTPResponse) )
    {
        // debug.
        std::string hresprcvd = std::string("") + __SEG__LINE__ + "\n" + HTTP_RESPONSE_PACKET_RECEIVED "\n" + __SEG__LINE__ + "\n";
        std::cout << hresprcvd << std::endl;

        if( !p_ins_HttpMonitor->unlockHTTPResponsePacket(tcpPacket, ad) )
        {
            std::cout << "Analyze http response packet failed, something went wrong." << std::endl;  
        }
        p_ins_HttpMonitor->httpResponseLogic(ad);
        statCollector->collectHttpResponse(ad, *tcpPacket);
    }

    else 
    {
        // std::cout << "\n not Http and not DNS" << std::endl << std::endl;
    }

#ifndef CLEAR_CHEAT
    p_ins_HttpMonitor->clearHttpHeaderFields();
    p_ins_HttpMonitor->clearLowLayersInfo();
#else 
    p_ins_HttpMonitor->cheaty_clearHttpHeaderFields();
    p_ins_HttpMonitor->cheaty_clearLowLayersInfo();
#endif 

#ifdef CLEAR_ALL_AD_DATA
    ad->clearAll();
#endif
    return true;
}

bool CRawPacketManager::procUdpPacket(pcpp::Packet* udpPacket, pcpp::PcapLiveDevice* dev, adwfOn_t* ad) const
{
    // verify packet is UDP.
    if( !udpPacket->isPacketOfType(pcpp::UDP) )
        return false;
    pcpp::UdpLayer* udpLayer = udpPacket->getLayerOfType<pcpp::UdpLayer>();
    IO_t* p_ins_IO_t = IO_t::getInstance();
    if( CapWorker::useFilter && !checkUdpDNS(udpLayer) )
    {
        std::cout << "Received an unexpected UDP packet after filtering." << std::endl;
        return false;
    }

    DNSMonitor* p_ins_DNSMonitor = DNSMonitor::getInstance();
    DNSStatisticCollector* p_ins_DNSStatisticCollector = DNSStatisticCollector::getInstance();

    if( udpPacket->isPacketOfType(pcpp::DNS) )
    {
        // debug.
        std::string dnsprcvd = std::string("") + __SEG__LINE__ + "\n" + DNS_PACKET_RECEIVED + "\n" + __SEG__LINE__ + "\n";
        std::cout << dnsprcvd << std::endl;

        if( !p_ins_DNSMonitor->unlockDNSPacket(udpPacket,ad) )
        {
            std::cout << "Analyze DNS packet failed, something went wrong." << std::endl;  
        }
        p_ins_DNSMonitor->dnsLogic(ad);
        p_ins_DNSStatisticCollector->collectDns(ad, *udpPacket);
    }
    return true;
}


bool CRawPacketManager::checkTcpHttp(pcpp::TcpLayer* tcpLayer) const 
{
    return ( tcpLayer->getSrcPort() == HTTP_PORT || tcpLayer->getDstPort() == HTTP_PORT );
}

bool CRawPacketManager::checkTcpHttps(pcpp::TcpLayer* tcpLayer) const 
{
    return ( tcpLayer->getSrcPort() == HTTPS_PORT || tcpLayer->getDstPort() == HTTPS_PORT );
}

bool CRawPacketManager::checkTcpMysql(pcpp::TcpLayer* tcpLayer) const 
{
    return ( tcpLayer->getSrcPort() == MYSQL_PORT || tcpLayer->getDstPort() == MYSQL_PORT );
}

bool CRawPacketManager::checkTcpDNS(pcpp::TcpLayer* tcpLayer) const 
{
    return ( tcpLayer->getSrcPort() == DNS_PORT || tcpLayer->getDstPort() == DNS_PORT );
}

bool CRawPacketManager::checkUdpDNS(pcpp::UdpLayer* udpLayer) const 
{
    return ( udpLayer->getSrcPort() == DNS_PORT || udpLayer->getDstPort() == DNS_PORT );
}