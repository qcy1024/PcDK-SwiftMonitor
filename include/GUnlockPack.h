#ifndef __GUNLOCKPACK_H__
#define __GUNLOCKPACK_H__

#include <string>
#include <pthread.h>
#include <memory>

#include "Packet.h"
#include "TcpLayer.h"
#include "IPv4Layer.h"
#include "EthLayer.h"
#include "HttpLayer.h"
#include "SystemUtils.h"

#define __SEG__LINE__ "---------------------------------------------------"

typedef struct adwfOn_t
{

    // logicInCapTh == 1 to dill logic in capture thread and logicInCapTh == 0 to dill logic in other thread.
    bool logicInCapTh;

    // all the type of field of this class may be the same of the returns of PcapPlusPlus official API
    /* Ethernet */
    std::string                             srcMacAddress;
    std::string                             dstMacAddress;
    uint16_t                                etherType;          
    
    /* IPv4 */
    std::string                             srcIPAddress;
    std::string                             dstIPAddress;
    uint16_t                                ipid;
    uint8_t                                 TTL;

    /* TCP */
    uint16_t                                srcTcpPort;
    uint16_t                                dstTcpPort;
    std::string                             tcpFlags;
    uint16_t                                wdSize;
    
    /* HTTP */
    std::shared_ptr<std::string>            httpMsgBody;

    /* http request */
    std::string                             httpMethod;      
    std::string                             httpRequestUri;        
    std::string                             httpHost;
    std::string                             httpWholeURL;
    std::string                             httpUserAgent;
    std::string                             httpCookie;


    /* http response */
    // unknown                              httpResponseVersion;
    int                                     httpResponseStatusCode;
    std::string                             httpResponseStatusCodeString;
    std::string                             httpResponseWholeStatusString;
    std::string                             httpResponseServer;
    std::string                             httpResponseConnection;
    std::string                             httpResponseContentType;
    std::string                             httpResponseContentLength;
    std::string                             httpResponseContentEncoding;
    

adwfOn_t() 
{
    srcMacAddress = "";     dstMacAddress = "";
    etherType = 0;          srcIPAddress = "";
    dstIPAddress = "";      ipid = 0;
    TTL = 0;                srcTcpPort = 0;
    dstTcpPort = 0;         tcpFlags = "";
    wdSize = 0;
    
    httpMethod = "";        httpRequestUri = "";
    httpHost = "";          httpUserAgent = "";
    httpCookie = "";        httpWholeURL = "";

    httpResponseStatusCode = 0;         httpResponseStatusCodeString = "";
    httpResponseServer = "";            httpResponseConnection = "";
    httpResponseContentType = "";       httpResponseContentLength = "";
    httpResponseContentEncoding = "";   httpResponseWholeStatusString = "";
}

void clearAll() 
{
    /* Low layers */
    srcMacAddress = "";     dstMacAddress = "";
    etherType = 0;          srcIPAddress = "";
    dstIPAddress = "";      ipid = 0;
    TTL = 0;                srcTcpPort = 0;
    dstTcpPort = 0;         tcpFlags = "";
    wdSize = 0;
    
    /* Application layer */
    httpMethod = "";        httpRequestUri = "";
    httpHost = "";          httpUserAgent = "";
    httpCookie = "";        httpWholeURL = "";

    httpResponseStatusCode = 0;         httpResponseStatusCodeString = "";
    httpResponseServer = "";            httpResponseConnection = "";
    httpResponseContentType = "";       httpResponseContentLength = "";
    httpResponseContentEncoding = "";   httpResponseWholeStatusString = "";
}

}adwfOn_t,*p_adwfOn_t;

class CLowLayersUnlocker
{
public:
    CLowLayersUnlocker() {};
    virtual ~CLowLayersUnlocker() {};
    std::shared_ptr<std::string> getLowLayerInfo(const p_adwfOn_t ad) const ;
    virtual bool unlockLowLayers(pcpp::Packet* parsedPacket, p_adwfOn_t ad) const ;
    std::string getTcpFlagsByLayer(const pcpp::TcpLayer* tcpLayer) const;
};


// mutexes for multithread IO.
// To be determined.


#endif
