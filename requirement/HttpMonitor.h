#ifndef __HTTP_ANALYZER__H__
#define __HTTP_ANALYZER__H__

#include <string>

#include "RawPacket.h"
#include "Packet.h"
#include "SystemUtils.h"
#include "ThreadPoolForIO.h"
#include "Vec.h"

#define HTTPHEADER_NAME_METHOD                  "HTTP Method"
#define HTTPHEADER_VALUE_METHOD_GET             "GET"
#define HTTPHEADER_NAME_REQUEST_URI             "HTTP Request URI"
#define HTTPHEADER_NAME_REQUEST_WHOLE_URL       "HTTP Whole URL"
#define HTTPHEADER_NAME_RESPONSE_WHOLESTATUS    "HTTP Status"
#define HTTP_INFO_TITLE                         "  |-- HTTP INFO --|  "
#define HTTP_INFO_REQUEST_TITLE                 "  |-- HTTP REQUEST INFO --|  "
#define HTTP_INFO_RESPONSE_TITLE                "  |-- HTTP RESPONSE INFO --|  "
#define HTTP_HEADER_NAME_LENGTH                 20

#define DEBUG_UNLOCK_HTTPMONITOR_ON

typedef struct HTTPHeaderFieldClause
{
    std::string Name;
    std::string Value;
    HTTPHeaderFieldClause(std::string Name, std::string Value) : Name(Name), Value(Value) {};
}*p_HTTPHeaderFieldClause;

// base class function to analyze below protocol, specific class to analyzer application layer.
class HttpMonitor : public CLowLayersUnlocker
{
private:
    static HttpMonitor* m_instance;
    HttpMonitor();
    virtual ~HttpMonitor();

    class Garbo 
    {
    public:
        ~Garbo()
        {
            if( HttpMonitor::m_instance != nullptr )
            {
                delete m_instance;
                m_instance = nullptr;
            }
        }
    };
    static Garbo garbo;
    qcy::Vec<HTTPHeaderFieldClause> m_httpHeaderFields;
    
public:
    // inherited from CLowLayersUnlocker
    // bool unlockLowLayers(pcpp::Packet* parsedPacket, p_adwfOn_t ad);
    // std::shared_ptr<std::string> getLowLayerInfo(p_adwfOn_t ad);

    // We make sure that getInstance() is called only by packet capturing thread.
    static HttpMonitor* getInstance() 
    {
        if( m_instance == nullptr )
        {
            m_instance = new HttpMonitor;
        }
        return m_instance;
    }


    /* Interfaces */

    // HTTP Request
    bool unlockHTTPRequestPacket(pcpp::Packet* httpPacket, p_adwfOn_t ad);
    std::shared_ptr<std::string> getHttpRequestInfo(const p_adwfOn_t ad) const;
    void httpRequestLogic(adwfOn_t* ad) const ;
    auto getHttpRequestMsgBody(const pcpp::HttpRequestLayer* httpRequestLayer) const -> std::shared_ptr<std::string>;
    
    
    // HTTP Response
    bool unlockHTTPResponsePacket(pcpp::Packet* httpPacket, p_adwfOn_t ad);
    std::shared_ptr<std::string> getHttpResponseInfo(const p_adwfOn_t ad) const;
    bool httpResponseLogic(adwfOn_t* ad) const ;
    auto getHttpResponseMsgBody(const pcpp::HttpResponseLayer* httpResponseLayer) const -> std::shared_ptr<std::string>;
    
    // other interfaces
    void printAD(const p_adwfOn_t ad);
    void clearHttpHeaderFields();

// tool functions of class
private:
    std::string getHttpMethodByRequestLayer(const pcpp::HttpRequestLayer* httpLayer) const;
    
};


#endif
