#include "CRawPacketManager.h"

CRawPacketManager* CRawPacketManager::m_instance = nullptr;
pthread_mutex_t CRawPacketManager::m_mtx_instance = PTHREAD_MUTEX_INITIALIZER;

bool CRawPacketManager::printRawPacketData(const pcpp::RawPacket* rawPacket) const
{
    const unsigned char* packBeg = rawPacket->getRawData();
    int packRawDataLen = rawPacket->getRawDataLen();
    std::string rawData(std::string(""));
    for( int i=1; i<=packRawDataLen; i++ )
    {
        rawData += *(packBeg+i);
    }
    IO_t* IO_instance = IO_t::getInstance();
    IOOp_t tmpIOOp(STDOUT, rawData);
    IO_instance->putIOOpInQAndSignal(tmpIOOp);
    return true;
}

