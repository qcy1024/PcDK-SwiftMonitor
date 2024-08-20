#include "WorkerThread.h"
#include <unistd.h>
#include <pthread.h>

extern double totalKBofCapturedPacket;
extern long long totalNumofCapturedPacket;
extern pthread_mutex_t mtx;

CapWorkerThread::CapWorkerThread(pcpp::DpdkDevice* rxDevice, pcpp::DpdkDevice* txDevice) :
	m_RxDevice(rxDevice), m_TxDevice(txDevice), m_Stop(true), m_CoreId(MAX_NUM_OF_CORES+1)
{
}

bool CapWorkerThread::run(uint32_t coreId)
{
	// Register coreId for this worker
	m_CoreId = coreId;
	m_Stop = false;
	// initialize a mbuf packet array of size 64
	pcpp::MBufRawPacket* mbufArr[1024] = {};

	while (!m_Stop)
	{
		// receive packets from RX device
		uint16_t numOfPackets = m_RxDevice->receivePackets(mbufArr, 1024, 0);

		if (numOfPackets > 0)
		{
			// std::cout << "packet received, num: " << numOfPackets << std::endl;
			// send received packet on the TX device
			for( int i=0; i<numOfPackets; i++ )
			{
				pcpp::MBufRawPacket* tmpRawPacket = mbufArr[i];
				pcpp::Packet parsedPacket(tmpRawPacket);

				pcpp::TcpLayer* tcpLayer = parsedPacket.getLayerOfType<pcpp::TcpLayer>();

				HttpMonitor* p_ins_HttpMonitor = HttpMonitor::getInstance();
				StatisticCollector* statCollector = StatisticCollector::getInstance();
				
				// Unlock pack and do logic.
				if( parsedPacket.isPacketOfType(pcpp::HTTPRequest) )
				{
					std::cout << __SEG__LINE__ << std::endl;
					std::cout << "|  * * * * Http request packet received. * * * *  |" << std::endl;
					std::cout << __SEG__LINE__ << std::endl;

					if( !p_ins_HttpMonitor->unlockHTTPRequestPacket(&parsedPacket,ad) )
					{
						std::cout << "Analyze http request packet failed, something went wrong." << std::endl;  
					}
					p_ins_HttpMonitor->httpRequestLogic(ad);
					statCollector->collectHttpRequest(ad, parsedPacket);
				}

				else if( parsedPacket.isPacketOfType(pcpp::HTTPResponse) )
				{
					std::cout << __SEG__LINE__ << std::endl;
					std::cout << "|  * * * * Http response packet received. * * * * |" << std::endl;
					std::cout << __SEG__LINE__ << std::endl;

					if( !p_ins_HttpMonitor->unlockHTTPResponsePacket(&parsedPacket, ad) )
					{
						std::cout << "Analyze http response packet failed, something went wrong." << std::endl;  
					}
					p_ins_HttpMonitor->httpResponseLogic(ad);
					statCollector->collectHttpResponse(ad, parsedPacket);
				}

				else 
				{
					std::cout << "\n not request and not response" << std::endl << std::endl;
				}

			#ifndef CLEAR_CHEAT
				p_ins_HttpMonitor->clearHttpHeaderFields();
			#else 
				p_ins_HttpMonitor->cheaty_clearHttpHeaderFields();
			#endif 
			} // end for( int i=0; i<numOfPackets; i++ )
		} // end if (numOfPackets > 0)
	} // end while(!m_stop)

	return true;
}

void CapWorkerThread::stop()
{
	m_Stop = true;
}

uint32_t CapWorkerThread::getCoreId() const
{
	return m_CoreId;
}
