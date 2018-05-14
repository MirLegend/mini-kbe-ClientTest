
#include "KBEvent.h"
#include "KBDebug.h"

std::map<std::string, std::vector<KBEvent::EventObj>> KBEvent::events_;

KBEvent::KBEvent()
{
}

KBEvent::~KBEvent()
{
}

void KBEvent::clear()
{
	clearFiredEvents();
}

void KBEvent::clearFiredEvents()
{

}

bool KBEvent::registerEvent(const std::string& eventName, const std::string& funcName, TFunction func, void* objPtr)
{
	std::map<std::string, std::vector<EventObj>>::iterator it = events_.find(eventName);

	if (it == events_.end())
	{
		events_.insert(std::make_pair(eventName, std::vector<EventObj>()));
	}
	std::vector<EventObj>& eo_array = events_[eventName];
	EventObj eo;
	eo.funcName = funcName;
	eo.method = func;
	eo.objPtr = objPtr;
	eo_array.push_back(eo);
	return true;
}

bool KBEvent::deregister(void* objPtr, const std::string& eventName, const std::string& funcName)
{
	std::map<std::string, std::vector<EventObj>>::iterator it = events_.find(eventName);
	if (it == events_.end())
	{
		return false;
	}
	std::vector<EventObj>& eo_array = it->second;
	if (eo_array.size() == 0)
	{
		return false;
	}
	// 从后往前遍历，以避免中途删除的问题
	for (std::vector<EventObj>::iterator vit = eo_array.begin(); vit != eo_array.end(); /*vit++*/)
	{
		EventObj& item = *vit;
		if (objPtr == item.objPtr && (funcName.length() == 0 || funcName == item.funcName))
		{
			vit = eo_array.erase(vit);
		}
		else 
		{
			vit++;
		}
	}

	return true;
}

bool KBEvent::deregister(void* objPtr)
{
	for (auto& item : events_)
	{
		deregister(objPtr, item.first, "");
	}

	return true;
}

void KBEvent::fire(const std::string& eventName, void* pEventData)
{
	std::map<std::string, std::vector<EventObj>>::iterator it = events_.find(eventName);
	if (it == events_.end())
	{
		return;
	}
	std::vector<EventObj>& eo_array_find = it->second;
	if (eo_array_find.size() == 0)
	{
		return;
	}

	//pEventData->eventName = eventName;

	for (auto& item : eo_array_find)
	{
		item.method(pEventData);
	}
}