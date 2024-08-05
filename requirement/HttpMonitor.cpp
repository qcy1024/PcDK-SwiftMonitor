#include "HttpMonitor.h"
#include <iostream>

bool checkTcpHttp(pcpp::TcpLayer*);
bool checkTcpHttps(pcpp::TcpLayer*);
bool checkTcpMysql(pcpp::TcpLayer*);

HttpMonitor* HttpMonitor::m_instance = nullptr;


#define __SEG__LINE__ "---------------------------------------------------"

// interfaces
HttpMonitor::HttpMonitor()
{

}

HttpMonitor::~HttpMonitor()
{

}

bool HttpMonitor::unlockHTTPRequestPacket(pcpp::Packet* httpPacket, p_adwfOn_t ad)
{
    // std::cout << "start of unlock Http request." << std::endl;
    pcpp::EthLayer* packEthernetLayer ;
    pcpp::IPv4Layer* packIPv4Layer;
    pcpp::TcpLayer* packTCPLayer;
    pcpp::HttpRequestLayer* packHttpRequestLayer;
    if( !(packEthernetLayer = httpPacket->getLayerOfType<pcpp::EthLayer>()) || !(packIPv4Layer = httpPacket->getLayerOfType<pcpp::IPv4Layer>())
        || !(packTCPLayer = httpPacket->getLayerOfType<pcpp::TcpLayer>()) || !(packHttpRequestLayer = httpPacket->getLayerOfType<pcpp::HttpRequestLayer>()) ) return false;


    // Debug
    if( checkTcpHttp(packTCPLayer) )
        std::cout << "http request packet received." << std::endl;
    else if( checkTcpHttps(packTCPLayer) )
        std::cout << "https request packet received." << std::endl;

    ad->srcMacAddress = packEthernetLayer->getSourceMac().toString();
    ad->dstMacAddress = packEthernetLayer->getDestMac().toString();
    ad->etherType = pcpp::netToHost16(packEthernetLayer->getEthHeader()->etherType);

    ad->srcIPAddress = packIPv4Layer->getSrcIPAddress().toString();
    ad->dstIPAddress = packIPv4Layer->getDstIPAddress().toString();
    ad->ipid = pcpp::netToHost16(packIPv4Layer->getIPv4Header()->ipId);
    ad->TTL = (uint8_t)packIPv4Layer->getIPv4Header()->timeToLive;

    ad->srcTcpPort = packTCPLayer->getSrcPort();
    ad->dstTcpPort = packTCPLayer->getDstPort();
    ad->wdSize = pcpp::netToHost16(packTCPLayer->getTcpHeader()->windowSize);
    ad->tcpFlags = getTcpFlagsByLayer(packTCPLayer);

    ad->httpMethod = getHttpMethodByRequestLayer(packHttpRequestLayer);
    ad->httpRequestUri = packHttpRequestLayer->getFirstLine()->getUri();
    ad->httpHost = packHttpRequestLayer->getFieldByName(PCPP_HTTP_HOST_FIELD)->getFieldValue();
    ad->httpUserAgent = packHttpRequestLayer->getFieldByName(PCPP_HTTP_USER_AGENT_FIELD)->getFieldValue();

    
    // try to output a whole parsed http packet.
    size_t packHeaderLen = packHttpRequestLayer->getHeaderLen();
    std::cout << "Header: " << std::endl;
    unsigned char* hdBegin = packHttpRequestLayer->getData();
    for( int i=0; i<packHeaderLen; i++ )
    {
        std::cout << *(hdBegin+i) ;
    }
    std::cout << std::endl;

    std::cout << "Body: " << std::endl;
    unsigned char* bdBegin = hdBegin + packHeaderLen;
    size_t packBodyLen = packHttpRequestLayer->getDataLen() - packHeaderLen;
    for( int i=0; i<packHeaderLen; i++ )
    {
        std::cout << *(bdBegin+i) ;
    }
    std::cout << std::endl;

    // a segmentation falut here.
    // ad->httpCookie = packHttpRequestLayer->getFieldByName(PCPP_HTTP_COOKIE_FIELD)->getFieldValue();
    // std::cout << "end of unlockHttpRequest." << std::endl;
    return true;
}

