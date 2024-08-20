#include <arpa/inet.h>
#include <signal.h>

#include "PcapFilter.h"

#include "CConfig.h"
#include "CapWorker.h"

/*----------------------------- debug -----------------------------*/

bool useFilter = true;
bool display_low_layers_info = false;
bool stillCapture = true;
bool debug_http = false;
bool debug_dns = true;

// #define DEBUF_THREADPOOL

void debugPrintRawPacket(pcpp::RawPacket* rawPacket)
{
    CRawPacketManager* ins = CRawPacketManager::getInstance();
    ins->printRawPacketData(rawPacket);
}

/*----------------------------- debug -----------------------------*/


// Ctrl+C to stop. We will use sigaction in future.
void sigHandler(int signum)
{
#ifdef DEBUG_THREADPOOL
    if( signum == SIGINT )
    {
        exit(0);
    }
#else 
    if(signum == SIGINT)
    {
        stillCapture = false;
    }
#endif
}

CRawPacketManager* g_packManager;

int main(int argc, char* argv[]) 
{
    // This is for easy. We will use sigaction in future.
    signal(SIGINT, sigHandler);
    std::cout << "signal registered." << std::endl;

    // load config.
    CConfig* p_ins_Config = CConfig::getInstance();
    if( !p_ins_Config->load("../config.conf") )
    {
        std::cout << "load config failed, exit!" << std::endl;
        exit(0); 
    }
    useFilter = std::string(p_ins_Config->getString("use_filter")) == "true" ? true : false;
    display_low_layers_info = std::string(p_ins_Config->getString("display_low_layers_info")) == "true" ? true : false;
    debug_http = std::string(p_ins_Config->getString("debug_http")) == "true" ? true : false;
    debug_dns = std::string(p_ins_Config->getString("debug_dns")) == "true" ? true : false;
    std::cout << "debug_dns=" << debug_dns << ", debug_http=" << debug_http << std::endl;
    std::cout << "Config loaded." << std::endl;
    
    // Init thread pool.
    ThreadPoolForIO* p_thPoolIO = new ThreadPoolForIO(p_ins_Config);
    if( !p_thPoolIO->createThreads() )
    {
        std::cout << "Failed to create threads, exit!" << std::endl;
        exit(0);
    }
    std::cout << "thread pool inited" << std::endl;

    // get net interface
    char* c_network_interface_ip = p_ins_Config->getString("network_interface_ip");
    pcpp::PcapLiveDevice* dev = pcpp::PcapLiveDeviceList::getInstance().getPcapLiveDeviceByIpOrName(c_network_interface_ip);
    if( dev == nullptr )
    {
        std::cout << "Open interface failed." << std::endl;
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

    // set packet filter: http, https, mysql, DNS
    if( useFilter )
    {
        std::cout << "Using filter." << std::endl;
        pcpp::OrFilter awfFilter;

        pcpp::ProtoFilter TCPFilter(pcpp::TCP);
        // pcpp::ProtoFilter UDPFilter(pcpp::UDP);

        pcpp::PortFilter httpPortFilter(HTTP_PORT,pcpp::SRC_OR_DST);
        pcpp::PortFilter httpsPortFilter(HTTPS_PORT,pcpp::SRC_OR_DST);
        pcpp::PortFilter DNSPortFilter(DNS_PORT,pcpp::SRC_OR_DST);
        pcpp::PortFilter mysqlPortFilter(MYSQL_PORT,pcpp::SRC_OR_DST);
        
        pcpp::AndFilter httpFilter;
        httpFilter.addFilter(&TCPFilter);
        httpFilter.addFilter(&httpPortFilter);

        pcpp::AndFilter httpsFilter;
        httpsFilter.addFilter(&TCPFilter);
        httpsFilter.addFilter(&httpsPortFilter);

        pcpp::AndFilter mysqlFilter;
        mysqlFilter.addFilter(&TCPFilter);
        mysqlFilter.addFilter(&mysqlPortFilter);

        pcpp::AndFilter DNSFilter;
        DNSFilter.addFilter(&DNSPortFilter);

        awfFilter.addFilter(&httpFilter);
        awfFilter.addFilter(&httpsFilter);
        awfFilter.addFilter(&mysqlFilter);
        awfFilter.addFilter(&DNSFilter);
        
        dev->setFilter(awfFilter);
    }

    // start capturing.
    p_adwfOn_t p_adwfOn = new adwfOn_t;
    CapWorker::init(useFilter);
    if( dev->startCapture(CapWorker::procPacketArrived,p_adwfOn) )
    {
        std::cout << "successfully started capturing." << std::endl;
    }
    else 
    {
        std::cout << "failed to start capture." << std::endl;
        exit(0);
    }


    // logic in main().
    StatisticCollector* statCollector = StatisticCollector::getInstance();
    IO_t* IOProcessor = IO_t::getInstance();

    // Perhaps sleep, giving up it's time slice to packet capturing thread, or maybe periodically calculate real time information , I don't know now.
    while(stillCapture)
    {
        // sleep here, or do something. do nothing is sidiupide.
        std::cout << "       * * * |-- STATISTIC INFO --| * * * " << std::endl;
        
        std::shared_ptr<std::string> strIOMSG = statCollector->getAllStatDatas();
        TYPE_IOMSG tmpIOMSG = *strIOMSG;
        IOOp_t tmpIOOP(STDOUT, tmpIOMSG);
        IOProcessor->putIOOpInQAndSignal(tmpIOOP);
        sleep(5);
    }
    std::cout << "\nShutting down..." << std::endl;

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
