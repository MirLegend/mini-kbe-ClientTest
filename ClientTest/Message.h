// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"

class Channel;
class MemoryStream;

/** 一个消息的参数抽象类 */
class  MessageArgs
{
public:
	enum MESSAGE_ARGS_TYPE
	{
		MESSAGE_ARGS_TYPE_VARIABLE = -1,		// 可变参数长度
		MESSAGE_ARGS_TYPE_FIXED = 0				// 固定参数长度
	};

	MessageArgs() :strArgsTypes() {};
	virtual ~MessageArgs() {};
	virtual void createFromStream(MemoryStream& s) = 0;
	virtual void addToStream(MemoryStream& s) = 0;
	virtual int32 dataSize(void) = 0;
	virtual MessageArgs::MESSAGE_ARGS_TYPE type(void) { return MESSAGE_ARGS_TYPE_FIXED; }

	std::vector<std::string> strArgsTypes;
};

/*
消息模块
客户端与服务端交互基于消息通讯， 任何一个行为一条指令都是以一个消息包来描述
*/
class  Message
{
public:
	Message();
	Message(MessageID mid, const std::string& mname, int16 mmsglen);
	virtual ~Message();

public:

	std::string c_str();

	virtual void handle(MemoryStream& s);

public:
	MessageID id;
	std::string name;
	int16 msglen;
};

class  Messages
{
public:
	Messages();
	virtual ~Messages();

	static Messages& getSingleton();

public:
	void clear();
	void bindFixedMessage();


	Message* add(Message* pMessage, MessageID mid, const std::string& mname, int16 mmsglen);
	Message* findClientMessage(MessageID mid);
	Message* findMessage(const std::string& mname);

	void HandleMessage(uint8 mainCmd, uint8 subCmd, void* data, uint32 datasize);

public:
	std::map<MessageID, Message*> loginappMessages;
	std::map<MessageID, Message*> baseappMessages;
	std::map<int, Message*> clientMessages;

	std::map<std::string, Message*> messages;

	std::map<std::string, MessageID> fixedMessageID;
};