// print the http request information to stdout. Maybe other logics we need.
void HttpMonitor::httpRequestLogic(adwfOn_t* ad)
{
    // std::cout << "start of httpLogic" << std::endl;
    std::string httpInfo = std::string("") 
        + "\n"
        + __SEG__LINE__ + "\n"
        + "  |-- ETHERNET INFO --|   " + "\n"
        + "Source Mac Address:   " + ad->srcMacAddress + "\n"
        + "Dest Mac Address:     " + ad->dstMacAddress + "\n"
        + "Ethernet type:        " + std::to_string(ad->etherType) + "\n"
        + __SEG__LINE__ + "\n"
        + "  |-- IPV4 INFO --|   " + "\n"
        + "Source IP Address:    " + ad->srcIPAddress + "\n"
        + "Dest IP Address:      " + ad->dstIPAddress + "\n"
        + "IP ID:                " + std::to_string(ad->ipid) + "\n"
        + "TTL:                  " + std::to_string(ad->TTL) + "\n"
        + __SEG__LINE__ + "\n"
        + "  |-- TCP INFO --|   " + "\n"
        + "Source TCP Port:      " + std::to_string(ad->srcTcpPort) + "\n"
        + "Dest TCP Port:        " + std::to_string(ad->dstTcpPort) + "\n"
        + "TCP FLAGS:            " + ad->tcpFlags + "\n"
        + "Window Size:          " + std::to_string(ad->wdSize) + "\n"
        + __SEG__LINE__ + "\n"
        + "  |-- HTTP INFO --|   " + "\n"
        + "HTTP Method:          " + ad->httpMethod + "\n"
        + "HTTP Request URI:     " + ad->httpRequestUri + "\n"
        + "HTTP Host:            " + ad->httpHost + "\n"
        + "HTTP User Agent:      " + ad->httpUserAgent + "\n"
        + "HTTP Cookie:          " + ad->httpCookie + "\n"
        + "\n";
    IOOP_TYPE io_op = STDOUT;
    IOOp_t tmpIOOp(io_op,httpInfo);
    IO_t* io_ins = IO_t::getInstance();
    
    // all IO operations are executed in IO thread pool
    io_ins->putIOOpInQAndSignal(tmpIOOp);

    // Perhaps IO operations are executed in packet capturing thread when all threads in IO thread pool are busy.
    // if( all threads are busy )
    // {
    //     std::cout << "Thread pool not busy." << std::endl;
    //     io_ins->procIOOp(tmpIOOp);
    // }
    // else 
    // {
    //     std::cout << "Thread pool busy." << std::endl;
    //     io_ins->putIOOpInQAndSignal(tmpIOOp);
    // }
    
}

bool unlockHTTPResponsePacket(pcpp::Packet* httpPacket, p_adwfOn_t ad)
{


    return true;
}

bool httpResponseLogic(adwfOn_t* ad)
{


    return true;
}

