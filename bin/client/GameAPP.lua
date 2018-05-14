require "pbc.protobuf"
local c = require "ZiYuServer"

GameApp = GameApp or {}

local serverIp = "127.0.0.1"
local serverPort = 20018

local bConnected = false --是否连接到服务器

--注册pb文件
function GameApp.InitPb()
	local pbFilePath = "./cl.pb"
    print("InitPb file path: "..pbFilePath)
    
    local buffer = read_protobuf_file_c(pbFilePath)
    protobuf.register(buffer) --注:protobuf 是因为在protobuf.lua里面使用module(protobuf)来修改全局名字

end

--function GameApp.setIpPort(ip, port)
--	serverIp = ip;
--	serverPort = port
--	c.set_ip_port(ip, port)
--end

function GameApp.connectServer(ip, port)
	serverIp = ip;
	serverPort = port
	local ret, err = c.connect(ip, port)
	
	if not ret then
		print("connect error!!  can not connect to "..serverIp.." port:"..serverPort)
		return
	end
end

--function GameApp.Login(useraccount, passwd)
--
--	local ret, err = c.login(useraccount, passwd)
--	if not ret then
--	   print("connect error!!  can not connect to "..serverIp.." port:"..serverPort)
--	   return
--	end
	--print("lua =============  onConnected")
	--bConnected = true
--end

local clientversion = 51

function GameApp.Hello(useraccount, passwd)
	
	local stringbuffer = protobuf.encode("client_loginserver.Hello",      
        {         
            version = clientversion,      
            extraData = "helloziyu"     
        })
	local slen = string.len(stringbuffer)
    print("Hello slen ========= "..slen)
	c.send(90, 1, stringbuffer)
	
end


--有待优化 存入registry中
function onConnected()
	print("lua =============  onConnected")
	bConnected = true
	
	--连接后 第一步是hello下
	GameApp.Hello()
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
	
	local result = protobuf.decode("client_loginserver.HelloCB", buffer)
    print("HelloCB result: "..result.result)
    print("HelloCB version: "..result.version)
	print("HelloCB extraData: "..result.extraData)
end

--return GameApp
