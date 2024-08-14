#ifndef __STATISTICS_COLLECTOR_H__
#define __STATISTICS_COLLECTOR_H__

#include <string>
#include <map>
#include <memory>
#include <iostream>

#include "GUnlockPack.h"

// Maybe BIG NUMBER CLASS in future. 
#define TYPE_STAT_IPPACK_NUM         uint32_t
#define TYPE_STAT_NIPPACK_NUM        uint16_t
#define TYPE_STAT_TCPPACK_NUM        uint32_t
#define TYPE_STAT_HTTPPACK_NUM       uint32_t
#define TYPE_STAT_HTTPHOSTNAME       uint16_t
#define TYPE_STAT_HTTPREQUEST_NUM    uint32_t
#define TYPE_STAT_HTTPRESPONSE_NUM   uint32_t
#define TYPE_STAT_REQUEST_URI        uint64_t
#define TYPE_STAT_REQUEST_HOST       uint32_t

struct packetTypeCnt_t
{
    int typeNo;
    std::string typeName;
    unsigned int cnt;
};

// If we have necessary to diliver some classes such as "HttpRequestStatisticCollector" by inherited from this class? I don't know now.
// Looks like no necessary for now.
class StatisticCollector
{
public:
    static StatisticCollector* getInstance()
    {
        if( m_instance == nullptr )
        {
            pthread_mutex_lock(&StatisticCollector::m_mtx_ins);
            if( m_instance == nullptr )
            {
                m_instance = new StatisticCollector;
            }
            pthread_mutex_unlock(&StatisticCollector::m_mtx_ins);
        }
        return m_instance;
    }

    /* interfaces */
    bool collectEth(const p_adwfOn_t ad);
    bool collectIPv4(const p_adwfOn_t ad);

    bool collectHttpRequest(const p_adwfOn_t ad, const pcpp::Packet& parsedPacket);
    bool collectHttpResponse(const p_adwfOn_t ad, const pcpp::Packet& parsedPacket);

    std::shared_ptr<std::string> getAllStatDatas();

    inline TYPE_STAT_IPPACK_NUM        getIPPackNum()  const { return ipPackNum; };
    inline TYPE_STAT_NIPPACK_NUM       getNIPPackNum() const { return nipPackNum; };
    inline TYPE_STAT_TCPPACK_NUM       getTcpPackNum() const { return tcpPackNum; };
    inline TYPE_STAT_HTTPPACK_NUM      getHttpPackNum() const { return httpPackNum; };
    inline TYPE_STAT_HTTPREQUEST_NUM   getHttpRequestNum() const { return httpRequestNum; };
    inline TYPE_STAT_HTTPRESPONSE_NUM  getHttpResponseNum() const { return httpResponseNum; };


// Maybe protected.
private:
    StatisticCollector();
    virtual ~StatisticCollector();

    static StatisticCollector* m_instance;
    static pthread_mutex_t m_mtx_ins;
    struct Garbo
    {
        ~Garbo() 
        {
            if( StatisticCollector::m_instance != nullptr )
            {
                delete StatisticCollector::m_instance;
                StatisticCollector::m_instance == nullptr;
            }
        }
    };
    Garbo garbo;
    
    std::map<std::string, TYPE_STAT_IPPACK_NUM> m_ethStat;
    std::map<std::string, TYPE_STAT_IPPACK_NUM> m_IPv4Stat;

    std::map<std::string, TYPE_STAT_REQUEST_URI> m_httpRequestUrlStat;
    std::map<std::string, TYPE_STAT_REQUEST_HOST> m_httpRequestHost;

    TYPE_STAT_IPPACK_NUM         ipPackNum;
    TYPE_STAT_NIPPACK_NUM        nipPackNum;
    TYPE_STAT_TCPPACK_NUM        tcpPackNum;
    TYPE_STAT_HTTPPACK_NUM       httpPackNum;
    // Index 0 indicate the number of "GET" and index 1 indicate the number of "POST"
    TYPE_STAT_HTTPREQUEST_NUM    httpRequestMethods[3];
    TYPE_STAT_HTTPREQUEST_NUM    httpRequestNum;
    TYPE_STAT_HTTPRESPONSE_NUM   httpResponseNum;
    TYPE_STAT_HTTPRESPONSE_NUM   responseWithContentLength;
    

// may be public, but public is not good.
private:
    inline void incIPPackNum()      { ++ipPackNum; };       
    inline void incNIPPackNum()     { ++nipPackNum; };
    inline void incTcpPackNum()     { ++tcpPackNum; };
    
};

#endif 

