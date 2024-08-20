#include "GUnlockPack.h"

bool CLowLayersUnlocker::unlockEth(const pcpp::Packet* parsedPacket, p_adwfOn_t ad) 
{
    pcpp::EthLayer* packEthernetLayer ;
    if( !(packEthernetLayer = parsedPacket->getLayerOfType<pcpp::EthLayer>()) )
        return false;
    m_lowLayersInfo.push_back(lowLayersInfoItem(__SEG__LINE__, " "));
    m_lowLayersInfo.push_back(lowLayersInfoItem(LOWLAYERSINFO_TITLE_ETH, " "));
    m_lowLayersInfo.push_back(lowLayersInfoItem(__SEG__LINE__, " "));
    m_lowLayersInfo.push_back(lowLayersInfoItem(LOWLAYERSINFO_NAME_MACSRCADDR,
                                                ad->srcMacAddress = packEthernetLayer->getSourceMac().toString())
                                                );
    m_lowLayersInfo.push_back(lowLayersInfoItem(LOWLAYERSINFO_NAME_MACDSTADDR,
                                                ad->dstMacAddress = packEthernetLayer->getDestMac().toString())
                                                );                     
    m_lowLayersInfo.push_back(lowLayersInfoItem(LOWLAYERSINFO_NAME_ETHERTYPE,
                                                std::to_string(ad->etherType = pcpp::netToHost16(packEthernetLayer->getEthHeader()->etherType)))
                                                );
    return true;
}

bool CLowLayersUnlocker::unlockIPv4(const pcpp::Packet* parsedPacket, p_adwfOn_t ad) 
{
    pcpp::IPv4Layer* packIPv4Layer;
    if( !(packIPv4Layer = parsedPacket->getLayerOfType<pcpp::IPv4Layer>()) )
        return false;
    m_lowLayersInfo.push_back(lowLayersInfoItem(__SEG__LINE__, " "));
    m_lowLayersInfo.push_back(lowLayersInfoItem(LOWLAYERSINFO_TITLE_IPV4," "));
    m_lowLayersInfo.push_back(lowLayersInfoItem(__SEG__LINE__, " "));
    m_lowLayersInfo.push_back(lowLayersInfoItem(LOWLAYERSINFO_NAME_IPV4SRCADDR,
                                                ad->srcIPAddress = packIPv4Layer->getSrcIPAddress().toString()
                                                ));
    m_lowLayersInfo.push_back(lowLayersInfoItem(LOWLAYERSINFO_NAME_IPV4DSTADDR,
                                                ad->dstIPAddress = packIPv4Layer->getDstIPAddress().toString()
                                                ));
    m_lowLayersInfo.push_back(lowLayersInfoItem(LOWLAYERSINFO_NAME_IPV4IPID,
                                                std::to_string(ad->ipid = pcpp::netToHost16(packIPv4Layer->getIPv4Header()->ipId))
                                                ));
    m_lowLayersInfo.push_back(lowLayersInfoItem(LOWLAYERSINFO_NAME_IPV4TTL,
                                                std::to_string(ad->TTL = (uint8_t)packIPv4Layer->getIPv4Header()->timeToLive)
                                                ));                                
    return true;
}

bool CLowLayersUnlocker::unlockTcp(const pcpp::Packet* parsedPacket, p_adwfOn_t ad) 
{
    pcpp::TcpLayer* packTCPLayer;
    if( !(packTCPLayer = parsedPacket->getLayerOfType<pcpp::TcpLayer>()) )
        return false;
    m_lowLayersInfo.push_back(lowLayersInfoItem(__SEG__LINE__, " "));
    m_lowLayersInfo.push_back(lowLayersInfoItem(LOWLAYERSINFO_TITLE_TCP, " "));
    m_lowLayersInfo.push_back(lowLayersInfoItem(__SEG__LINE__, " "));
    m_lowLayersInfo.push_back(lowLayersInfoItem(LOWLAYERSINFO_NAME_TCPSRCPORT,
                                                std::to_string(ad->srcTcpPort = packTCPLayer->getSrcPort())
                                                ));
    m_lowLayersInfo.push_back(lowLayersInfoItem(LOWLAYERSINFO_NAME_TCPDSTPORT,
                                                std::to_string(ad->dstTcpPort = packTCPLayer->getDstPort())
                                                ));
    m_lowLayersInfo.push_back(lowLayersInfoItem(LOWLAYERSINFO_NAME_TCPWDSIZE,
                                                std::to_string(ad->wdSize = pcpp::netToHost16(packTCPLayer->getTcpHeader()->windowSize))
                                                ));
    m_lowLayersInfo.push_back(lowLayersInfoItem(LOWLAYERSINFO_NAME_TCPFLAGS,
                                                ad->tcpFlags = getTcpFlagsByLayer(packTCPLayer)
                                                ));
    
    return true;
}

