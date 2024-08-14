#include "GUnlockPack.h"
#include <string>
#include <pthread.h>
#include <memory>

#include "Packet.h"
#include "TcpLayer.h"
#include "IPv4Layer.h"
#include "EthLayer.h"
#include "HttpLayer.h"
#include "SystemUtils.h"

bool CLowLayersUnlocker::unlockLowLayers(pcpp::Packet* parsedPacket, p_adwfOn_t ad) const 
{
    pcpp::EthLayer* packEthernetLayer ;
    pcpp::IPv4Layer* packIPv4Layer;
    pcpp::TcpLayer* packTCPLayer;
    if( !(packEthernetLayer = parsedPacket->getLayerOfType<pcpp::EthLayer>()) || !(packIPv4Layer = parsedPacket->getLayerOfType<pcpp::IPv4Layer>())
        || !(packTCPLayer = parsedPacket->getLayerOfType<pcpp::TcpLayer>()) ) return false;

    ad->srcMacAddress = packEthernetLayer->getSourceMac().toString();
    ad->dstMacAddress = packEthernetLayer->getDestMac().toString();
    ad->etherType = pcpp::netToHost16(packEthernetLayer->getEthHeader()->etherType);

    ad->srcIPAddress = packIPv4Layer->getSrcIPAddress().toString();
    ad->dstIPAddress = packIPv4Layer->getDstIPAddress().toString();
    ad->ipid = pcpp::netToHost16(packIPv4Layer->getIPv4Header()->ipId);
    ad->TTL = (uint8_t)packIPv4Layer->getIPv4Header()->timeToLive;

    ad->srcTcpPort = packTCPLayer->getSrcPort();
    ad->dstTcpPort = packTCPLayer->getDstPort();
    ad->wdSize = pcpp::netToHost16(packTCPLayer->getTcpHeader()->windowSize);
    ad->tcpFlags = getTcpFlagsByLayer(packTCPLayer);

    return true;
}

std::shared_ptr<std::string> CLowLayersUnlocker::getLowLayerInfo(const p_adwfOn_t ad) const 
{
    std::string httpInfo = std::string("") 
        + "\n"
        + "  |-- ETHERNET INFO --|   " + "\n"
        + __SEG__LINE__ + "\n"
        + "Source Mac Address:   " + ad->srcMacAddress + "\n"
        + "Dest Mac Address:     " + ad->dstMacAddress + "\n"
        + "Ethernet type:        " + std::to_string(ad->etherType) + "\n"
        + __SEG__LINE__ + "\n"
        + "  |-- IPV4 INFO --|   " + "\n"
        + __SEG__LINE__ + "\n"
        + "Source IP Address:    " + ad->srcIPAddress + "\n"
        + "Dest IP Address:      " + ad->dstIPAddress + "\n"
        + "IP ID:                " + std::to_string(ad->ipid) + "\n"
        + "TTL:                  " + std::to_string(ad->TTL) + "\n"
        + __SEG__LINE__ + "\n"
        + "  |-- TCP INFO --|   " + "\n"
        + __SEG__LINE__ + "\n"
        + "Source TCP Port:      " + std::to_string(ad->srcTcpPort) + "\n"
        + "Dest TCP Port:        " + std::to_string(ad->dstTcpPort) + "\n"
        + "TCP FLAGS:            " + ad->tcpFlags + "\n"
        + "Window Size:          " + std::to_string(ad->wdSize) + "\n";
    
    std::shared_ptr<std::string> ret = std::make_shared<std::string>(httpInfo);
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

