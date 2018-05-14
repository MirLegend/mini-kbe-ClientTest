// Fill out your copyright notice in the Description page of Project Settings.

#include "KBEMain.h"
#include "KBEngine.h"
#include "KBEngineArgs.h"
#include "MemoryStream.h"
#include "Bundle.h"
#include "KBDebug.h"
#include "KBEvent.h"

#include "lua_pbc_manual.h"

// Sets default values for this component's properties
ClientApp::ClientApp()
{
	ip = KBETEXT("127.0.0.1");
	port = 20018;
	L = NULL;
	serverHeartbeatTick = 15;
}

bool ClientApp::LoginServer(const char* useraccount, const char* passwd)
{
	std::vector<uint8> asd;
	return KBEngineApp::getSingleton().login(useraccount, passwd, asd);
}

void ClientApp::UninitializeComponent()
{
	
}

void onConnected(void* p)
{
	printf("onConnected to server! \n");
	lua_State* L = ClientApp::getSingleton().getLuaState();

	lua_getglobal(L, "onConnected");
	if (lua_pcall(L, 0, 0, 0) != 0) {
		printf("error %s\n", lua_tostring(L, -1));
		return ;
	}
}

ClientApp& ClientApp::getSingleton()
{
	static ClientApp* pClientApp = NULL;

	if (!pClientApp)
		pClientApp = new ClientApp();

	return *pClientApp;
}

void onConnectedFail(void* p)
{
	printf("onConnectedFail to server! \n");
	lua_State* L = ClientApp::getSingleton().getLuaState();

	lua_getglobal(L, "onConnectedFail");
	if (lua_pcall(L, 0, 0, 0) != 0) {
		printf("error %s\n", lua_tostring(L, -1));
		return;
	}
}

// Called when the game starts
void ClientApp::BeginPlay()
{
	KBEvent::registerEvent("onconnect", "", onConnected, this);
	KBEvent::registerEvent("onconnectfail", "", onConnectedFail, this);

	KBEngineArgs* pArgs = new KBEngineArgs();
	pArgs->ip = ip;
	pArgs->port = port;
	pArgs->clientType = CLIENT_TYPE_WIN;
	pArgs->serverHeartbeatTick = serverHeartbeatTick;
	pArgs->SEND_BUFFER_MAX = SEND_BUFFER_MAX;
	pArgs->RECV_BUFFER_MAX = RECV_BUFFER_MAX;

	if (!KBEngineApp::getSingleton().initialize(pArgs))
		delete pArgs;

	//lua 相关
	L = lua_open();

	/* 载入Lua基本库 */
	luaL_openlibs(L);
	/* 注册函数 */
	register_pbc_module(L);
	/* 运行脚本 */
	//printf("luaL_dostring \n");
	luaL_dostring(L, "require \"hello\" ");
	//luaL_dofile(L, "hello.lua");
}

void ClientApp::EndPlay()
{
	KBEvent::deregister(NULL, "onconnect", NULL);
	KBEvent::deregister(NULL, "onconnectfail", NULL);
	/* 清除Lua */
	lua_close(L);
	L = NULL;
}

// Called every frame
void ClientApp::TickComponent( float DeltaTime)
{
	KBEngineApp::getSingleton().process();
}

void ClientApp::OnNetMessage(uint8 mainCmd, uint8 subCmd, void* data, uint32 datasize)
{
	lua_State* L = ClientApp::getSingleton().getLuaState();

	lua_getglobal(L, "onNetMessage");
	lua_pushnumber(L, mainCmd);
	lua_pushnumber(L, subCmd);
	lua_pushlstring(L, (char *)data, datasize);
	if (lua_pcall(L, 3, 0, 0) != 0) {
		printf("error %s\n", lua_tostring(L, -1));
		return;
	}
}
