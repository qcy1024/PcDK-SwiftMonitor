#include "StatisticsCollector.h"

StatisticCollector* StatisticCollector::m_instance;
pthread_mutex_t StatisticCollector::m_mtx_ins;

/* interfaces */
StatisticCollector::StatisticCollector()
{
    pthread_mutex_init(&m_mtx_ins, nullptr);
    ipPackNum = 0;
    nipPackNum = 0;
    tcpPackNum = 0;
    httpPackNum = 0;
    httpRequestMethods[0] = httpRequestMethods[1] = httpRequestMethods[2] = 0;
    httpRequestNum = 0;
    httpResponseNum = 0;
    responseWithContentLength = 0;
}

StatisticCollector::~StatisticCollector()
{

}

bool StatisticCollector::collectEth(const p_adwfOn_t ad)
{
    if( ad->dstMacAddress != "" && ad->srcMacAddress != "" )
    {
        m_ethStat[ad->dstMacAddress] ++;
        m_ethStat[ad->srcMacAddress] ++;
    }
    return true;
}

bool StatisticCollector::collectIPv4(const p_adwfOn_t ad)
{
    ++ ipPackNum;
    if( ad->dstIPAddress != "" && ad->srcIPAddress != "" )
    {
        m_IPv4Stat[ad->dstIPAddress] ++;
        m_IPv4Stat[ad->srcIPAddress] ++;

    }
    return true;
}

bool StatisticCollector::collectHttpRequest(const p_adwfOn_t ad, const pcpp::Packet& parsedPacket)
{
    collectEth(ad);
    collectIPv4(ad);
    if( ad->httpRequestUri != "" )
    {
        m_httpRequestUrlStat[ad->httpWholeURL] ++;
        m_httpRequestHost[ad->httpHost] ++;
    }
    ++ httpPackNum;
    ++ httpRequestNum;

    pcpp::HttpRequestLayer* packHttpRequestLayer;
    if( !(packHttpRequestLayer = parsedPacket.getLayerOfType<pcpp::HttpRequestLayer>()) ) 
    {
        std::cout << "something went wrong in collectHttpRequest." << std::endl;
        return false;
    }
    pcpp::HttpRequestLayer::HttpMethod thisMethod = packHttpRequestLayer->getFirstLine()->getMethod();
    if( thisMethod == pcpp::HttpRequestLayer::HttpGET ) 
    {
        ++ httpRequestMethods[0];
    }
    else if( thisMethod == pcpp::HttpRequestLayer::HttpPOST ) {
        ++ httpRequestMethods[1];
    }
    else {
        ++ httpRequestMethods[2];
        return false;
    }
    return true;
}

bool StatisticCollector::collectHttpResponse(const p_adwfOn_t ad, const pcpp::Packet& parsedPacket)
{
    collectEth(ad);
    collectIPv4(ad);
    ++ httpPackNum;
    ++ httpResponseNum;
    if( ad->httpResponseContentLength != "" ) 
    {
        ++ responseWithContentLength;
    }
    return true;
}

std::shared_ptr<std::string> StatisticCollector::getAllStatDatas()
{
    std::string tmpHttpRequestURIStat("");
    for( std::map<std::string, TYPE_STAT_REQUEST_URI>::iterator it = m_httpRequestUrlStat.begin(); it != m_httpRequestUrlStat.end(); ++it )
    {
        tmpHttpRequestURIStat = tmpHttpRequestURIStat + it->first + ":  " + std::to_string(it->second) + "\n";
    }
    std::string tmpHttpRequestHostStat("");
    for( std::map<std::string, TYPE_STAT_HTTPREQUEST_NUM>::iterator it = m_httpRequestHost.begin(); it != m_httpRequestHost.end(); ++it )
    {
        tmpHttpRequestHostStat = tmpHttpRequestHostStat + it->first + ":  " + std::to_string(it->second) + "\n";
    }

    std::string collecData = std::string("");
    collecData = collecData + __SEG__LINE__ + "\n"
        + "              |-- HTTP STAT --|  " + "\n" 
        + __SEG__LINE__ + "\n"
        + "HTTP Packet Num:         " + std::to_string(httpPackNum) + "\n"
        + "HTTP Request Num:        " + std::to_string(httpRequestNum) + "\n"
        + "HTTP Response Num:       " + std::to_string(httpResponseNum) + "\n"

        + __SEG__LINE__ + "\n"
        + "          |-- HTTP REQUEST STAT --|  " + "\n"
        + __SEG__LINE__ + "\n"

        + __SEG__LINE__ + "\n"
        + "  |-- HTTP REQUEST URL --|  " + "\n"
        + __SEG__LINE__ + "\n"

        + tmpHttpRequestURIStat + "\n"

        + __SEG__LINE__ + "\n"
        + "  |-- HTTP REQUEST HOST --|  " + "\n"
        + __SEG__LINE__ + "\n"
        + tmpHttpRequestHostStat 
        
        + __SEG__LINE__ + "\n"
        + "  |-- HTTP REQUEST METHOD --|  " + "\n"
        + __SEG__LINE__ + "\n"
        + "GET:     " + std::to_string(httpRequestMethods[0]) + "\n"
        + "POST:    " + std::to_string(httpRequestMethods[1]) + "\n"
        + "OTHER:   " + std::to_string(httpRequestMethods[2]) + "\n"


        + __SEG__LINE__ + "\n"
        + "          |-- HTTP RESPONSE STAT --|  " + "\n"
        + __SEG__LINE__ + "\n"
        + "responses with content-length num:  " + std::to_string(responseWithContentLength) + "\n"

        + "\n" + "\n";

    return std::make_shared<std::string>(collecData);
}
