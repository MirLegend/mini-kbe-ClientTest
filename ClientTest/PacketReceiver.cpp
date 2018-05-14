
#include "PacketReceiver.h"
#include "KBEngine.h"
#include "NetworkInterface.h"
#include "MessageReader.h"
#include "KBDebug.h"

PacketReceiver::PacketReceiver(NetworkInterface* pNetworkInterface):
	pNetworkInterface_(pNetworkInterface),
	pMessageReader_(new MessageReader()),
	pBuffer_(new MemoryStream())
{
}

PacketReceiver::~PacketReceiver()
{
	KBE_SAFE_RELEASE(pMessageReader_);
	KBE_SAFE_RELEASE(pBuffer_);
	
	INFO_MSG("PacketReceiver::~PacketReceiver(), destroyed!");
}

void PacketReceiver::process()
{
	if (!pNetworkInterface_->getEndPoint().good())
	{
		return;
	}
	fd_set	readFDs;
	fd_set	writeFDs;
	struct timeval		nextTimeout;

	FD_ZERO(&readFDs);
	FD_ZERO(&writeFDs);

	KBESOCKET socket = pNetworkInterface_->socket(); //注册监听套接字读取
	FD_SET(socket, &readFDs);

	nextTimeout.tv_sec = 0;
	nextTimeout.tv_usec = 0;

	int countReady = 0;

	{
		countReady = select(socket + 1, &readFDs,
			NULL, NULL, &nextTimeout);
	}

	if (countReady > 0)
	{
		if (this->processRecv(/*expectingPacket:*/true))
		{
			while (this->processRecv(/*expectingPacket:*/false))
			{
				/* pass */;
			}
		}
	}
	else if (countReady == -1)
	{
		// TODO: Clean this up on shutdown
		// if (!breakProcessing_)
		{
			WARNING_MSG("EventDispatcher::processContinuously: "
				"error in select(): {}\n");
		}
	}
	/*FSocket *socket = pNetworkInterface_->socket();
	uint32 DataSize = 0;

	while (socket->HasPendingData(DataSize))
	{
		pBuffer_->resize(FMath::Min(DataSize, 65507u));

		int32 BytesRead = 0;
		if (socket->Recv(pBuffer_->data(), pBuffer_->size(), BytesRead))
		{
			pMessageReader_->process(pBuffer_->data(), 0, BytesRead);
		}
	}*/
}

bool PacketReceiver::processRecv(bool expectingPacket)
{
	EndPoint& ep = pNetworkInterface_->getEndPoint();
	if (!ep.good())
	{
		return false;
	}

	//int len = pReceiveWindow->recvFromEndPoint(*pEndpoint_);

	int len = ep.recv(pBuffer_->data(), pBuffer_->size());
	if (len < 0)
	{
		if (expectingPacket)
		{
			WARNING_MSG("net rev error!!");
		}

		return false;
	}
	else if (len == 0) // 客户端正常退出
	{
		return false;
	}

	pMessageReader_->process(pBuffer_->data(), 0, len);

	return true;
}
