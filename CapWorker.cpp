#include "CapWorker.h"

bool CapWorker::useFilter = false;

void CapWorker::init(bool useFilter)
{
    CapWorker::useFilter = useFilter;
}

// the callback.
void CapWorker::procPacketArrived(pcpp::RawPacket* packet, pcpp::PcapLiveDevice* dev, void* cookie)
{
    pcpp::Packet parsedPacket(packet);
    // verify packet is TCP
    if ( !parsedPacket.isPacketOfType(pcpp::TCP) )
        return;
    pcpp::TcpLayer* tcpLayer = parsedPacket.getLayerOfType<pcpp::TcpLayer>();
    if( CapWorker::useFilter && !checkTcpHttp(tcpLayer) && !checkTcpHttps(tcpLayer) && !checkTcpMysql(tcpLayer) )
    {
        // I/O may be executed in a new thread.
        std::cout << "Received an unexpected packet after filtering." << std::endl;
        return ;
    }

    // Debug
    // std::cout << "srcPort: " << tcpLayer->getSrcPort() << ",  dstPort: " << tcpLayer->getDstPort() << std::endl;
    // debugPrintRawPacket(packet);

    p_adwfOn_t ad = static_cast<adwfOn_t*>(cookie);
    HttpMonitor* p_ins_HttpMonitor = HttpMonitor::getInstance();
    StatisticCollector* statCollector = StatisticCollector::getInstance();
    
    if( parsedPacket.isPacketOfType(pcpp::HTTPRequest) )
    {
        // debug.
        std::cout << __SEG__LINE__ << std::endl;
        std::cout << "|  * * * * Http request packet received. * * * *  |" << std::endl;
        std::cout << __SEG__LINE__ << std::endl;

        if( !p_ins_HttpMonitor->unlockHTTPRequestPacket(&parsedPacket,ad) )
        {
            std::cout << "Analyze http request packet failed, something went wrong." << std::endl;  
        }
        p_ins_HttpMonitor->httpRequestLogic(ad);
        statCollector->collectHttpRequest(ad, parsedPacket);
    }

    else if( parsedPacket.isPacketOfType(pcpp::HTTPResponse) )
    {
        // debug.
        std::cout << __SEG__LINE__ << std::endl;
        std::cout << "|  * * * * Http response packet received. * * * * |" << std::endl;
        std::cout << __SEG__LINE__ << std::endl;

        if( !p_ins_HttpMonitor->unlockHTTPResponsePacket(&parsedPacket, ad) )
        {
            std::cout << "Analyze http response packet failed, something went wrong." << std::endl;  
        }
        p_ins_HttpMonitor->httpResponseLogic(ad);
        statCollector->collectHttpResponse(ad, parsedPacket);
    }

    else 
    {
        // std::cout << "\n not request and not response" << std::endl << std::endl;
    }

    ad->clearAll();

#ifndef CLEAR_CHEAT
    p_ins_HttpMonitor->clearHttpHeaderFields();
#else 
    p_ins_HttpMonitor->cheaty_clearHttpHeaderFields();
#endif 
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

