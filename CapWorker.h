#ifndef __CAP_WORKER_H__
#define __CAP_WORKER_H__

#include "PcapFileDevice.h"
#include "PcapLiveDeviceList.h"
#include "HttpMonitor.h"
#include "CRawPacketManager.h"
#include "global.h"
#include "StatisticsCollector.h"

// global funcfions about packet capturing are wrapped in this class 
class CapWorker
{
public:
    static void init(bool useFilter);
    static void procPacketArrived(pcpp::RawPacket* packet, pcpp::PcapLiveDevice* dev, void* cookie);
    static pcpp::Packet generateHTTPPacket(int type);

private:
    static bool useFilter;
};



#endif