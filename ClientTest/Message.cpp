
#include "Message.h"
#include "KBDebug.h"
#include "KBEMain.h"

Message::Message() :
	id(0),
	name(("")),
	msglen(-1)
{
}

Message::Message(MessageID mid, const std::string& mname, int16 mmsglen):
	id(mid),
	name(mname),
	msglen(mmsglen)
{
}

Message::~Message()
{
}

std::string Message::c_str()
{

	return "";// sprintf_s(KBETEXT("%s id:%u, len : %d"), name.c_str(), id, msglen);
}

void Message::handle(MemoryStream& s) 
{
	SCREEN_ERROR_MSG("Message::handle(): interface(%s), handleMethod no implement!",
		c_str());
}

Messages::Messages() :
	loginappMessages(),
	baseappMessages(),
	clientMessages(),
	messages(),
	fixedMessageID()
{
	bindFixedMessage();
}

Messages::~Messages()
{
}
extern Messages g_Messages;
Messages& Messages::getSingleton()
{
	return g_Messages;
}

void Messages::clear()
{
	/* 此处不做清理，否则KBEngine被destroy之后将无法再找回消息
		for (auto& Elem : messages)
			delete Elem.Value;

		messages.Empty();

		loginappMessages.Empty();
		baseappMessages.Empty();
		clientMessages.Empty();

		bindFixedMessage();
	*/

	return;
}

void Messages::bindFixedMessage()
{
	if (messages.size() > 0)
		return;
}

Message* Messages::add(Message* pMessage, MessageID mid, const std::string& mname, int16 mmsglen)
{
	if (findClientMessage(mid))
		return pMessage;

	//if (fixedMessageID.Contains(mname))
	//{
	//	mid = fixedMessageID.FindRef(mname);
	//}

	pMessage->id = mid;
	pMessage->name = mname;
	pMessage->msglen = mmsglen;

	static int tmpID = 0;

	//// 避免ID重复
	//if (mid == 0)
	//{
	//	--tmpID;
	//	clientMessages.Add(tmpID, pMessage);
	//}
	//else
	//{
	//	clientMessages.Add(pMessage->id, pMessage);
	//}

	//messages.Add(pMessage->name, pMessage);
	//
	//INFO_MSG("Message::add(): %s", *pMessage->c_str());
	return pMessage;
}

Message* Messages::findClientMessage(MessageID mid)
{
	std::map<int, Message*>::iterator it = clientMessages.find(mid);
	return it == clientMessages.end() ? NULL : it->second;
}

Message* Messages::findMessage(const std::string& mname)
{
	std::map<std::string, Message*>::iterator it = messages.find(mname);
	return it == messages.end() ? NULL : it->second;
}

void Messages::HandleMessage(uint8 mainCmd, uint8 subCmd, void* data, uint32 datasize)
{
	ClientApp::getSingleton().OnNetMessage(mainCmd, subCmd, data, datasize);
}

