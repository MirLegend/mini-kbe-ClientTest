require "GameAPP"

function main()
	print("hello lua world!")
	GameApp.InitPb()
	GameApp.connectServer("127.0.0.1", 20018)
end

pcall(main)