#include "CapWorker.h"

// debug.
extern bool debug_http;
extern bool debug_dns;

extern CRawPacketManager* g_packManager;
bool CapWorker::useFilter = false;

void CapWorker::init(bool useFilter)
{
    CapWorker::useFilter = useFilter;
}

// the callback.
void CapWorker::procPacketArrived(pcpp::RawPacket* packet, pcpp::PcapLiveDevice* dev, void* cookie)
{
    p_adwfOn_t ad = static_cast<adwfOn_t*>(cookie);
    pcpp::Packet parsedPacket(packet);

    pcpp::TcpLayer* tcpLayer = parsedPacket.getLayerOfType<pcpp::TcpLayer>();
    if( tcpLayer != nullptr )
    {
        if( debug_dns )
            return ;    // for debug procUdpPacket.
        g_packManager->procTcpPacket(&parsedPacket, dev, ad);
    }

    pcpp::UdpLayer* udpLayer = parsedPacket.getLayerOfType<pcpp::UdpLayer>();
    if( udpLayer != nullptr )
    {
        if( debug_http )
            return ;
        g_packManager->procUdpPacket(&parsedPacket, dev, ad);
    }
}

// officlial has labeled as a bug.
// type == 0 to generate a HTTPRequest packet and type == 1 to generate a HTTPResponse packet.
pcpp::Packet CapWorker::generateHTTPPacket(int type)
{
    // construct ethLayer
    pcpp::MacAddress srcMac("00:0c:29:e8:17:58");
    pcpp::MacAddress dstMac("00:0c:29:e8:17:58");
    pcpp::EthLayer ethLayer(srcMac, dstMac, PCPP_ETHERTYPE_IP);

    // construct IPv4Layer
    pcpp::IPv4Address srcIp("192.168.1.10");
    pcpp::IPv4Address dstIp("192.168.1.10");
    pcpp::IPv4Layer ipLayer(srcIp, dstIp);
    ipLayer.getIPv4Header()->ipId = pcpp::hostToNet16(2000);
    ipLayer.getIPv4Header()->timeToLive = 64;

    // construct TCP Layer
    uint16_t srcPort = 12345; // random src port 
    uint16_t dstPort = 80;    // HTTP default port 80
    pcpp::TcpLayer tcpLayer(srcPort, dstPort);

    // construct packcet by add layers 
    pcpp::Packet httpPacket(1024);      
    httpPacket.addLayer(&ethLayer);
    httpPacket.addLayer(&ipLayer);
    httpPacket.addLayer(&tcpLayer);

    if( type == 0 )
    {
        // construct HttpRequest Layer
        std::cout << "type==0 is runing" << std::endl;   //This can output normally when parameter "type" is 0
        pcpp::HttpRequestLayer httpRequest(pcpp::HttpRequestLayer::HttpGET, "www.example.com",pcpp::OneDotOne );

        httpPacket.addLayer(&httpRequest);
        httpPacket.computeCalculateFields();
    }
    else if( type == 1 )
    {
        // construct HttpResponse Layer
        pcpp::HttpResponseLayer httpResponse(pcpp::OneDotOne, pcpp::HttpResponseStatusCode::Http200OK );

        httpPacket.addLayer(&httpResponse);
        httpPacket.computeCalculateFields();
    }

    // segmentation falut before here.
    std::cout << "end of generateHttpPacket " << std::endl;
    
    return httpPacket;

}


// debug.
void CapWorker::debugDNSPacketArrive(pcpp::Packet* parsedPacket, pcpp::PcapLiveDevice* dev, void* cookie)
{
    std::cout << std::endl;

    if( parsedPacket->isPacketOfType(pcpp::DNS) )
    {
        std::cout << "* * * DNS packet received. * * *" << std::endl;
    }
    pcpp::DnsLayer* packDNSLayer = parsedPacket->getLayerOfType<pcpp::DnsLayer>();
    pcpp::dnshdr* packDNShdr = packDNSLayer->getDnsHeader();

    std::cout << "DNShdr中有如下字段: " << std::endl;
    std::cout << "transactionID: " << packDNShdr->transactionID << std::endl;
    std::cout << "recursionDesired: " << packDNShdr->recursionDesired << std::endl;
    std::cout << "truncation: " << packDNShdr->truncation << std::endl;
    std::cout << "authoritativeAnswer: " << packDNShdr->authoritativeAnswer << std::endl;
    std::cout << "queryOrResponse: " << packDNShdr->queryOrResponse << std::endl;
    std::cout << "responseCode: " << packDNShdr->responseCode << std::endl;
    std::cout << "checkingDisabled: " << packDNShdr->checkingDisabled << std::endl;
    std::cout << "authenticData: " << packDNShdr->authenticData << std::endl;
    std::cout << "zero: " << packDNShdr->zero << std::endl;
    std::cout << "recursionAvailable: " << packDNShdr->recursionAvailable << std::endl;
    std::cout << "numberOfQuestions: " << packDNShdr->numberOfQuestions << std::endl;
    std::cout << "numberOfAnswers: " << packDNShdr->numberOfAnswers << std::endl;
    std::cout << "numberOfAuthority: " << packDNShdr->numberOfAuthority << std::endl;
    std::cout << "numberOfAdditional: " << packDNShdr->numberOfAdditional << std::endl;

    std::cout << std::endl;

    std::cout << "接下来解析Querys字段中的所有Query。" << std::endl;

    DNSMonitor* p_ins_DNSMonitor = DNSMonitor::getInstance();

    qcy::Vec<DNSQuery_t> dnsQueries;

    size_t packQueryCount = packDNSLayer->getQueryCount();
    pcpp::DnsQuery* it = packDNSLayer->getFirstQuery();
    if( it == nullptr ) return ;
    // std::string curQueryType = p_ins_DNSMonitor->getStrDnsTypeFrompcpp_DnsType(it->getDnsType());
    // std::string curQueryClass = p_ins_DNSMonitor->getStrDnsClassFrompcpp_DnsClass(it->getDnsClass());
    // std::string curQueryName = it->getName();
    // dnsQueries.push_back(DNSQuery_t(curQueryName, curQueryType, curQueryClass));
    while( packQueryCount-- )
    {
        pcpp::DnsQuery* nxt = packDNSLayer->getNextQuery(it);
        std::string curQueryType = p_ins_DNSMonitor->getStrDnsTypeFrompcpp_DnsType(it->getDnsType());
        std::string curQueryClass = p_ins_DNSMonitor->getStrDnsClassFrompcpp_DnsClass(it->getDnsClass());
        std::string curQueryName = it->getName();
        dnsQueries.push_back(DNSQuery_t(curQueryName, curQueryType, curQueryClass));
        it = nxt;
    }

    std::cout << "Querys中所有字段如下: " << std::endl;

    for( int i=0; i<dnsQueries.size(); i++ ) 
    {
        std::cout << "Query " << i+1 << ": ";
        std::cout << dnsQueries[i] << std::endl;
    }

    dnsQueries.clear();

    std::cout << std::endl;

}

