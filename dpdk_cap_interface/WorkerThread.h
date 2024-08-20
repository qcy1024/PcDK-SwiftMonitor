#ifndef __WORKERTHREAD__H__
#define __WORKERTHREAD__H__

#include "WorkerThread.h"
#include "DpdkDevice.h"
#include "DpdkDeviceList.h"
#include "Packet.h"
#include "TcpLayer.h"
#include "IPv4Layer.h"
#include "EthLayer.h"
#include "HttpLayer.h"
#include "SystemUtils.h"

#include "GUnlockPack.h"
#include "HttpMonitor.h"
#include "StatisticsCollector.h"
#include "CRawPacketManager.h"


class CapWorkerThread : public pcpp::DpdkWorkerThread
{
 private:
	pcpp::DpdkDevice* m_RxDevice;
	pcpp::DpdkDevice* m_TxDevice;
	bool m_Stop;
	uint32_t m_CoreId;

public:
 	// c'tor
	CapWorkerThread(pcpp::DpdkDevice* rxDevice, pcpp::DpdkDevice* txDevice);

	// d'tor (does nothing)
	~CapWorkerThread() { }

	// implement abstract method

	// start running the worker thread
	bool run(uint32_t coreId);

	// ask the worker thread to stop
	void stop();

	// get worker thread core ID
	uint32_t getCoreId() const;
};

#endif
