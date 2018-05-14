// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "KBECommon.h"
#include "lua.h"

/*
可以理解为插件的入口模块
在这个入口中安装了需要监听的事件(installEvents)，同时初始化KBEngine(initKBEngine)
*/
class KBEngineApp;

class  ClientApp
{
public:	
	// Sets default values for this component's properties
	ClientApp();
public:
	static ClientApp& getSingleton();

	bool LoginServer(const char* useraccount, const char* passwd);

	void OnNetMessage(uint8 mainCmd, uint8 subCmd, void* data, uint32 datasize);

	// Called when the game starts
	virtual void BeginPlay();

	/**
	* Ends gameplay for this component.
	* Called from AActor::EndPlay only if bHasBegunPlay is true
	*/
	virtual void EndPlay();

	/**
	* Handle this component being Uninitialized.
	* Called from AActor::EndPlay only if bHasBeenInitialized is true
	*/
	virtual void UninitializeComponent();

	// Called every frame
	virtual void TickComponent( float DeltaTime);

	lua_State* getLuaState() { return L; }

	std::string ip;
	uint16 port;
	uint16 serverHeartbeatTick;

	lua_State* L;
};
