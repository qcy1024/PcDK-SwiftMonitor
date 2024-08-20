#include "HttpMonitor.h"
#include <iostream>

HttpMonitor* HttpMonitor::m_instance = nullptr;
extern CRawPacketManager* g_packManager;

/* interfaces */
HttpMonitor::HttpMonitor() : CLowLayersUnlocker()
{

}

HttpMonitor::~HttpMonitor()
{
    
}

bool HttpMonitor::unlockHTTPRequestPacket(pcpp::Packet* httpPacket,
                                          p_adwfOn_t ad) 
{
#ifdef DEBUG_MODE_GLOBAL
    std::cout << "start of unlock Http request." << std::endl;
#endif
    unlockLowLayers(httpPacket, ad);
    pcpp::HttpRequestLayer* packHttpRequestLayer;
    // verify the packet has http request layer.
    if( !(packHttpRequestLayer = httpPacket->getLayerOfType<pcpp::HttpRequestLayer>()) )
    {
        return false;
    }

    ad->httpMethod = getHttpMethodByRequestLayer(packHttpRequestLayer);
    ad->httpRequestUri = packHttpRequestLayer->getFirstLine()->getUri();

    m_httpHeaderFields.push_back(HTTPHeaderFieldClause(HTTPHEADER_NAME_METHOD,ad->httpMethod));
    m_httpHeaderFields.push_back(HTTPHeaderFieldClause(HTTPHEADER_NAME_REQUEST_URI,ad->httpRequestUri));

    pcpp::HeaderField* hf;
    if( hf = packHttpRequestLayer->getFieldByName(PCPP_HTTP_HOST_FIELD) ) {
        ad->httpHost = hf->getFieldValue();
        m_httpHeaderFields.push_back(HTTPHeaderFieldClause(PCPP_HTTP_HOST_FIELD,ad->httpHost));
    }
    else ad->httpHost = "";
    ad->httpWholeURL = ad->httpHost + ad->httpRequestUri;
    m_httpHeaderFields.push_back(HTTPHeaderFieldClause(HTTPHEADER_NAME_REQUEST_WHOLE_URL,ad->httpWholeURL));
    if( hf = packHttpRequestLayer->getFieldByName(PCPP_HTTP_USER_AGENT_FIELD) ) {
        ad->httpUserAgent = hf->getFieldValue();
        m_httpHeaderFields.push_back(HTTPHeaderFieldClause(PCPP_HTTP_USER_AGENT_FIELD,ad->httpUserAgent));
    }
    else ad->httpUserAgent = "";

    if( packHttpRequestLayer->getFieldByName(PCPP_HTTP_COOKIE_FIELD) )
    {
        ad->httpCookie = packHttpRequestLayer->getFieldByName(PCPP_HTTP_COOKIE_FIELD)->getFieldValue();
        m_httpHeaderFields.push_back(HTTPHeaderFieldClause(PCPP_HTTP_COOKIE_FIELD,ad->httpCookie));
    } 
    else ad->httpCookie = "";

    if( hf = packHttpRequestLayer->getFieldByName(PCPP_HTTP_ACCEPT_LANGUAGE_FIELD) )
    {
        ad->httpAcceptLanguage = packHttpRequestLayer->getFieldByName(PCPP_HTTP_ACCEPT_LANGUAGE_FIELD)->getFieldValue();
        m_httpHeaderFields.push_back(HTTPHeaderFieldClause(PCPP_HTTP_ACCEPT_LANGUAGE_FIELD,ad->httpAcceptLanguage));
    }
    else ad->httpAcceptLanguage = "";

    if( hf = packHttpRequestLayer->getFieldByName(PCPP_HTTP_ACCEPT_ENCODING_FIELD) ) {
        ad->httpAcceptEncoding = packHttpRequestLayer->getFieldByName(PCPP_HTTP_ACCEPT_ENCODING_FIELD)->getFieldValue();
        m_httpHeaderFields.push_back(HTTPHeaderFieldClause(PCPP_HTTP_ACCEPT_ENCODING_FIELD,ad->httpAcceptEncoding));
    }
    else ad->httpAcceptEncoding = "";

    if( hf = packHttpRequestLayer->getFieldByName(PCPP_HTTP_CONNECTION_FIELD) )  {
        ad->httpConnection = hf->getFieldValue();
        m_httpHeaderFields.push_back(HTTPHeaderFieldClause(PCPP_HTTP_CONNECTION_FIELD,ad->httpConnection));
    }
    else ad->httpConnection = "";


    ad->httpMsgBody = getHttpRequestMsgBody(packHttpRequestLayer);
#ifdef DEBUG_MODE_GLOBAL
    std::cout << "end of unlockHttpRequest." << std::endl;
#endif

    return true;

}