void HttpMonitor::printAD(p_adwfOn_t ad)
{

    // std::cout << "start of httpLogic" << std::endl;
    std::string httpInfo = std::string("") 
        + "\n"
        + __SEG__LINE__ + "\n"
        + "  |-- ETHERNET INFO --|   " + "\n"
        + "Source Mac Address:   " + ad->srcMacAddress + "\n"
        + "Dest Mac Address:     " + ad->dstMacAddress + "\n"
        + "Ethernet type:        " + std::to_string(ad->etherType) + "\n"
        + __SEG__LINE__ + "\n"
        + "  |-- IPV4 INFO --|   " + "\n"
        + "Source IP Address:    " + ad->srcIPAddress + "\n"
        + "Dest IP Address:      " + ad->dstIPAddress + "\n"
        + "IP ID:                " + std::to_string(ad->ipid) + "\n"
        + "TTL:                  " + std::to_string(ad->TTL) + "\n"
        + __SEG__LINE__ + "\n"
        + "  |-- TCP INFO --|   " + "\n"
        + "Source TCP Port:      " + std::to_string(ad->srcTcpPort) + "\n"
        + "Dest TCP Port:        " + std::to_string(ad->dstTcpPort) + "\n"
        + "TCP FLAGS:            " + ad->tcpFlags + "\n"
        + "Window Size:          " + std::to_string(ad->wdSize) + "\n"
        + __SEG__LINE__ + "\n"
        + "  |-- HTTP INFO --|   " + "\n"
        + "HTTP Method:          " + ad->httpMethod + "\n"
        + "HTTP Request URI:     " + ad->httpRequestUri + "\n"
        + "HTTP Host:            " + ad->httpHost + "\n"
        + "HTTP User Agent:      " + ad->httpUserAgent + "\n"
        + "HTTP Cookie:          " + ad->httpCookie + "\n"
        + "\n";
    IOOP_TYPE io_op = STDOUT;
    IOOp_t tmpIOOp(io_op,httpInfo);
    IO_t* io_ins = IO_t::getInstance();
    
    // all IO operations are executed in IO thread pool
    io_ins->putIOOpInQAndSignal(tmpIOOp);
    // std::cout << std::endl 
    //     << __SEG__LINE__ << std::endl
    //     << "  |-- ETHERNET INFO --|   " << std::endl
    //     << "Source Mac Address:   " << ad->srcMacAddress << std::endl
    //     << "Dest Mac Address:     " << ad->dstMacAddress << std::endl
    //     << "Ethernet type:        " << ad->etherType << std::endl
    //     << __SEG__LINE__ << std::endl
    //     << "  |-- IPV4 INFO --|   " << std::endl
    //     << "Source IP Address:    " << ad->srcIPAddress << std::endl
    //     << "Dest IP Address:      " << ad->dstIPAddress << std::endl
    //     << "IP ID:                " << ad->ipid << std::endl
    //     << "TTL:                  " << ad->TTL << std::endl
    //     << __SEG__LINE__ << std::endl
    //     << "  |-- TCP INFO --|   " << std::endl
    //     << "Source TCP Port:      " << ad->srcTcpPort << std::endl
    //     << "Dest TCP Port:        " << ad->dstTcpPort << std::endl
    //     << "TCP FLAGS:            " << ad->tcpFlags << std::endl
    //     << "Window Size:          " << ad->wdSize << std::endl
    //     << __SEG__LINE__ << std::endl
    //     << "  |-- HTTP INFO --|   " << std::endl
    //     << "HTTP Method:          " << ad->httpMethod << std::endl
    //     << "HTTP Request URI:     " << ad->httpRequestUri << std::endl
    //     << "HTTP Host:            " << ad->httpHost << std::endl
    //     << "HTTP User Agent:      " << ad->httpUserAgent << std::endl
    //     << "HTTP Cookie:          " << ad->httpCookie << std::endl
    //     << std::endl;
}

// tool functions of class
std::string HttpMonitor::getTcpFlagsByLayer(pcpp::TcpLayer* tcpLayer)
{
    std::string result = "";
    if (tcpLayer->getTcpHeader()->synFlag == 1)
        result += "SYN ";
    if (tcpLayer->getTcpHeader()->ackFlag == 1)
        result += "ACK ";
    if (tcpLayer->getTcpHeader()->pshFlag == 1)
        result += "PSH ";
    if (tcpLayer->getTcpHeader()->cwrFlag == 1)
        result += "CWR ";
    if (tcpLayer->getTcpHeader()->urgFlag == 1)
        result += "URG ";
    if (tcpLayer->getTcpHeader()->eceFlag == 1)
        result += "ECE ";
    if (tcpLayer->getTcpHeader()->rstFlag == 1)
        result += "RST ";
    if (tcpLayer->getTcpHeader()->finFlag == 1)
        result += "FIN ";

    return result;
}

std::string HttpMonitor::getHttpMethodByRequestLayer(pcpp::HttpRequestLayer* httpLayer)
{
    pcpp::HttpRequestLayer::HttpMethod thisMethod = httpLayer->getFirstLine()->getMethod();
    switch( thisMethod )
    {
        case pcpp::HttpRequestLayer::HttpGET : return "GET" ; break;
        case pcpp::HttpRequestLayer::HttpPOST : return "POST" ; break;
        default : break;
    }
    return "OTHER";
}

