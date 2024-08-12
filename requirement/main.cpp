#include <arpa/inet.h>
#include <signal.h>

#include "PcapFilter.h"

#include "CConfig.h"
#include "CapWorker.h"
#include "StatisticsCollector.h"

#define HTTP_PORT       80
#define HTTPS_PORT      443
#define MYSQL_PORT      3306

// for debug.
bool useFilter = true;
bool stillCapture = true;

void debugPrintRawPacket(pcpp::RawPacket* rawPacket)
{
    CRawPacketManager* ins = CRawPacketManager::getInstance();
    ins->printRawPacketData(rawPacket);
}

// Ctrl+C to stop.
void sigHandler(int signum)
{
    if(signum == SIGINT)
    {
        stillCapture = false;
    }
}

// 程序有时候运行之后一直阻塞，问题出在线程池初始化的时候。
int main(int argc, char* argv[]) 
{
    std::cout << "start main." << std::endl;
    // For easy. We will use sigaction in future.
    signal(SIGINT, sigHandler);
    std::cout << "signal registered." << std::endl;
    CConfig* p_ins_Config = CConfig::getInstance();

    if( !p_ins_Config->load("../config.conf") )
    {
        std::cout << "load config failed, exit!" << std::endl;
        exit(0); 
    }

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

    // set packet filter: http, https, mysql 
    if( useFilter )
    {
        std::cout << "Using filter." << std::endl;
        pcpp::OrFilter httpAndMysqlFilter;

        pcpp::ProtoFilter TCPFilter(pcpp::TCP);

        pcpp::PortFilter httpPortFilter(HTTP_PORT,pcpp::SRC_OR_DST);
        pcpp::PortFilter httpsPortFilter(HTTPS_PORT,pcpp::SRC_OR_DST);
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

        httpAndMysqlFilter.addFilter(&httpFilter);
        httpAndMysqlFilter.addFilter(&httpsFilter);
        httpAndMysqlFilter.addFilter(&mysqlFilter);
        dev->setFilter(httpAndMysqlFilter);
    }

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

    StatisticCollector* statCollector = StatisticCollector::getInstance();
    IO_t* IOProcessor = IO_t::getInstance();

    // Perhaps sleep, giving up it's time slice to packet capturing thread, or maybe periodically calculate real time information , I don't know now.
    while(stillCapture)
    {
        // sleep, or do something. do nothing is sidiupide.
        std::cout << " * * * |-- STATISTIC INFO --| * * * " << std::endl;
        
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
    sleep(1);
    return 0;
}