std::shared_ptr<std::string> HttpMonitor::getHttpRequestInfo(const p_adwfOn_t ad) const
{
    std::string httpInfo = std::string("") 
        + *(getLowLayerInfo(ad)) + "\n"
        + __SEG__LINE__ + "\n"
        + HTTP_INFO_REQUEST_TITLE + "\n";

    for( qcy::Vec<HTTPHeaderFieldClause>::iterator it = m_httpHeaderFields.begin(); it != m_httpHeaderFields.end(); ++it )
    {
        size_t nameLen = it->Name.size();
        httpInfo = httpInfo + it->Name + ": ";
        for( size_t i = nameLen; i < HTTP_HEADER_NAME_LENGTH; i++ ) httpInfo = httpInfo + " ";
        httpInfo = httpInfo + it->Value + "\n";
    }
    httpInfo = httpInfo + __SEG__LINE__ + "\n" + "\n";

    return std::make_shared<std::string>(httpInfo);
}

// print the http request information to stdout. In future may be other logics we need.
void HttpMonitor::httpRequestLogic(adwfOn_t* ad) const 
{
#ifdef DEBUG_MODE_GLOBAL
    std::cout << "start of httpLogic" << std::endl;
#endif
    std::shared_ptr<std::string> shp_httpInfo = getHttpRequestInfo(ad);
    TYPE_IOOP io_op = STDOUT;
    IOOp_t tmpIOOp(io_op,*shp_httpInfo);
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

auto 
HttpMonitor::getHttpRequestMsgBody(const pcpp::HttpRequestLayer* httpRequestLayer) const -> std::shared_ptr<std::string>
{
    // try to put msgBody in ad.
    std::string httpRequestMsg = std::string("");
    size_t packHeaderLen = httpRequestLayer->getHeaderLen();
    unsigned char* hdBegin = httpRequestLayer->getData();
    unsigned char* bdBegin = hdBegin + packHeaderLen;
    size_t packBodyLen = httpRequestLayer->getDataLen() - packHeaderLen;
    for( int i = 0; i < packBodyLen ; i ++ )
    {
        httpRequestMsg += *(bdBegin+i) ;
    }
    std::shared_ptr<std::string> ret = std::make_shared<std::string>(httpRequestMsg);
    // std::cout << httpRequestMsg << std::endl;
    return ret;
}

bool HttpMonitor::unlockHTTPResponsePacket(pcpp::Packet* httpPacket,
                                           p_adwfOn_t ad)
{
    unlockLowLayers(httpPacket, ad);
    pcpp::HttpResponseLayer* packHttpResponseLayer;
    if( !(packHttpResponseLayer = httpPacket->getLayerOfType<pcpp::HttpResponseLayer>()) ) return false;

    ad->httpResponseStatusCode = packHttpResponseLayer->getFirstLine()->getStatusCode();
    ad->httpResponseStatusCodeString = packHttpResponseLayer->getFirstLine()->getStatusCodeString();
    ad->httpResponseWholeStatusString = std::to_string(ad->httpResponseStatusCode) + ad->httpResponseStatusCodeString;
    m_httpHeaderFields.push_back(HTTPHeaderFieldClause(HTTPHEADER_NAME_RESPONSE_WHOLESTATUS,ad->httpResponseWholeStatusString));

    pcpp::HeaderField* hf;
    if( hf = packHttpResponseLayer->getFieldByName(PCPP_HTTP_SERVER_FIELD) )  {
        ad->httpResponseServer = hf->getFieldValue();
        m_httpHeaderFields.push_back(HTTPHeaderFieldClause(PCPP_HTTP_SERVER_FIELD,ad->httpResponseServer));
    }
    else ad->httpResponseServer = "";

    if( hf = packHttpResponseLayer->getFieldByName(PCPP_HTTP_CONNECTION_FIELD) )  {
        ad->httpConnection = hf->getFieldValue();
        m_httpHeaderFields.push_back(HTTPHeaderFieldClause(PCPP_HTTP_CONNECTION_FIELD,ad->httpConnection));
    }
    else ad->httpConnection = "";

    if( hf = packHttpResponseLayer->getFieldByName(PCPP_HTTP_CONTENT_TYPE_FIELD) )  {
        ad->httpResponseContentType = hf->getFieldValue();
        m_httpHeaderFields.push_back(HTTPHeaderFieldClause(PCPP_HTTP_CONTENT_TYPE_FIELD,ad->httpResponseContentType));
    }
    else ad->httpResponseContentType = "";

    if( hf = packHttpResponseLayer->getFieldByName(PCPP_HTTP_CONTENT_LENGTH_FIELD) )  {
        ad->httpResponseContentLength = hf->getFieldValue();
        m_httpHeaderFields.push_back(HTTPHeaderFieldClause(PCPP_HTTP_CONTENT_LENGTH_FIELD,ad->httpResponseContentLength));
    }
    else ad->httpResponseContentLength = "";

    if( hf = packHttpResponseLayer->getFieldByName(PCPP_HTTP_CONTENT_ENCODING_FIELD) ) {
        ad->httpResponseContentEncoding = hf->getFieldValue();
        m_httpHeaderFields.push_back(HTTPHeaderFieldClause(PCPP_HTTP_CONTENT_ENCODING_FIELD,ad->httpResponseContentEncoding));
    }
    else { ad->httpResponseContentEncoding = "nullptr"; m_httpHeaderFields.push_back(HTTPHeaderFieldClause(PCPP_HTTP_CONTENT_ENCODING_FIELD,ad->httpResponseContentEncoding));}

    ad->httpMsgBody = getHttpResponseMsgBody(packHttpResponseLayer);

    // std::cout << "end of unlockHTTPResponsePacket." << std::endl;


    if( hf = packHttpResponseLayer->getFieldByName("charset") ) {
        std::cout << "Found charset field, charset : " << hf->getFieldValue() << std::endl;
    }
    else std::cout << "No charset field." << std::endl;

    return true;
}

std::shared_ptr<std::string> HttpMonitor::getHttpResponseInfo( adwfOn_t* const ad) const
{
    std::string httpResponseInfo = std::string("") 
        + *(getLowLayerInfo(ad)) + "\n"
        + __SEG__LINE__ + "\n"
        + HTTP_INFO_RESPONSE_TITLE + "\n"
        + __SEG__LINE__ + "\n";

    for( qcy::Vec<HTTPHeaderFieldClause>::iterator it = m_httpHeaderFields.begin(); it != m_httpHeaderFields.end(); ++it )
    {
        size_t nameLen = it->Name.size();
        httpResponseInfo = httpResponseInfo + it->Name + ": ";
        for( size_t i = nameLen; i < HTTP_HEADER_NAME_LENGTH; i++ ) httpResponseInfo = httpResponseInfo + " ";
        httpResponseInfo = httpResponseInfo + it->Value + "\n";
    }
    httpResponseInfo = httpResponseInfo + __SEG__LINE__ + "\n" + "\n";
    
    return std::make_shared<std::string>(httpResponseInfo);
}

bool HttpMonitor::httpResponseLogic(adwfOn_t* ad) const 
{
    std::shared_ptr<std::string> shp_httpResponseInfo = getHttpResponseInfo(ad);
    TYPE_IOOP io_op = STDOUT;
    IOOp_t tmpIOOp(io_op,*shp_httpResponseInfo);
    IO_t* io_ins = IO_t::getInstance();
    
    // all IO operations are executed in IO thread pool
    io_ins->putIOOpInQAndSignal(tmpIOOp);

    return true;
}

auto
HttpMonitor::getHttpResponseMsgBody(const pcpp::HttpResponseLayer* httpResponseLayer) const -> std::shared_ptr<std::string> 
{
    // try to put msgBody in ad.
    std::string httpResponseMsg = std::string("");
    size_t packHeaderLen = httpResponseLayer->getHeaderLen();
    unsigned char* hdBegin = httpResponseLayer->getData();
    unsigned char* bdBegin = hdBegin + packHeaderLen;
    size_t packBodyLen = httpResponseLayer->getDataLen() - packHeaderLen;
    for( int i = 0; i < packBodyLen; i++ )
    {
        httpResponseMsg += *(bdBegin+i) ;
    }
    std::shared_ptr<std::string> ret = std::make_shared<std::string>(httpResponseMsg);
    // std::cout << httpResponseMsg << std::endl;
    return ret;
}

void HttpMonitor::printAD(const p_adwfOn_t ad)
{
    std::string httpInfo = std::string("") 
        + "\n"
        + __SEG__LINE__ + "\n"
        + *(getLowLayerInfo(ad)) + "\n"
        + __SEG__LINE__ + "\n"
        + "  |-- HTTP INFO --|   " + "\n"
        + "HTTP Method:          " + ad->httpMethod + "\n"
        + "HTTP Request URI:     " + ad->httpRequestUri + "\n"
        + "HTTP Host:            " + ad->httpHost + "\n"
        + "HTTP User Agent:      " + ad->httpUserAgent + "\n"
        + "HTTP Cookie:          " + ad->httpCookie + "\n"
        + "\n";
    TYPE_IOOP io_op = STDOUT;
    IOOp_t tmpIOOp(io_op,httpInfo);
    IO_t* io_ins = IO_t::getInstance();
    
    // all IO operations are executed in IO thread pool
    io_ins->putIOOpInQAndSignal(tmpIOOp);
}

/* tool functions */
std::string HttpMonitor::getHttpMethodByRequestLayer(const pcpp::HttpRequestLayer* httpLayer) const
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

void HttpMonitor::clearHttpHeaderFields()
{
    m_httpHeaderFields.clear();
}

#ifdef CLEAR_CHEAT
    // cheaty clear of my Vec. It's unsafe from a perspective of a interface of class but it's definitely safe in my application because I'm clear of every detail of my Vec.
    void HttpMonitor::cheaty_clearHttpHeaderFields()
    {
        m_httpHeaderFields.cheatyClear();
    }
#endif

