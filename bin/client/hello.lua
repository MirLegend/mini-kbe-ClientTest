--require "pbc.protobuf"
require "GameAPP"  -- GameAPP = 

function main1()
	print("hello lua world!")
	local pbFilePath = "./MsgProtocol.pb"
    print("PB file path: "..pbFilePath)
    
    local buffer = read_protobuf_file_c(pbFilePath)
    protobuf.register(buffer) --注:protobuf 是因为在protobuf.lua里面使用module(protobuf)来修改全局名字
    
    local stringbuffer = protobuf.encode("Person",      
        {      
            name = "Alice",      
            id = 12345,      
            phone = {      
                {      
                    number = "87654321"      
                },      
            }      
        })           
    
    
    local slen = string.len(stringbuffer)
    print("slen = "..slen)
    
    local temp = ""
    for i=1, slen do
        temp = temp .. string.format("0x%X, ", string.byte(stringbuffer, i))
    end
    print(temp)
    local result = protobuf.decode("Person", stringbuffer)
    print("result name: "..result.name)
    print("result name: "..result.id)
end

function main()
	print("hello lua world!")
	GameApp.InitPb()
	GameApp.connectServer("127.0.0.1", 20018)
	--GameApp.Login("zcg", "123123")
end

pcall(main)