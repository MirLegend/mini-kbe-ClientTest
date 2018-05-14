// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"
#include "Interfaces.h"

class KBEngineArgs;
class Entity;
class NetworkInterface;
class MemoryStream;
class PersistentInfos;

/*
	这是KBEngine插件的核心模块
	包括网络创建、持久化协议、entities的管理、以及引起对外可调用接口。

	一些可以参考的地方:
	http://www.kbengine.org/docs/programming/clientsdkprogramming.html
	http://www.kbengine.org/docs/programming/kbe_message_format.html
*/
class  KBEngineApp : public InterfaceConnect
{
public:
	KBEngineApp();
	KBEngineApp(KBEngineArgs* pArgs);
	virtual ~KBEngineApp();
	
public:
	static KBEngineApp& getSingleton();

public:
	bool isInitialized() const {
		return pArgs_ != NULL;
	}

	bool initialize(KBEngineArgs* pArgs);
	void destroy();
	void reset();

	NetworkInterface* pNetworkInterface() const {
		return pNetworkInterface_;
	}

	void _closeNetwork();

	KBEngineArgs* getInitArgs() const
	{
		return pArgs_;
	}

	void resetMessages();

	/*
		通过错误id得到错误描述
	*/
	std::string serverErr(uint16 id);

	/**
		插件的主循环处理函数
	*/
	void process();

	/*
		向服务端发送心跳以及同步角色信息到服务端
	*/
	void sendTick();

	/**
		登录到服务端，必须登录完成loginapp与网关(baseapp)，登录流程才算完毕
	*/
	bool login(const std::string& username, const std::string& password, const std::vector<uint8>& datas);
	virtual void onConnectCallback(std::string ip, uint16 port, bool success, int userdata) override;

	/*
		账号创建返回结果
	*/
	void Client_onCreateAccountResult(MemoryStream& stream);

	/*
		创建账号
	*/
	bool createAccount(const std::string& username, const std::string& password, const std::vector<uint8>& datas);

	/*
		重置密码, 通过loginapp
	*/
	void resetPassword(const std::string& username);
	void onOpenLoginapp_resetpassword();
	void onConnectTo_resetpassword_callback(std::string ip, uint16 port, bool success);
	void Client_onReqAccountResetPasswordCB(uint16 failcode);

	void Client_onReqAccountBindEmailCB(uint16 failcode);

	/*
		设置新密码，通过baseapp， 必须玩家登录在线操作所以是baseapp。
	*/
	void newPassword(const std::string& old_password, const std::string& new_password);
	void Client_onReqAccountNewPasswordCB(uint16 failcode);

	/*
	重登录到网关(baseapp)
	一些移动类应用容易掉线，可以使用该功能快速的重新与服务端建立通信
	*/
	void reloginBaseapp();
	void onReloginTo_baseapp_callback(std::string ip, uint16 port, bool success);

	/*
		登录loginapp失败了
	*/
	void Client_onLoginFailed(MemoryStream& stream);

	/*
		登录loginapp成功了
	*/
	void Client_onLoginSuccessfully(MemoryStream& stream);

	/*
		登录baseapp失败了
	*/
	void Client_onLoginBaseappFailed(uint16 failedcode);

	/*
		重登录baseapp失败了
	*/
	void Client_onReloginBaseappFailed(uint16 failedcode);

	/*
		登录baseapp成功了
	*/
	void Client_onReloginBaseappSuccessfully(MemoryStream& stream);

	void hello();
	void sendMsg(uint8 mainCmd, uint8 subCmd, const std::vector<uint8> buffer);
	void Client_onHelloCB(MemoryStream& stream);

	void Client_onVersionNotMatch(MemoryStream& stream);
	void Client_onScriptVersionNotMatch(MemoryStream& stream);

	/*
		被服务端踢出
	*/
	void Client_onKicked(uint16 failedcode);
	/*
		服务器心跳回调
	*/
	void Client_onAppActiveTickCB();


	//////////////////////////////////////////////////////////
	bool connectTo(const char* ip, uint16 port);

private:

	bool initNetwork();

	bool login_loginapp(bool noconnect);
	void onConnectTo_loginapp_login_callback(std::string ip, uint16 port, bool success);
	void onLogin_loginapp();

	void login_baseapp(bool noconnect);
	void onConnectTo_baseapp_callback(std::string ip, uint16 port, bool success);
	void onLogin_baseapp();

	void resetpassword_loginapp(bool noconnect);

	void createAccount_loginapp(bool noconnect);
	void onOpenLoginapp_createAccount();
	void onConnectTo_loginapp_create_callback(std::string ip, uint16 port, bool success);

public:

protected:
	KBEngineArgs* pArgs_;
	NetworkInterface* pNetworkInterface_;

	std::string username_;
	std::string password_;

	// 服务端分配的baseapp地址
	std::string baseappIP_;
	uint16 baseappPort_;

	double lastTickTime_;
	double lastTickCBTime_;
	double lastUpdateToServerTime_;

	std::string currserver_;
};

