#include "endpoint.h"
#include "KBDebug.h"

#if KBE_PLATFORM == PLATFORM_WIN32
#include <Iphlpapi.h>
#pragma comment (lib,"iphlpapi.lib") 
#else
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#endif

#ifdef unix
#else	// not unix
	// Need to implement if_nameindex functions on Windows
	/** @internal */
	struct if_nameindex
	{

		unsigned int if_index;	/* 1, 2, ... */

		char *if_name;			/* null terminated name: "eth0", ... */

	};

	/** @internal */
	struct if_nameindex *if_nameindexr(void)
	{
		static struct if_nameindex staticIfList[3] =
		{ { 1, "eth0" }, { 2, "lo" }, { 0, 0 } };

		return staticIfList;
	}

	/** @internal */
	inline void if_freenameindex(struct if_nameindex *)
	{}
#endif	// not unix

static bool g_networkInitted = false;

//-------------------------------------------------------------------------------------
void EndPoint::onReclaimObject()
{
#if KBE_PLATFORM == PLATFORM_WIN32
	socket_ = INVALID_SOCKET;
#else
	socket_ = -1;
#endif

	address_ = Address::NONE;
}

//-------------------------------------------------------------------------------------
bool EndPoint::getClosedPort(Address & closedPort)
{
	bool isResultSet = false;

#ifdef unix
//	KBE_ASSERT(errno == ECONNREFUSED);

	struct sockaddr_in	offender;
	offender.sin_family = 0;
	offender.sin_port = 0;
	offender.sin_addr.s_addr = 0;

	struct msghdr	errHeader;
	struct iovec	errPacket;

	char data[ 256 ];
	char control[ 256 ];

	errHeader.msg_name = &offender;
	errHeader.msg_namelen = sizeof(offender);
	errHeader.msg_iov = &errPacket;
	errHeader.msg_iovlen = 1;
	errHeader.msg_control = control;
	errHeader.msg_controllen = sizeof(control);
	errHeader.msg_flags = 0;	// result only

	errPacket.iov_base = data;
	errPacket.iov_len = sizeof(data);

	int errMsgErr = recvmsg(*this, &errHeader, MSG_ERRQUEUE);
	if (errMsgErr < 0)
	{
		return false;
	}

	struct cmsghdr * ctlHeader;

	for (ctlHeader = CMSG_FIRSTHDR(&errHeader);
		ctlHeader != NULL;
		ctlHeader = CMSG_NXTHDR(&errHeader,ctlHeader))
	{
		if (ctlHeader->cmsg_level == SOL_IP &&
			ctlHeader->cmsg_type == IP_RECVERR) break;
	}

	// Was there an IP_RECVERR error.

	if (ctlHeader != NULL)
	{
		struct sock_extended_err * extError =
			(struct sock_extended_err*)CMSG_DATA(ctlHeader);

		// Only use this address if the kernel has the bug where it does not
		// report the packet details.

		if (errHeader.msg_namelen == 0)
		{
			// Finally we figure out whose fault it is except that this is the
			// generator of the error (possibly a machine on the path to the
			// destination), and we are interested in the actual destination.
			offender = *(sockaddr_in*)SO_EE_OFFENDER(extError);
			offender.sin_port = 0;

			ERROR_MSG("EndPoint::getClosedPort: "
				"Kernel has a bug: recv_msg did not set msg_name.\n");
		}

		closedPort.ip = offender.sin_addr.s_addr;
		closedPort.port = offender.sin_port;

		isResultSet = true;
	}
#endif // unix

	return isResultSet;
}

//-------------------------------------------------------------------------------------
int EndPoint::getBufferSize(int optname) const
{
	KBE_ASSERT(optname == SO_SNDBUF || optname == SO_RCVBUF);

	int recvbuf = -1;
	int rbargsize = sizeof(int);

	int rberr = getsockopt(socket_, SOL_SOCKET, optname,
		(char*)&recvbuf, &rbargsize);

	if (rberr == 0 && rbargsize == sizeof(int))
		return recvbuf;

	INFO_MSG("EndPoint::getBufferSize: "
		"Failed to read option {}: {}\n");

	return -1;
}

