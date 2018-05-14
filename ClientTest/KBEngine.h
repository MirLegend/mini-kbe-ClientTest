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
	����KBEngine����ĺ���ģ��
	�������紴�����־û�Э�顢entities�Ĺ����Լ��������ɵ��ýӿڡ�

	һЩ���Բο��ĵط�:
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
		ͨ������id�õ���������
	*/
	std::string serverErr(uint16 id);

	/**
		�������ѭ��������
	*/
	void process();

	/*
		�����˷��������Լ�ͬ����ɫ��Ϣ�������
	*/
	void sendTick();

	/**
		��¼������ˣ������¼���loginapp������(baseapp)����¼���̲������
	*/
	bool login(const std::string& username, const std::string& password, const std::vector<uint8>& datas);
	virtual void onConnectCallback(std::string ip, uint16 port, bool success, int userdata) override;

	/*
		�˺Ŵ������ؽ��
	*/
	void Client_onCreateAccountResult(MemoryStream& stream);

	/*
		�����˺�
	*/
	bool createAccount(const std::string& username, const std::string& password, const std::vector<uint8>& datas);

	/*
		��������, ͨ��loginapp
	*/
	void resetPassword(const std::string& username);
	void onOpenLoginapp_resetpassword();
	void onConnectTo_resetpassword_callback(std::string ip, uint16 port, bool success);
	void Client_onReqAccountResetPasswordCB(uint16 failcode);

	void Client_onReqAccountBindEmailCB(uint16 failcode);

	/*
		���������룬ͨ��baseapp�� ������ҵ�¼���߲���������baseapp��
	*/
	void newPassword(const std::string& old_password, const std::string& new_password);
	void Client_onReqAccountNewPasswordCB(uint16 failcode);

	/*
	�ص�¼������(baseapp)
	һЩ�ƶ���Ӧ�����׵��ߣ�����ʹ�øù��ܿ��ٵ����������˽���ͨ��
	*/
	void reloginBaseapp();
	void onReloginTo_baseapp_callback(std::string ip, uint16 port, bool success);

	/*
		��¼loginappʧ����
	*/
	void Client_onLoginFailed(MemoryStream& stream);

	/*
		��¼loginapp�ɹ���
	*/
	void Client_onLoginSuccessfully(MemoryStream& stream);

	/*
		��¼baseappʧ����
	*/
	void Client_onLoginBaseappFailed(uint16 failedcode);

	/*
		�ص�¼baseappʧ����
	*/
	void Client_onReloginBaseappFailed(uint16 failedcode);

	/*
		��¼baseapp�ɹ���
	*/
	void Client_onReloginBaseappSuccessfully(MemoryStream& stream);

	void hello();
	void sendMsg(uint8 mainCmd, uint8 subCmd, const std::vector<uint8> buffer);
	void Client_onHelloCB(MemoryStream& stream);

	void Client_onVersionNotMatch(MemoryStream& stream);
	void Client_onScriptVersionNotMatch(MemoryStream& stream);

	/*
		��������߳�
	*/
	void Client_onKicked(uint16 failedcode);
	/*
		�����������ص�
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

	// ����˷����baseapp��ַ
	std::string baseappIP_;
	uint16 baseappPort_;

	double lastTickTime_;
	double lastTickCBTime_;
	double lastUpdateToServerTime_;

	std::string currserver_;
};

