// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#pragma warning(disable:4996)
#pragma warning(disable:4819)
#pragma warning(disable:4049)
#pragma warning(disable:4217)
#include <io.h>
#include <time.h> 
//#define FD_SETSIZE 1024
#ifndef WIN32_LEAN_AND_MEAN 
#include <winsock2.h>		// 必须在windows.h之前包含， 否则网络模块编译会出错
#include <mswsock.h> 
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h> 
#include <unordered_map>
#include <functional>
#include <memory>
#define _SCL_SECURE_NO_WARNINGS
#else
// linux include
#include <errno.h>
#include <float.h>
#include <pthread.h>	
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <iconv.h>
#include <langinfo.h>   /* CODESET */
#include <stdint.h>
#include <signal.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h> 
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <tr1/unordered_map>
#include <tr1/functional>
#include <tr1/memory>
#include <linux/types.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/resource.h> 
#include <linux/errqueue.h>
#endif

#include <vector>
#include <map>
#include <algorithm>

#undef min
#define min min
#undef max
#define max max

template <class T>
inline const T & min(const T & a, const T & b)
{
	return b < a ? b : a;
}

template <class T>
inline const T & max(const T & a, const T & b)
{
	return a < b ? b : a;
}

//DECLARE_LOG_CATEGORY_EXTERN(LogKBEngine, Log, All);
#define TCP_PACKET_MAX 1460		//网络MTU最大值
#define KBE_ASSERT _ASSERT
#define KBESOCKET SOCKET
#define INLINE    inline
#define KBETEXT 
#ifndef NULL
#define NULL 0
#endif

typedef signed __int64											int64;
typedef signed __int32											int32;
typedef signed __int16											int16;
typedef signed __int8											int8;
typedef unsigned __int64										uint64;
typedef unsigned __int32										uint32;
typedef unsigned __int16										uint16;
typedef unsigned __int8											uint8;
typedef unsigned __int8											uint8;
typedef unsigned __int8											uint8;

#ifndef _WIN32
typedef int32*													INT_PTR;
typedef uint32*													UINT_PTR;
typedef INT_PTR        											intptr;
typedef UINT_PTR        										uintptr;
#endif

typedef uint16 MessageID;
typedef uint16 MessageLength;
typedef uint32 MessageLengthEx;
typedef int32 ENTITY_ID;
typedef uint32 SPACE_ID;
typedef uint64 DBID;
typedef std::vector<uint8> ByteArray;

const uint32 BROADCAST = 0xFFFFFFFF;
const uint32 LOCALHOST = 0x0100007F;
const uint32 SEND_BUFFER_MAX = TCP_PACKET_MAX;
const uint32 RECV_BUFFER_MAX = TCP_PACKET_MAX;

#define KBE_FLT_MAX FLT_MAX

/** 安全的释放一个指针内存 */
#define KBE_SAFE_RELEASE(i)									\
	if (i)													\
		{													\
			delete i;										\
			i = NULL;										\
		}

/** 安全的释放一个指针数组内存 */
#define KBE_SAFE_RELEASE_ARRAY(i)							\
	if (i)													\
		{													\
			delete[] i;										\
			i = NULL;										\
		}

enum EKCLIENT_TYPE
{
	CLIENT_TYPE_UNKNOWN = 0,
	CLIENT_TYPE_MOBILE = 1,
	CLIENT_TYPE_WIN = 2,
	CLIENT_TYPE_LINUX = 3,
	CLIENT_TYPE_MAC = 4,
	CLIENT_TYPE_BROWSER = 5,
	CLIENT_TYPE_BOTS = 6,
	CLIENT_TYPE_MINI = 7,
};

// 网络消息类别
enum NETWORK_MESSAGE_TYPE
{
	NETWORK_MESSAGE_TYPE_COMPONENT = 0,		// 组件消息
	NETWORK_MESSAGE_TYPE_ENTITY = 1,		// entity消息
};

enum ProtocolType
{
	PROTOCOL_TCP = 0,
	PROTOCOL_UDP = 1,
};

enum EntityDataFlags
{
	ED_FLAG_UNKOWN = 0x00000000,			// 未定义
	ED_FLAG_CELL_PUBLIC = 0x00000001,		// 相关所有cell广播
	ED_FLAG_CELL_PRIVATE = 0x00000002,		// 当前cell
	ED_FLAG_ALL_CLIENTS = 0x00000004,		// cell广播与所有客户端
	ED_FLAG_CELL_PUBLIC_AND_OWN = 0x00000008, // cell广播与自己的客户端
	ED_FLAG_OWN_CLIENT = 0x00000010,		// 当前cell和客户端
	ED_FLAG_BASE_AND_CLIENT = 0x00000020,	// base和客户端
	ED_FLAG_BASE = 0x00000040,				// 当前base
	ED_FLAG_OTHER_CLIENTS = 0x00000080,		// cell广播和其他客户端
};

// 加密额外存储的信息占用字节(长度+填充)
#define ENCRYPTTION_WASTAGE_SIZE			(1 + 7)

#define PACKET_MAX_SIZE						1500
#ifndef PACKET_MAX_SIZE_TCP
#define PACKET_MAX_SIZE_TCP					1460
#endif
#define PACKET_MAX_SIZE_UDP					1472

typedef uint16								PacketLength;				// 最大65535
#define PACKET_LENGTH_SIZE					sizeof(PacketLength)

#define NETWORK_MESSAGE_ID_SIZE				sizeof(MessageID)
#define NETWORK_MESSAGE_LENGTH_SIZE			sizeof(MessageLength)
#define NETWORK_MESSAGE_LENGTH1_SIZE		sizeof(MessageLengthEx)
#define NETWORK_MESSAGE_MAX_SIZE			65535
#define NETWORK_MESSAGE_MAX_SIZE1			4294967295

// 游戏内容可用包大小
#define GAME_PACKET_MAX_SIZE_TCP			PACKET_MAX_SIZE_TCP - NETWORK_MESSAGE_ID_SIZE - \
											NETWORK_MESSAGE_LENGTH_SIZE - ENCRYPTTION_WASTAGE_SIZE

/*
	网络消息类型， 定长或者变长。
	如果需要自定义长度则在NETWORK_INTERFACE_DECLARE_BEGIN中声明时填入长度即可。
*/
#ifndef NETWORK_FIXED_MESSAGE
#define NETWORK_FIXED_MESSAGE 0
#endif

#ifndef NETWORK_VARIABLE_MESSAGE
#define NETWORK_VARIABLE_MESSAGE -1
#endif



extern double getTimeSeconds();