//-------------------------------------------------------------------------------------
bool EndPoint::getInterfaces(std::map< uint32, std::string > &interfaces)
{
#if KBE_PLATFORM == PLATFORM_WIN32
	int count = 0;
	char hostname[1024];
	struct hostent* inaddrs;

	if(gethostname(hostname, 1024) == 0)
	{
		inaddrs = gethostbyname(hostname);
		if(inaddrs)
		{
			while(inaddrs->h_addr_list[count])
			{
				unsigned long addrs = *(unsigned long*)inaddrs->h_addr_list[count];
				interfaces[addrs] = "eth0";
				char *ip = inet_ntoa (*(struct in_addr *)inaddrs->h_addr_list[count]);
				DEBUG_MSG("EndPoint::getInterfaces: found eth0 {}\n");
				++count;
			}
		}
	}

	return count > 0;
#else
	struct ifconf ifc;
	char          buf[1024];

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;

	if(ioctl(socket_, SIOCGIFCONF, &ifc) < 0)
	{
		ERROR_MSG("EndPoint::getInterfaces: ioctl(SIOCGIFCONF) failed.\n");
		return false;
	}

	struct ifreq * ifr         = ifc.ifc_req;
	int nInterfaces = ifc.ifc_len / sizeof(struct ifreq);
	for (int i = 0; i < nInterfaces; ++i)
	{
		struct ifreq *item = &ifr[i];

		interfaces[ ((struct sockaddr_in *)&item->ifr_addr)->sin_addr.s_addr ] =
			item->ifr_name;
	}

	return true;
#endif
}

//-------------------------------------------------------------------------------------
int EndPoint::findIndicatedInterface(const char * spec, uint32 & address)
{
	address = 0;

	if(spec == NULL || spec[0] == 0) 
	{
		return -1;
	}

	// 是否指定地址
	if(0 != Address::string2ip(spec, address))
	{
		return -1;
	}
	else if(0 != this->getInterfaceAddressByMAC(spec, address))
	{
		return -1;
	}
	else if(0 != this->getInterfaceAddressByName(spec, address))
	{
		return -1;
	}

	return 0;
}

