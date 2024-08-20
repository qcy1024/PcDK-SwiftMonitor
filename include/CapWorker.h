/**
 * Worker Thread for Capturing Packets and Parsing them.
 */
#ifndef __CAP_WORKER_H__
#define __CAP_WORKER_H__

#include "PcapFileDevice.h"
#include "PcapLiveDeviceList.h"

#include "CRawPacketManager.h"
#include "StatisticsCollector.h"

// global funcfions about packet capturing are wrapped in this class 
class CapWorker
{
public:
    static void init(bool useFilter);
    // the callback.
    static void procPacketArrived(pcpp::RawPacket* packet, pcpp::PcapLiveDevice* dev, void* cookie);
    static pcpp::Packet generateHTTPPacket(int type);
    static bool useFilter;
    
    // debug.
    static void debugDNSPacketArrive(pcpp::Packet* parsedPacket, pcpp::PcapLiveDevice* dev, void* cookie);
};



#endif