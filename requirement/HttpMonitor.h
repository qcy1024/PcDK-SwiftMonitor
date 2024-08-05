#ifndef __HTTP_ANALYZER__H__
#define __HTTP_ANALYZER__H__

#include "RawPacket.h"
#include "Packet.h"
#include "GUnlockPack.h"
#include "SystemUtils.h"
#include "ProcIO.h"
#include <string>
#include <queue>
#include "ThreadPoolForIO.h"

// we can inherit a base class maybe name Monitor to get this class HttpMonitor 
// base class function to analyze below protocol, specific class to analyzer apply layer.
class HttpMonitor
{
private:
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
    static HttpMonitor* m_instance;
public:
    static HttpMonitor* getInstance() 
    {
        if( m_instance == nullptr )
        {
            m_instance = new HttpMonitor;
        }
        return m_instance;
    }

    // adwfOn stand for all data we focused on
    bool unlockHTTPRequestPacket(pcpp::Packet* httpPacket, p_adwfOn_t ad);
    void httpRequestLogic(adwfOn_t* ad);

    bool unlockHTTPResponsePacket(pcpp::Packet* httpPacket, p_adwfOn_t ad);
    bool httpResponseLogic(adwfOn_t* ad);

    void printAD(p_adwfOn_t ad);

// tools function of class
private:
    std::string getTcpFlagsByLayer(pcpp::TcpLayer* tcpLayer);
    std::string getHttpMethodByRequestLayer(pcpp::HttpRequestLayer* httpLayer);
    
};


#endif