//-------------------------------------------------------------------------------------
int EndPoint::getInterfaceAddressByName(const char * name, uint32 & address)
{
	int ret = -1;

#if KBE_PLATFORM == PLATFORM_WIN32

    PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
    unsigned long size = sizeof(IP_ADAPTER_INFO);

    int ret_info = ::GetAdaptersInfo(pIpAdapterInfo, &size);

    if (ERROR_BUFFER_OVERFLOW == ret_info)
    {
        delete pIpAdapterInfo;
        pIpAdapterInfo = (PIP_ADAPTER_INFO)new unsigned char[size];
        ret_info = ::GetAdaptersInfo(pIpAdapterInfo, &size);    
    }

    if (ERROR_SUCCESS == ret_info)
    {
		PIP_ADAPTER_INFO _pIpAdapterInfo = pIpAdapterInfo;
		while (_pIpAdapterInfo)
		{
			if(!strcmp(_pIpAdapterInfo->AdapterName, name))
			{
				IP_ADDR_STRING* pIpAddrString = &(_pIpAdapterInfo->IpAddressList);
				ret = Address::string2ip(pIpAddrString->IpAddress.String, address);
				break;
			}

			_pIpAdapterInfo = _pIpAdapterInfo->Next;
		}
    }

    if (pIpAdapterInfo)
    {
        delete pIpAdapterInfo;
    }

#else
	
	int fd;
	int interfaceNum = 0;
	struct ifreq buf[16];
	struct ifconf ifc;

	if((fd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		::close(fd);
		return -1;
	}

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = (caddr_t)buf;

	if(!ioctl(fd, SIOCGIFCONF, (char *)&ifc))
	{
		interfaceNum = ifc.ifc_len / sizeof(struct ifreq);
		while(interfaceNum-- > 0)
		{
			if(!strcmp((char*)buf[interfaceNum].ifr_name, (char*)name))
			{
				if(!ioctl(fd, SIOCGIFADDR, (char *)&buf[interfaceNum]))
				{
					ret = Address::string2ip((const char *)inet_ntoa(((struct sockaddr_in *)&(buf[interfaceNum].ifr_addr))->sin_addr), address);
				}

				break;
			}
		}
	}

	::close(fd);

#endif

	return ret;
}

//-------------------------------------------------------------------------------------
int EndPoint::getInterfaceAddressByMAC(const char * mac, uint32 & address)
{
	int ret = -1;

	if(!mac)
	{
		return ret;
	}

	// mac地址转换
	unsigned char macAddress[16] = {0};
	unsigned char macAddressIdx = 0;
	char szTemp[2] = {0};
	char szTempIdx = 0;
	char* pMac = (char*)mac;
	while(*pMac && macAddressIdx < sizeof(macAddress))
	{
		if(('a' <= *pMac && *pMac <= 'f') || ('A' <= *pMac && *pMac <= 'F') || ('0' <= *pMac && *pMac <= '9'))
		{
			szTemp[szTempIdx++] = *pMac;
			if(szTempIdx > 1)
			{
				macAddress[macAddressIdx++] = (unsigned char)::strtol(szTemp, NULL, 16);
				szTempIdx = 0;
			}
		}

		++pMac;
	}

#if KBE_PLATFORM == PLATFORM_WIN32

	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
	unsigned long size = sizeof(IP_ADAPTER_INFO);

	int ret_info = ::GetAdaptersInfo(pIpAdapterInfo, &size);

	if (ERROR_BUFFER_OVERFLOW == ret_info)
	{
		delete pIpAdapterInfo;
		pIpAdapterInfo = (PIP_ADAPTER_INFO)new unsigned char[size];
		ret_info = ::GetAdaptersInfo(pIpAdapterInfo, &size);    
	}

	if (ERROR_SUCCESS == ret_info)
	{
		PIP_ADAPTER_INFO _pIpAdapterInfo = pIpAdapterInfo;
		while (_pIpAdapterInfo)
		{
			if(!strcmp((char*)_pIpAdapterInfo->Address, (char*)macAddress))
			{
				IP_ADDR_STRING* pIpAddrString = &(_pIpAdapterInfo->IpAddressList);
				ret = Address::string2ip(pIpAddrString->IpAddress.String, address);
				break;
			}

			_pIpAdapterInfo = _pIpAdapterInfo->Next;
		}
	}

	if (pIpAdapterInfo)
	{
		delete pIpAdapterInfo;
	}

#else

	int fd;
	int interfaceNum = 0;
	struct ifreq buf[16];
	struct ifconf ifc;

	if((fd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		::close(fd);
		return -1;
	}

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = (caddr_t)buf;

	if(!ioctl(fd, SIOCGIFCONF, (char *)&ifc))
	{
		interfaceNum = ifc.ifc_len / sizeof(struct ifreq);
		while(interfaceNum-- > 0)
		{
			if(!ioctl(fd, SIOCGIFHWADDR, (char *)(&buf[interfaceNum])))
			{
				if(!strcmp((char*)buf[interfaceNum].ifr_hwaddr.sa_data, (char*)macAddress))
				{
					if(!ioctl(fd, SIOCGIFADDR, (char *)&buf[interfaceNum]))
					{
						ret = Address::string2ip((const char *)inet_ntoa(((struct sockaddr_in *)&(buf[interfaceNum].ifr_addr))->sin_addr), address);
					}

					break;
				}
			}
			else
			{
				break;
			}
		}
	}

	::close(fd);

#endif

	return ret;
}

//-------------------------------------------------------------------------------------
int EndPoint::findDefaultInterface(char * name)
{
#ifndef unix
	strcpy(name, "eth0");
	return 0;
#else
	int		ret = -1;

	struct if_nameindex* pIfInfo = if_nameindex();
	if (pIfInfo)
	{
		int		flags = 0;
		struct if_nameindex* pIfInfoCur = pIfInfo;
		while (pIfInfoCur->if_name)
		{
			flags = 0;
			this->getInterfaceFlags(pIfInfoCur->if_name, flags);

			if ((flags & IFF_UP) && (flags & IFF_RUNNING))
			{
				uint32	addr;
				if (this->getInterfaceAddress(pIfInfoCur->if_name, addr) == 0)
				{
					strcpy(name, pIfInfoCur->if_name);
					ret = 0;

					// we only stop if it's not a loopback address,
					// otherwise we continue, hoping to find a better one
					if (!(flags & IFF_LOOPBACK)) break;
				}
			}
			++pIfInfoCur;
		}
		if_freenameindex(pIfInfo);
	}
	else
	{
		ERROR_MSG(fmt::format("EndPoint::findDefaultInterface: "
							"if_nameindex returned NULL ({})\n",
						kbe_strerror()));
	}

	return ret;
#endif // unix
}

//-------------------------------------------------------------------------------------
int EndPoint::getDefaultInterfaceAddress(uint32 & address)
{
	int ret = -1;

	char interfaceName[256] = {0};
	ret = findDefaultInterface(interfaceName);
	if(0 == ret)
	{
		ret = getInterfaceAddressByName(interfaceName, address);
	}

	if(0 != ret)
	{
		char hostname[256] = {0};
		::gethostname(hostname, sizeof(hostname));
		struct hostent * host = gethostbyname(hostname);
		if(host)
		{
			if(host->h_addr_list[0] < host->h_name)
			{
				address = ((struct in_addr*)(host->h_addr_list[0]))->s_addr;
				ret = 0;
			}
		}
	}

	return ret;
}

//-------------------------------------------------------------------------------------
bool EndPoint::setBufferSize(int optname, int size)
{
	setsockopt(socket_, SOL_SOCKET, optname, (const char*)&size, sizeof(size));

	return this->getBufferSize(optname) >= size;
}

//-------------------------------------------------------------------------------------
bool EndPoint::recvAll(void * gramData, int gramSize)
{
	while (gramSize > 0)
	{
		int len = this->recv(gramData, gramSize);

		if (len <= 0)
		{
			if (len == 0)
			{
				WARNING_MSG("EndPoint::recvAll: Connection lost\n");
			}
			else
			{
				WARNING_MSG("EndPoint::recvAll: Got error '{}'\n");
			}

			return false;
		}
		gramSize -= len;
		gramData = ((char *)gramData) + len;
	}

	return true;
}

//-------------------------------------------------------------------------------------
void EndPoint::initNetwork()
{
	if (g_networkInitted) 
		return;
	
	g_networkInitted = true;

#if KBE_PLATFORM == PLATFORM_WIN32
	WSAData wsdata;
	WSAStartup(0x202, &wsdata);
#endif
}

//-------------------------------------------------------------------------------------
bool EndPoint::waitSend()
{
	fd_set	fds;
	struct timeval tv = { 0, 10000 };
	FD_ZERO( &fds );
	FD_SET(socket_, &fds);

	return select(socket_+1, NULL, &fds, NULL, &tv) > 0;
}


//-------------------------------------------------------------------------------------

EndPoint::EndPoint(/*uint32 networkAddr, uint16 networkPort*/):
#if KBE_PLATFORM == PLATFORM_WIN32
socket_(INVALID_SOCKET)
#else
socket_(-1)
#endif
{
	//if(networkAddr)
	//{
	//	address_.ip = networkAddr;
	//	address_.port = networkPort;
	//}
}

EndPoint::EndPoint(Address address):
#if KBE_PLATFORM == PLATFORM_WIN32
socket_(INVALID_SOCKET)
#else
socket_(-1)
#endif
{
	if(address.ip > 0)
	{
		address_ = address;
	}
}

EndPoint::~EndPoint()
{
	this->close();
}

uint32 EndPoint::getRTT()
{
#if KBE_PLATFORM != PLATFORM_WIN32
	struct tcp_info tcpinfo;
	int len = sizeof(tcpinfo);

	if (getsockopt((*this), SOL_TCP, TCP_INFO, &tcpinfo, &len) != -1)
		return tcpinfo.tcpi_rtt;
#endif

	return 0;
}

bool EndPoint::good() const
{
#if KBE_PLATFORM == PLATFORM_WIN32
	return socket_ != INVALID_SOCKET;
#else
	return socket_ != -1;
#endif
}

INLINE EndPoint::operator KBESOCKET() const
{
	return socket_;
}

KBESOCKET EndPoint::socket() const
{
	return socket_;
}

INLINE void EndPoint::setFileDescriptor(int fd)
{
	socket_ = fd;
}

void EndPoint::socket(int type)
{
	this->setFileDescriptor(::socket(AF_INET, type, 0));
#if KBE_PLATFORM == PLATFORM_WIN32
	if ((socket_ == INVALID_SOCKET) && (WSAGetLastError() == WSANOTINITIALISED))
	{
		EndPoint::initNetwork();
		this->setFileDescriptor(::socket(AF_INET, type, 0));
		KBE_ASSERT((socket_ != INVALID_SOCKET) && (WSAGetLastError() != WSANOTINITIALISED) && \
				"EndPoint::socket: create socket is error!");
	}
#endif
}

INLINE int EndPoint::setnodelay(bool nodelay)
{
	int arg = int(nodelay);
	return setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, (char*)&arg, sizeof(int));
}

INLINE int EndPoint::setnonblocking(bool nonblocking)
{
#ifdef unix
	int val = nonblocking ? O_NONBLOCK : 0;
	return ::fcntl(socket_, F_SETFL, val);
#else
	u_long val = nonblocking ? 1 : 0;
	return ::ioctlsocket(socket_, FIONBIO, &val);
#endif
}

INLINE int EndPoint::setbroadcast(bool broadcast)
{
#ifdef unix
	int val;
	if (broadcast)
	{
		val = 2;
		::setsockopt(socket_, SOL_IP, IP_MULTICAST_TTL, &val, sizeof(int));
	}
#else
	bool val;
#endif
	val = broadcast ? 1 : 0;
	return ::setsockopt(socket_, SOL_SOCKET, SO_BROADCAST, (char*)&val, sizeof(val));
}

INLINE int EndPoint::setreuseaddr(bool reuseaddr)
{
#ifdef unix
	int val;
#else
	bool val;
#endif
	val = reuseaddr ? 1 : 0;
	return ::setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR,
		(char*)&val, sizeof(val));
}

