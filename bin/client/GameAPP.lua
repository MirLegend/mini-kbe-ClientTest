require "pbc.protobuf"
local c = require "ZiYuServer"

GameApp = GameApp or {}

local serverIp = "127.0.0.1"
local serverPort = 20018

local serverBaseIp = "127.0.0.1"
local serverBasePort = 20018

local g_accountName = ""
local g_password = ""

local curserver = 1 --login

local bConnected = false --是否连接到服务器

--注册pb文件
function GameApp.InitPb()
	local pbFilePath = "./cl.pb"
    print("InitPb file path: "..pbFilePath)
    
    local buffer = read_protobuf_file_c(pbFilePath)
    protobuf.register(buffer) --注:protobuf 是因为在protobuf.lua里面使用module(protobuf)来修改全局名字

	local pbbaseFilePath = "./cb.pb"
    print("InitPb file path: "..pbbaseFilePath)
    
    buffer = read_protobuf_file_c(pbbaseFilePath)
    protobuf.register(buffer) --注:protobuf 是因为在protobuf.lua里面使用module(protobuf)来修改全局名字
end

function GameApp.loginServer(account, pwd)
	g_accountName = account;
	g_password = pwd
	curserver = 1
	GameApp.connectServer(serverIp, serverPort)
end

function GameApp.connectServer(ip, port)
	serverIp = ip;
	serverPort = port
	local ret, err = c.connect(ip, port)
	
	if not ret then
		print("connect error!!  can not connect to "..serverIp.." port:"..serverPort)
		return
	end
end

local clientversion = 51

function GameApp.Hello()
	
	local stringbuffer = protobuf.encode("client_loginserver.Hello",      
        {         
            version = clientversion,      
            extraData = "helloziyu"     
        })
	local slen = string.len(stringbuffer)
    print("Hello slen ========= "..slen)
	c.send(90, 1, stringbuffer)
	
end

function GameApp.HelloBase()
	
	local stringbuffer = protobuf.encode("client_baseserver.Hello",      
        {         
            version = clientversion,      
            extraData = "helloziyu"     
        })
	local slen = string.len(stringbuffer)
    print("Hello base slen ========= "..slen)
	c.send(91, 1, stringbuffer)
	
end

function GameApp.Login(loginName, password)
	
	local stringbuffer = protobuf.encode("client_loginserver.Login",      
        {         
            ctype = 1,      
            account = loginName,
			password = password,
			extraData = "helloziyu"
        })
	local slen = string.len(stringbuffer)
    print("Login slen ========= "..slen)
	c.send(90, 5, stringbuffer)
	
end

function GameApp.LoginBase(loginName, password)
	
	local stringbuffer = protobuf.encode("client_baseserver.Login",      
        {            
            account = loginName,
			password = password
        })
	local slen = string.len(stringbuffer)
    print("LoginBase slen ========= "..slen)
	c.send(91, 3, stringbuffer)
	
end


--有待优化 存入registry中
function onConnected()
	print("lua =============  onConnected curser: "..curserver)
	bConnected = true
	
	--连接后 第一步是hello下
	if curserver == 1 then
		GameApp.Hello()
	elseif curserver == 2 then
		GameApp.HelloBase()
	end
end

--有待优化 存入registry中
function onConnectedFail()
	print("lua =============  onConnectedFail")
	bConnected = false
end

--有待优化 存入registry中
function onConnectedClosed()
	print("lua =============  onConnectedClosed")
	bConnected = false
end

--有待优化 存入registry中
function onNetMessage(mainCmd, subCmd, buffer)
	print("onNetMessage ============================================= cmd:"..mainCmd.." subCmd:"..subCmd)
	
	if mainCmd == 90 then --login cmd
		if subCmd == 2 then
			local result = protobuf.decode("client_loginserver.HelloCB", buffer)
			print("HelloCB result: "..result.result)
			print("HelloCB version: "..result.version)
			print("HelloCB extraData: "..result.extraData)
			GameApp.Login(g_accountName, g_password)
		elseif subCmd == 6 then
			local result = protobuf.decode("client_loginserver.LoginFailed", buffer)
			print("LoginFailed failedcode: "..result.failedcode.." datas:"..result.extraData)
		elseif subCmd == 7 then
			local result = protobuf.decode("client_loginserver.LoginSuccessfully", buffer)
			print("LoginSuccessfully ip: "..result.baseIp)
			print("LoginSuccessfully port: "..result.basePort)
			serverBaseIp = result.baseIp
			serverBasePort = result.basePort
			--连接到base
			curserver = 2
			GameApp.connectServer(serverBaseIp, serverBasePort)
		end
	elseif mainCmd == 91 then --base cmd
		if subCmd == 2 then
			local result = protobuf.decode("client_baseserver.HelloCB", buffer)
			print("HelloCB base result: "..result.result)
			print("HelloCB base version: "..result.version)
			print("HelloCB base extraData: "..result.extraData)
			GameApp.LoginBase(g_accountName, g_password)
		elseif subCmd == 4 then  --登陆失败
			local result = protobuf.decode("client_baseserver.LoginBaseappFailed", buffer)
			print("LoginBaseappFailed failedcode: "..result.retCode)
		end
	end
	
	
end

--return GameApp
