
#include "PacketSender.h"
#include "MemoryStream.h"
#include "KBDebug.h"

PacketSender::PacketSender(NetworkInterface* pNetworkInterface) :
	pNetworkInterface_(pNetworkInterface)
{
}

PacketSender::~PacketSender()
{
}

bool PacketSender::send(MemoryStream* pMemoryStream)
{
	return pNetworkInterface_->getEndPoint().send(pMemoryStream->data() + pMemoryStream->rpos(), pMemoryStream->length());
}