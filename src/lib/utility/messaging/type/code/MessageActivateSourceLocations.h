#ifndef MESSAGE_ACTIVATE_SOURCE_LOCATIONS_H
#define MESSAGE_ACTIVATE_SOURCE_LOCATIONS_H

#include "Message.h"
#include "TabIds.h"
#include "types.h"

class MessageActivateSourceLocations: public Message<MessageActivateSourceLocations>
{
public:
	MessageActivateSourceLocations(const std::vector<Id>& locationIds, bool containsUnsolvedLocations)
		: locationIds(locationIds), containsUnsolvedLocations(containsUnsolvedLocations)
	{
		setSchedulerId(TabIds::currentTab());
	}

	static const std::string getStaticType()
	{
		return "MessageActivateSourceLocations";
	}

	void print(std::ostream& os) const override
	{
		for (const Id& id: locationIds)
		{
			os << id << " ";
		}
	}

	const std::vector<Id> locationIds;
	const bool containsUnsolvedLocations;
};

#endif	  // MESSAGE_ACTIVATE_SOURCE_LOCATIONS_H
