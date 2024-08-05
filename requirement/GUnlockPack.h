#ifndef __GUNLOCKPACK_H__
#define __GUNLOCKPACK_H__

#include <string>
#include <pthread.h>
#include "TcpLayer.h"
#include "IPv4Layer.h"
#include "EthLayer.h"
#include "HttpLayer.h"

class ThreadPoolForIO;

enum IOOP_TYPE 
{
    STDOUT
};

typedef struct adwfOn_t
{

    // logicInCapTh == 1 to dill logic in capture thread and logicInCapTh == 0 to dill logic in other thread.
    bool logicInCapTh;

    // all the type of field of this class may be the same of the returns of PcapPlusPlus official API
    /*Ethernet*/
    std::string                             srcMacAddress;
    std::string                             dstMacAddress;
    uint16_t                                etherType;          
    
    /*IPv4*/
    std::string                             srcIPAddress;
    std::string                             dstIPAddress;
    uint16_t                                ipid;
    uint8_t                                 TTL;

    /*TCP*/
    uint16_t                                srcTcpPort;
    uint16_t                                dstTcpPort;
    std::string                             tcpFlags;
    uint16_t                                wdSize;
    
    /* http request */
    std::string                             httpMethod;      
    std::string                             httpRequestUri;        
    std::string                             httpHost;
    std::string                             httpUserAgent;
    std::string                             httpCookie;


    /*http response*/


adwfOn_t() 
{
    srcMacAddress = "";
    dstMacAddress = "";
    etherType = 0;
    srcIPAddress = "";
    dstIPAddress = "";
    ipid = 0;
    TTL = 0;
    srcTcpPort = 0;
    dstTcpPort = 0;
    tcpFlags = "";
    wdSize = 0;
    httpMethod = "";
    httpRequestUri = "";
    httpHost = "";
    httpUserAgent = "";
    httpCookie = "";
}
}adwfOn_t,*p_adwfOn_t;

typedef struct IOOp_t 
{
    std::string msg;
    IOOP_TYPE opType;
    IOOp_t(IOOP_TYPE opType, std::string msg) : msg(msg), opType(opType) {};
}IOOp_t, *p_IOOp_t;


// mutexes for multithread IO.
// To be determined


#endif
