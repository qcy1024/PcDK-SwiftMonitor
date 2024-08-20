/** 
 * This class's name should be "PacketProcessor", I will change it in future. 
 * RawPacket and ParsedPacket manager.
 * manipulate packet and execute the corresponding logic based on whether the packet is a TCP or UDP packet or other types maybe.
 */

#ifndef __RAW_PACKET_MANAGER__H__
#define __RAW_PACKET_MANAGER__H__

#include <pthread.h>
#include <string>

#include "CapWorker.h"
#include "PcapFileDevice.h"
#include "PcapLiveDeviceList.h"
#include "HttpMonitor.h"
#include "DNSMonitor.h"
#include "DNSStatisticsCollector.h"


#define HTTP_REQUEST_PACKET_RECEIVED    "|  * * * * Http request packet received. * * * *  |"
#define HTTP_RESPONSE_PACKET_RECEIVED   "|  * * * * Http response packet received. * * * * |"
#define DNS_PACKET_RECEIVED             "|  *  *  *  *  DNS packet received.  *  *  *  *   |"

#define CLEAR_ALL_AD_DATA

class CRawPacketManager
{
private:
    CRawPacketManager(){};
    static CRawPacketManager* m_instance;
    static pthread_mutex_t m_mtx_instance;

    class Garbo
    {
    public:
        Garbo(){};
        ~Garbo()
        {
            if( CRawPacketManager::m_instance != nullptr )
            {
                delete m_instance;
                m_instance = nullptr;
            }
        };

    };
    Garbo garbo;

public:
    static CRawPacketManager* getInstance()
    {
        if( m_instance == nullptr )
        {
            pthread_mutex_lock(&CRawPacketManager::m_mtx_instance);
            if( m_instance == nullptr )
            {
                m_instance = new CRawPacketManager;
            }
            pthread_mutex_unlock(&CRawPacketManager::m_mtx_instance);
        }
        return m_instance;
    }

    // interfaces
    bool printRawPacketData(const pcpp::RawPacket* rawPacket) const;
    bool procTcpPacket(pcpp::Packet* tcpPacket, pcpp::PcapLiveDevice* dev, adwfOn_t* ad) const;
    bool procUdpPacket(pcpp::Packet* udpPacket, pcpp::PcapLiveDevice* dev, adwfOn_t* ad) const;

private:
    // for safe.
    bool checkTcpHttp(pcpp::TcpLayer* tcpLayer) const ;
    bool checkTcpHttps(pcpp::TcpLayer* tcpLayer) const ;
    bool checkTcpMysql(pcpp::TcpLayer* tcpLayer) const ;
    bool checkTcpDNS(pcpp::TcpLayer* tcpLayer) const ;

    bool checkUdpDNS(pcpp::UdpLayer* udpLayer) const ;

};




#endif

