#ifndef __GUNLOCKPACK_H__
#define __GUNLOCKPACK_H__

#include <string>
#include <pthread.h>
#include <memory>

#include "Vec.h"


#include "Packet.h"
#include "TcpLayer.h"
#include "UdpLayer.h"
#include "IPv4Layer.h"
#include "EthLayer.h"
#include "HttpLayer.h"
#include "SystemUtils.h"

#define HTTP_PORT       80
#define HTTPS_PORT      443
#define DNS_PORT        53
#define MYSQL_PORT      3306

#define LOWLAYERSINFO_TITLE_ETH                 "  |-- ETHERNET INFO --|   "
#define LOWLAYERSINFO_NAME_MACSRCADDR           "Source Mac Address"
#define LOWLAYERSINFO_NAME_MACDSTADDR           "Dest Mac Address"
#define LOWLAYERSINFO_NAME_ETHERTYPE            "Ethernet type"
#define LOWLAYERSINFO_TITLE_IPV4                "  |-- IPV4 INFO --|   "
#define LOWLAYERSINFO_NAME_IPV4SRCADDR          "Source IP Address"
#define LOWLAYERSINFO_NAME_IPV4DSTADDR          "Dest IP Address"
#define LOWLAYERSINFO_NAME_IPV4IPID             "IP ID"
#define LOWLAYERSINFO_NAME_IPV4TTL              "TTL"
#define LOWLAYERSINFO_TITLE_TCP                 "  |-- TCP INFO --|   "
#define LOWLAYERSINFO_NAME_TCPSRCPORT           "Source TCP Port"
#define LOWLAYERSINFO_NAME_TCPDSTPORT           "Dest TCP Port"
#define LOWLAYERSINFO_NAME_TCPFLAGS             "TCP FLAGS"
#define LOWLAYERSINFO_NAME_TCPWDSIZE            "Window Size"
#define LOWLAYERSINFO_TITLE_UDP                 "  |-- UDP INFO --|   "
#define LOWLAYERSINFO_NAME_UDPSRCPORT           "Source UDP Port"
#define LOWLAYERSINFO_NAME_UDPDSTPORT           "Dest UDP Port"
#define LOWLAYERSINFO_NAME_LENGTH               20


#define __SEG__LINE__ "---------------------------------------------------"
#define CLEAR_ALL_AD_DATA

typedef struct lowLayersInfoItem 
{
    std::string itemName;
    std::string itemValue;
    lowLayersInfoItem(std::string itemName, std::string itemValue) : itemName(itemName), itemValue(itemValue) {};
    lowLayersInfoItem& operator = (const lowLayersInfoItem& robj)
    {
        this->itemName = robj.itemName;
        this->itemValue = robj.itemValue;
        return *this;
    }
}*p_lowLayersInfoItem;

struct DNSQuery_t;

// We make sure there's only one instance of adwfOn_t in our whole program.
// In fact, it's no necessary to clear all fields of ad before the end of CapWorker::procOnPacketArrived(), because a logic processing of a packet is just access to 
// the fields of that certain packet focused on, and these fields will be set by corresponding Monitor before they are accessed.
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

    /* UDP */
    std::string                             srcUdpPort;
    std::string                             dstUdpPort;
    
    /* HTTP */
    std::shared_ptr<std::string>            httpMsgBody;
    std::string                             httpConnection;

    /* http request */
    std::string                             httpMethod;      
    std::string                             httpRequestUri;        
    std::string                             httpHost;
    std::string                             httpWholeURL;
    std::string                             httpUserAgent;
    std::string                             httpCookie;
    std::string                             httpAcceptLanguage;
    std::string                             httpAcceptEncoding;

    /* http response */
    // unknown                              httpResponseVersion;
    int                                     httpResponseStatusCode;
    std::string                             httpResponseStatusCodeString;
    std::string                             httpResponseWholeStatusString;
    std::string                             httpResponseServer;
    std::string                             httpResponseContentType;
    std::string                             httpResponseContentLength;
    std::string                             httpResponseContentEncoding;
    
    /* DNS */
    std::string                             dnsTransactionID;
    qcy::Vec<DNSQuery_t*>                   dnsQueries;


adwfOn_t() 
{
    srcMacAddress = "";     dstMacAddress = "";
    etherType = 0;          srcIPAddress = "";
    dstIPAddress = "";      ipid = 0;
    TTL = 0;                srcTcpPort = 0;
    dstTcpPort = 0;         tcpFlags = "";
    wdSize = 0;

    srcUdpPort = "";        dstUdpPort = "";
    
    httpMethod = "";            httpRequestUri = "";
    httpHost = "";              httpUserAgent = "";
    httpCookie = "";            httpWholeURL = "";
    httpAcceptLanguage = "";    httpAcceptEncoding = "";

    httpResponseStatusCode = 0;         httpResponseStatusCodeString = "";
    httpResponseServer = "";            httpConnection = "";
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

    srcUdpPort = "";        dstUdpPort = "";
    
    /* Application layer */
    httpMethod = "";            httpRequestUri = "";
    httpHost = "";              httpUserAgent = "";
    httpCookie = "";            httpWholeURL = "";
    httpAcceptLanguage = "";    httpAcceptEncoding = "";

    httpResponseStatusCode = 0;         httpResponseStatusCodeString = "";
    httpResponseServer = "";            httpConnection = "";
    httpResponseContentType = "";       httpResponseContentLength = "";
    httpResponseContentEncoding = "";   httpResponseWholeStatusString = "";
}

}adwfOn_t,*p_adwfOn_t;


class CLowLayersUnlocker
{
public:
    CLowLayersUnlocker() {};
    virtual ~CLowLayersUnlocker() {};
    bool unlockEth(const pcpp::Packet* parsedPacket, p_adwfOn_t ad) ;
    bool unlockIPv4(const pcpp::Packet* parsedPacket, p_adwfOn_t ad) ;
    bool unlockTcp(const pcpp::Packet* parsedPacket, p_adwfOn_t ad) ;
    bool unlockUdp(const pcpp::Packet* parsedPacket, p_adwfOn_t ad) ;
    std::shared_ptr<std::string> getLowLayerInfo(const p_adwfOn_t ad) const ;
    virtual bool unlockLowLayers(const pcpp::Packet* parsedPacket, p_adwfOn_t ad) ;
    std::string getTcpFlagsByLayer(const pcpp::TcpLayer* tcpLayer) const;

#ifdef CLEAR_CHEAT
    // cheaty clear of my qcy::Vec. It's unsafe from a perspective of a interface of class but it's definitely safe in my application because I'm clear of every detail of my Vec.
    void cheaty_clearLowLayersInfo();
#else 
    void clearLowLayersInfo();
#endif

protected:
    qcy::Vec<lowLayersInfoItem> m_lowLayersInfo;
};


// mutexes for multithread IO.
// To be determined.


#endif
