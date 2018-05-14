#ifndef KBE_ENDPOINT_H
#define KBE_ENDPOINT_H
#include "KBECommon.h"
#include "address.h"

class Bundle;
class EndPoint
{
public:
	void onReclaimObject();

	virtual size_t getPoolObjectBytes()
	{
		size_t bytes = sizeof(KBESOCKET)
		 + address_.getPoolObjectBytes();

		return bytes;
	}

	EndPoint(Address address);
	EndPoint(/*uint32 networkAddr = 0, uint16 networkPort = 0*/);
	virtual ~EndPoint();

	operator KBESOCKET() const;
	
	static void initNetwork();
	bool good() const;
		
	void socket(int type);
	KBESOCKET socket() const;
	
	INLINE void setFileDescriptor(int fd);

	INLINE int joinMulticastGroup(uint32 networkAddr);
	INLINE int quitMulticastGroup(uint32 networkAddr);
	
	INLINE int close();
	
	INLINE int setnonblocking(bool nonblocking);
	INLINE int setbroadcast(bool broadcast);
	INLINE int setreuseaddr(bool reuseaddr);
	INLINE int setkeepalive(bool keepalive);
	INLINE int setnodelay(bool nodelay = true);
	INLINE int setlinger(uint16 onoff, uint16 linger);

	INLINE int bind(uint16 networkPort = 0, uint32 networkAddr = INADDR_ANY);

	int listen(int backlog = 5);

	int connect(uint16 networkPort, uint32 networkAddr = INADDR_BROADCAST, bool autosetflags = true);
	int connect(bool autosetflags = true);

	EndPoint* accept(uint16 * networkPort = NULL, uint32 * networkAddr = NULL, bool autosetflags = true);
	
	 int send(const void * gramData, int gramSize);
	void send(Bundle * pBundle);
	void sendto(Bundle * pBundle, uint16 networkPort, uint32 networkAddr = BROADCAST);

	int recv(void * gramData, int gramSize);
	bool recvAll(void * gramData, int gramSize);
	
	INLINE uint32 getRTT();

	int getInterfaceFlags(char * name, int & flags);
	int getInterfaceAddress(const char * name, uint32 & address);
	int getInterfaceNetmask(const char * name, uint32 & netmask);
	bool getInterfaces(std::map< uint32, std::string > &interfaces);

	int findIndicatedInterface(const char * spec, uint32 & address);
	int findDefaultInterface(char * name);

	int getInterfaceAddressByName(const char * name, uint32 & address);
	int getInterfaceAddressByMAC(const char * mac, uint32 & address);
	int getDefaultInterfaceAddress(uint32 & address);

	int getBufferSize(int optname) const;
	bool setBufferSize(int optname, int size);
	
	int getlocaladdress(uint16 * networkPort, uint32 * networkAddr) const;
	int getremoteaddress(uint16 * networkPort, uint32 * networkAddr) const;
	
	Address getLocalAddress() const;
	Address getRemoteAddress() const;

	bool getClosedPort(Address & closedPort);

	const char * c_str() const;
	int getremotehostname(std::string * name) const;
	
	int sendto(void * gramData, int gramSize, uint16 networkPort, uint32 networkAddr = BROADCAST);
	INLINE int sendto(void * gramData, int gramSize, struct sockaddr_in & sin);
	INLINE int recvfrom(void * gramData, int gramSize, uint16 * networkPort, uint32 * networkAddr);
	INLINE int recvfrom(void * gramData, int gramSize, struct sockaddr_in & sin);
	
	INLINE const Address& addr() const;
	void addr(const Address& newAddress);
	void addr(uint16 newNetworkPort, uint32 newNetworkAddress);

	bool waitSend();

protected:
	KBESOCKET	socket_;
	Address address_;
};
#endif // KBE_ENDPOINT_H