INLINE int EndPoint::setlinger(uint16 onoff, uint16 linger)
{
	struct linger l = { 0 };
	l.l_onoff = onoff;
	l.l_linger = linger;
	return setsockopt(socket_, SOL_SOCKET, SO_LINGER, (const char *) &l, sizeof(l));
}

INLINE int EndPoint::setkeepalive(bool keepalive)
{
#ifdef unix
	int val;
#else
	bool val;
#endif
	val = keepalive ? 1 : 0;
	return ::setsockopt(socket_, SOL_SOCKET, SO_KEEPALIVE,
		(char*)&val, sizeof(val));
}

INLINE int EndPoint::bind(uint16 networkPort, uint32 networkAddr)
{
	sockaddr_in	sin;
	memset(&sin, 0, sizeof(sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = networkPort;
	sin.sin_addr.s_addr = networkAddr;
	return ::bind(socket_, (struct sockaddr*)&sin, sizeof(sin));
}

INLINE int EndPoint::joinMulticastGroup(uint32 networkAddr)
{
#ifdef unix
	struct ip_mreqn req;
	req.imr_multiaddr.s_addr = networkAddr;
	req.imr_address.s_addr = INADDR_ANY;
	req.imr_ifindex = 0;
	return ::setsockopt(socket_, SOL_IP, IP_ADD_MEMBERSHIP, &req, sizeof(req));
#else
	return -1;
#endif
}

INLINE int EndPoint::quitMulticastGroup(uint32 networkAddr)
{
#ifdef unix
	struct ip_mreqn req;
	req.imr_multiaddr.s_addr = networkAddr;
	req.imr_address.s_addr = INADDR_ANY;
	req.imr_ifindex = 0;
	return ::setsockopt(socket_, SOL_IP, IP_DROP_MEMBERSHIP,&req, sizeof(req));
#else
	return -1;
#endif
}

INLINE int EndPoint::close()
{
	if (socket_ == -1)
	{
		return 0;
	}

#ifdef unix
	int ret = ::close(socket_);
#else
	int ret = ::closesocket(socket_);
#endif

	if (ret == 0)
	{
		this->setFileDescriptor(-1);
	}

	return ret;
}

INLINE int EndPoint::getlocaladdress(
	uint16 * networkPort, uint32 * networkAddr) const
{
	sockaddr_in		sin;
	int		sinLen = sizeof(sin);
	int ret = ::getsockname(socket_, (struct sockaddr*)&sin, &sinLen);
	if (ret == 0)
	{
		if (networkPort != NULL) *networkPort = sin.sin_port;
		if (networkAddr != NULL) *networkAddr = sin.sin_addr.s_addr;
	}
	return ret;
}

INLINE int EndPoint::getremoteaddress(
	uint16 * networkPort, uint32 * networkAddr) const
{
	sockaddr_in		sin;
	int		sinLen = sizeof(sin);
	int ret = ::getpeername(socket_, (struct sockaddr*)&sin, &sinLen);
	if (ret == 0)
	{
		if (networkPort != NULL) *networkPort = sin.sin_port;
		if (networkAddr != NULL) *networkAddr = sin.sin_addr.s_addr;
	}
	return ret;
}

INLINE const char * EndPoint::c_str() const
{
	return address_.c_str();
}

const Address& EndPoint::addr() const
{
	return address_;
}

void EndPoint::addr(const Address& newAddress)
{
	address_ = newAddress;
}

void EndPoint::addr(uint16 newNetworkPort, uint32 newNetworkAddress)
{
	address_.port = newNetworkPort;
	address_.ip = newNetworkAddress;
}

INLINE int EndPoint::getremotehostname(std::string * host) const
{
	sockaddr_in		sin;
	int		sinLen = sizeof(sin);
	int ret = ::getpeername(socket_, (struct sockaddr*)&sin, &sinLen);
	if (ret == 0)
	{
		hostent* h = gethostbyaddr((char*) &sin.sin_addr,
				sizeof(sin.sin_addr), AF_INET);

		if (h)
		{
			*host = h->h_name;
		}
		else
		{
			ret = -1;
		}
	}

	return ret;
}

INLINE int EndPoint::sendto(void * gramData, int gramSize,
	uint16 networkPort, uint32 networkAddr)
{
	sockaddr_in	sin;
	sin.sin_family = AF_INET;
	sin.sin_port = networkPort;
	sin.sin_addr.s_addr = networkAddr;

	return this->sendto(gramData, gramSize, sin);
}

INLINE int EndPoint::sendto(void * gramData, int gramSize,
	struct sockaddr_in & sin)
{
	return ::sendto(socket_, (char*)gramData, gramSize,
		0, (sockaddr*)&sin, sizeof(sin));
}

INLINE int EndPoint::recvfrom(void * gramData, int gramSize,
	uint16 * networkPort, uint32 * networkAddr)
{
	sockaddr_in sin;
	int result = this->recvfrom(gramData, gramSize, sin);

	if (result >= 0)
	{
		if (networkPort != NULL) *networkPort = sin.sin_port;
		if (networkAddr != NULL) *networkAddr = sin.sin_addr.s_addr;
	}

	return result;
}

INLINE int EndPoint::recvfrom(void * gramData, int gramSize,
	struct sockaddr_in & sin)
{
	int		sinLen = sizeof(sin);
	int ret = ::recvfrom(socket_, (char*)gramData, gramSize,
		0, (sockaddr*)&sin, &sinLen);

	return ret;
}

INLINE int EndPoint::listen(int backlog)
{
	return ::listen(socket_, backlog);
}

int EndPoint::connect(bool autosetflags)
{
	return connect(address_.port, address_.ip, autosetflags);
}

int EndPoint::connect(uint16 networkPort, uint32 networkAddr, bool autosetflags)
{
	sockaddr_in	sin;
	sin.sin_family = AF_INET;
	sin.sin_port = networkPort;
	sin.sin_addr.s_addr = networkAddr;

	int ret = ::connect(socket_, (sockaddr*)&sin, sizeof(sin));
	if(autosetflags)
	{
		setnonblocking(true);
		setnodelay(true);
	}
	
	return ret;
}

INLINE EndPoint * EndPoint::accept(uint16 * networkPort, uint32 * networkAddr, bool autosetflags)
{
	sockaddr_in		sin;
	int		sinLen = sizeof(sin);
	int ret = ::accept(socket_, (sockaddr*)&sin, &sinLen);

#if defined(unix)
	if (ret < 0) return NULL;
#else
	if (ret == INVALID_SOCKET) return NULL;
#endif

	EndPoint * pNew = new EndPoint();

	pNew->setFileDescriptor(ret);
	pNew->addr(sin.sin_port, sin.sin_addr.s_addr);
	
	if(autosetflags)
	{
		pNew->setnonblocking(true);
		pNew->setnodelay(true);
	}
	
	if (networkPort != NULL) *networkPort = sin.sin_port;
	if (networkAddr != NULL) *networkAddr = sin.sin_addr.s_addr;

	return pNew;
}

 int EndPoint::send(const void * gramData, int gramSize)
{
	return ::send(socket_, (char*)gramData, gramSize, 0);
}

 int EndPoint::recv(void * gramData, int gramSize)
{
	return ::recv(socket_, (char*)gramData, gramSize, 0);
}


#ifdef unix
INLINE int EndPoint::getInterfaceFlags(char * name, int & flags)
{
	struct ifreq	request;

	strncpy(request.ifr_name, name, IFNAMSIZ);
	if (ioctl(socket_, SIOCGIFFLAGS, &request) != 0)
	{
		return -1;
	}

	flags = request.ifr_flags;
	return 0;
}

INLINE int EndPoint::getInterfaceAddress(const char * name, uint32 & address)
{
	struct ifreq	request;

	strncpy(request.ifr_name, name, IFNAMSIZ);
	if (ioctl(socket_, SIOCGIFADDR, &request) != 0)
	{
		return -1;
	}

	if (request.ifr_addr.sa_family == AF_INET)
	{
		address = ((sockaddr_in*)&request.ifr_addr)->sin_addr.s_addr;
		return 0;
	}
	else
	{
		return -1;
	}
}

INLINE int EndPoint::getInterfaceNetmask(const char * name,
	uint32 & netmask)
{
	struct ifreq request;
	strncpy(request.ifr_name, name, IFNAMSIZ);

	if (ioctl(socket_, SIOCGIFNETMASK, &request) != 0)
	{
		return -1;
	}

	netmask = ((sockaddr_in&)request.ifr_netmask).sin_addr.s_addr;

	return 0;
}

#else

INLINE int EndPoint::getInterfaceAddress(const char * name, uint32 & address)
{
	if (!strcmp(name, "eth0"))
	{
		char	myname[256];
		::gethostname(myname, sizeof(myname));

		struct hostent * myhost = gethostbyname(myname);
		if (!myhost)
		{
			return -1;
		}

		address = ((struct in_addr*)(myhost->h_addr_list[0]))->s_addr;
		return 0;
	}
	else if (!strcmp(name, "lo"))
	{
		address = htonl(0x7F000001);
		return 0;
	}

	return -1;
}
#endif

