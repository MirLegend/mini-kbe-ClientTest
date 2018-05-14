#include <stdlib.h>
#include <sys/stat.h>
#include <string>
#include "pbc-lua.h"

#include "KBEMain.h"
#include "KBEngine.h"

//#include "proto/cl.pb.h"
int read_protobuf_file(lua_State *L) {
	const char *buff = luaL_checkstring(L, -1);

	FILE *fp = fopen(buff, "rb");
	if (!fp)
		return 0;

#if defined(_MSC_VER)
	auto descriptor = _fileno(fp);
#else
	auto descriptor = fileno(fp);
#endif
	struct stat statBuf;
	if (fstat(descriptor, &statBuf) == -1) {
		fclose(fp);
		return 0;
	}
	size_t size = statBuf.st_size;

	std::string buffer;
	buffer.resize(size);
	char* readbuff = new char[size];
	size_t readsize = fread(readbuff, 1, size, fp);
	fclose(fp);

	if (readsize < size) {
		delete readbuff;
		return 0;
	}

	//Data data = cocos2d::FileUtils::getInstance()->getDataFromFile(buff);
	lua_pushlstring(L, (const char*)readbuff, size);
	delete readbuff;
	return 1;
}

static int ziyu_connect(lua_State *L)
{
	uint16 serverPort = luaL_checknumber(L, -1);
	const char* serverIp = luaL_checkstring(L, -2);

	KBEngineApp& app = KBEngineApp::getSingleton();
	if (!app.connectTo(serverIp, serverPort))
	{
		lua_pushnil(L);
		lua_pushstring(L, "connected error!!");
		return 2;
	}

	lua_pushnumber(L, 1);
	return 1;
}


static int ziyu_send(lua_State *L)
{
	size_t msgLength = 0;
	const char *msgBuffer = lua_tolstring(L, -1, &msgLength);
	uint8 subCmd = luaL_checknumber(L, -2);
	uint8 mainCmd = luaL_checknumber(L, -3);
	
	std::vector<uint8> datas(msgLength);
	if (msgLength>0)
	{
		memcpy(&datas[0], msgBuffer, msgLength);
	}

	KBEngineApp::getSingleton().sendMsg(mainCmd, subCmd, datas);
	
	return 0;
}

int
ziyu_server_network(lua_State *L) {
	luaL_Reg reg[] = {
		{ "connect" , ziyu_connect },
		{ "send" , ziyu_send },
		{ NULL,NULL },
	};
	luaL_register(L, "ZiYuServer", reg);
	return 1;
}

int register_pbc_module(lua_State* L)
{
	lua_getglobal(L, "_G");
	if (lua_istable(L, -1))//stack:...,_G,
	{
		lua_register(L, "read_protobuf_file_c", read_protobuf_file);
		luaopen_protobuf_c(L);
		ziyu_server_network(L); //ÍøÂç
	}
	lua_pop(L, 1);
	return 1;
}