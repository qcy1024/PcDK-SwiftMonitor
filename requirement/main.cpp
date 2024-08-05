#include <arpa/inet.h>
#include <iostream>
#include <signal.h>
#include <pthread.h>

#include "PcapLiveDeviceList.h"
#include "PcapFilter.h"
#include "PcapFileDevice.h"
#include "HttpMonitor.h"
#include "ThreadPoolForIO.h"
#include "CConfig.h"

bool useFilter = false;
bool stillCapture = true;
const unsigned int MAXLEN_DATAPOOL = 100;
bool checkTcpHttp(pcpp::TcpLayer*);
bool checkTcpHttps(pcpp::TcpLayer*);
bool checkTcpMysql(pcpp::TcpLayer*);


void sigHandler(int signum)
{
    if(signum == SIGINT)
    {
        stillCapture = false;
    }
}

void procPacketArrived(pcpp::RawPacket* packet, pcpp::PcapLiveDevice* dev, void* cookie)
{
    pcpp::Packet parsedPacket(packet);
    // verify packet is TCP
    if ( !parsedPacket.isPacketOfType(pcpp::TCP) )
        return;
    pcpp::TcpLayer* tcpLayer = parsedPacket.getLayerOfType<pcpp::TcpLayer>();
    if( useFilter && !checkTcpHttp(tcpLayer) && !checkTcpHttps(tcpLayer) && !checkTcpMysql(tcpLayer) )
    {
        // I/O may be executed in a new thread.
        std::cout << "Received an unexpected packet after filtering." << std::endl;
        return ;
    }

    // Debug
    // std::cout << "srcPort: " << tcpLayer->getSrcPort() << ",  dstPort: " << tcpLayer->getDstPort() << std::endl;

    p_adwfOn_t ad = static_cast<adwfOn_t*>(cookie);
    
    HttpMonitor* p_ins_HttpMonitor = HttpMonitor::getInstance();
    if( parsedPacket.isPacketOfType(pcpp::HTTPRequest) )
    {
        // we make sure that cookie is thread secure, maybe.
        if( !p_ins_HttpMonitor->unlockHTTPRequestPacket(&parsedPacket,ad) )
        {
            std::cout << "Analyze http request packet failed, something went wrong." << std::endl;  
        }
        // debug 
        // p_ins_HttpMonitor->printAD(ad);
        // p_ins_HttpMonitor->httpRequestLogic(ad);
    }

    else if( parsedPacket.isPacketOfType(pcpp::HTTPResponse) )
    {
        
    }

    
    
}

// officlial has labeled as a bug.
// type == 0 to generate a HTTPRequest packet and type == 1 to generate a HTTPResponse packet.
pcpp::Packet generateHTTPPacket(int type)
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


int main(int argc, char* argv[]) 
{
    
    signal(SIGINT, sigHandler);
    CConfig* p_ins_Config = CConfig::getInstance();

    if( !p_ins_Config->load("../config.conf") )
    {
        std::cout << "load config failed, exit!" << std::endl;
        exit(0); 
    }
    
    ThreadPoolForIO* p_thPoolIO = new ThreadPoolForIO(p_ins_Config);
    p_thPoolIO->createThreads();

    // get net interface
    pcpp::PcapLiveDevice* dev = pcpp::PcapLiveDeviceList::getInstance().getPcapLiveDeviceByIpOrName("192.168.1.10");
    if( dev == nullptr )
    {
        std::cout << "couldn't open interface." << std::endl;
        exit(0);
    }
    std::cout << "interface available: ";

    std::cout << dev->getName() << std::endl;
    // open interface 
    if ( !dev->open() )
    {
        printf("Could not open the device\n");
        exit(0);
    }
    else 
        printf("interface have been successfully opened.\n");

    // set packet filter: http, https, mysql 
    if( useFilter )
    {
        std::cout << "Using filter." << std::endl;
        pcpp::OrFilter httpAndMysqlFilter;
        pcpp::PortFilter httpFilter(80,pcpp::SRC_OR_DST);
        pcpp::PortFilter httpsFilter(443,pcpp::SRC_OR_DST);
        pcpp::PortFilter mysqlFilter(3306,pcpp::SRC_OR_DST);
        httpAndMysqlFilter.addFilter(&httpFilter);
        httpAndMysqlFilter.addFilter(&httpsFilter);
        httpAndMysqlFilter.addFilter(&mysqlFilter);
        dev->setFilter(httpAndMysqlFilter);
    }

    p_adwfOn_t p_adwfOn = new adwfOn_t;
    if( dev->startCapture(procPacketArrived,p_adwfOn) )
    {
        std::cout << "successfully started capturing." << std::endl;
    }
    else 
    {
        std::cout << "failed to start capture." << std::endl;
        exit(0);
    }

    // Perhaps sleep, giving up it's time slice to packet capturing thread, or maybe periodically calculate real time information , I don't know now.
    while(stillCapture)
    {
        // sleep, or do something. do nothing is sidiupide.
    }

    IO_t* io_ins = IO_t::getInstance();
    // The number of IO operations placed in IOOpQueue.
    std::cout << "\nIO put count : " << io_ins->m_ioPutcnt;
    // The number of IO operations taken from IOOpQueue and processed by thPoolIO.
    std::cout << "\nIO processed count : " << io_ins->m_processedIOcnt << std::endl;

    dev->stopCapture();
    dev->close();

lbl_main_end:
    std::cout << std::endl;
    return 0;
}
