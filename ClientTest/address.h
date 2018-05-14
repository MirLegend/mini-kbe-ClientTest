#ifndef KBE_ADDRESS_H
#define KBE_ADDRESS_H

#include "KBECommon.h"

class Address 
{
public:
	static const Address NONE;
	void onReclaimObject();

	virtual size_t getPoolObjectBytes()
	{
		size_t bytes = sizeof(ip)
		 + sizeof(port);

		return bytes;
	}

	Address();
	Address(uint32 ipArg, uint16 portArg);
	Address(std::string ipArg, uint16 portArg);
	
	virtual ~Address(){}

	uint32	ip;
	uint16	port;

	int writeToString(char * str, int length) const;

	operator char*() const { return this->c_str(); }

	char * c_str() const;
	const char * ipAsString() const;
	bool isNone() const	{ return this->ip == 0; }

	static int string2ip(const char * string, uint32 & address);
	static int ip2string(uint32 address, char * string);

private:
	static char s_stringBuf[2][32];
	static int s_currStringBuf;
	static char * nextStringBuf();
};

inline Address::Address():
ip(0),
port(0)
{
}

inline Address::Address(uint32 ipArg, uint16 portArg) :
	ip(ipArg),
	port(portArg)
{
} 

// 比较操作符重载
inline bool operator==(const Address & a, const Address & b)
{
	return (a.ip == b.ip) && (a.port == b.port);
}

inline bool operator!=(const Address & a, const Address & b)
{
	return (a.ip != b.ip) || (a.port != b.port);
}

inline bool operator<(const Address & a, const Address & b)
{
	return (a.ip < b.ip) || (a.ip == b.ip && (a.port < b.port));
}

#endif // KBE_ADDRESS_H