bool CLowLayersUnlocker::unlockUdp(const pcpp::Packet* parsedPacket, p_adwfOn_t ad) 
{
    pcpp::UdpLayer* packUdpLayer;
    if( !(packUdpLayer = parsedPacket->getLayerOfType<pcpp::UdpLayer>()) )
        return false;
    m_lowLayersInfo.push_back(lowLayersInfoItem(__SEG__LINE__, " "));
    m_lowLayersInfo.push_back(lowLayersInfoItem(LOWLAYERSINFO_TITLE_UDP, " "));
    m_lowLayersInfo.push_back(lowLayersInfoItem(__SEG__LINE__, " "));
    m_lowLayersInfo.push_back(lowLayersInfoItem(LOWLAYERSINFO_NAME_UDPSRCPORT,
                                                ad->srcUdpPort = std::to_string(packUdpLayer->getSrcPort())
                                                ));
    m_lowLayersInfo.push_back(lowLayersInfoItem(LOWLAYERSINFO_NAME_UDPDSTPORT,
                                                ad->dstUdpPort = std::to_string(packUdpLayer->getDstPort())
                                                ));
    return true;
}

bool CLowLayersUnlocker::unlockLowLayers(const pcpp::Packet* parsedPacket, p_adwfOn_t ad) 
{
    if( !unlockEth(parsedPacket, ad) ) return false;
    if( !unlockIPv4(parsedPacket, ad) ) return false;
    if( parsedPacket->isPacketOfType(pcpp::TCP) ) 
    {
        if( !unlockTcp(parsedPacket, ad) ) return false;
    }
    else if( parsedPacket->isPacketOfType(pcpp::UDP) ) 
    {
        if( !unlockUdp(parsedPacket, ad) ) return false;
    }

    return true;
}

std::shared_ptr<std::string> CLowLayersUnlocker::getLowLayerInfo(const p_adwfOn_t ad) const 
{
    // std::string httpInfo = std::string("") 
    //     + "\n"
    //     + "  |-- ETHERNET INFO --|   " + "\n"
    //     + __SEG__LINE__ + "\n"
    //     + "Source Mac Address:   " + ad->srcMacAddress + "\n"
    //     + "Dest Mac Address:     " + ad->dstMacAddress + "\n"
    //     + "Ethernet type:        " + std::to_string(ad->etherType) + "\n"
    //     + __SEG__LINE__ + "\n"
    //     + "  |-- IPV4 INFO --|   " + "\n"
    //     + __SEG__LINE__ + "\n"
    //     + "Source IP Address:    " + ad->srcIPAddress + "\n"
    //     + "Dest IP Address:      " + ad->dstIPAddress + "\n"
    //     + "IP ID:                " + std::to_string(ad->ipid) + "\n"
    //     + "TTL:                  " + std::to_string(ad->TTL) + "\n"
    //     + __SEG__LINE__ + "\n"
    //     + "  |-- TCP INFO --|   " + "\n"
    //     + __SEG__LINE__ + "\n"
    //     + "Source TCP Port:      " + std::to_string(ad->srcTcpPort) + "\n"
    //     + "Dest TCP Port:        " + std::to_string(ad->dstTcpPort) + "\n"
    //     + "TCP FLAGS:            " + ad->tcpFlags + "\n"
    //     + "Window Size:          " + std::to_string(ad->wdSize) + "\n";
    
    // std::shared_ptr<std::string> ret = std::make_shared<std::string>(httpInfo);

    std::string lowLayerInfo = std::string("");
    for( qcy::Vec<lowLayersInfoItem>::iterator it = m_lowLayersInfo.begin(); it != m_lowLayersInfo.end(); ++it )
    {
        lowLayerInfo = lowLayerInfo + it->itemName ;
        for( size_t i = it->itemName.size(); i < LOWLAYERSINFO_NAME_LENGTH; i++ ) {
            lowLayerInfo = lowLayerInfo + " ";
        }
        lowLayerInfo = lowLayerInfo + " " + it->itemValue + "\n";
    }
    std::shared_ptr<std::string> ret = std::make_shared<std::string>(lowLayerInfo);
    return ret;
}

std::string CLowLayersUnlocker::getTcpFlagsByLayer(const pcpp::TcpLayer* tcpLayer) const
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

#ifdef CLEAR_CHEAT
    void CLowLayersUnlocker::cheaty_clearLowLayersInfo()
    {
        m_lowLayersInfo.cheatyClear();
    }
#else 
    void CLowLayersUnlocker::clearLowLayersInfo()
    {
        m_lowLayersInfo.clear();
    }
#endif


