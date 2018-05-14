// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"

typedef void (*TFunction)(void *);
/*
	事件模块
	KBEngine插件层与Unity3D表现层通过事件来交互
*/
class KBEvent
{
public:
	KBEvent();
	virtual ~KBEvent();
	
public:
		static bool registerEvent(const std::string& eventName, const std::string& funcName, TFunction func, void* objPtr);
		static bool deregister(void* objPtr, const std::string& eventName, const std::string& funcName);
		static bool deregister(void* objPtr);

		static void fire(const std::string& eventName, void* pEventData);

		static void clear();
		static void clearFiredEvents();

		static void processInEvents() {}
		static void processOutEvents() {}

protected:
	struct EventObj
	{
		TFunction method;
		std::string funcName;
		void* objPtr;
	};

	static std::map<std::string, std::vector<EventObj>> events_;
};


// 注册事件
//#define KBENGINE_REGISTER_EVENT(EVENT_NAME, EVENT_FUNC) \
//	KBEvent::registerEvent(EVENT_NAME, #EVENT_FUNC, [this](const UKBEventData* pEventData) {	EVENT_FUNC(pEventData);	}, (void*)this);

// 注册事件，可重写事件函数
#define KBENGINE_REGISTER_EVENT_OVERRIDE_FUNC(EVENT_NAME, FUNC_NAME, EVENT_FUNC) \
	KBEvent::registerEvent(EVENT_NAME, FUNC_NAME, EVENT_FUNC, (void*)this);

// 注销这个对象某个事件
#define KBENGINE_DEREGISTER_EVENT(EVENT_NAME) KBEvent::deregister((void*)this, EVENT_NAME, FUNC_NAME);

// 注销这个对象所有的事件
#define KBENGINE_DEREGISTER_ALL_EVENT()	KBEvent::deregister((void*)this);

// fire event
#define KBENGINE_EVENT_FIRE(EVENT_NAME, EVENT_DATA) KBEvent::fire(EVENT_NAME, EVENT_DATA);

