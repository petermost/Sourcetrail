#ifndef MESSAGE_SEARCH_AUTOCOMPLETE_H
#define MESSAGE_SEARCH_AUTOCOMPLETE_H

#include "Message.h"
#include "Node.h"
#include "NodeTypeSet.h"
#include "TabIds.h"

class MessageSearchAutocomplete: public Message<MessageSearchAutocomplete>
{
public:
	MessageSearchAutocomplete(const std::string& query, NodeTypeSet acceptedNodeTypes)
		: query(query), acceptedNodeTypes(acceptedNodeTypes)
	{
		setSchedulerId(TabIds::currentTab());
	}

	static const std::string getStaticType()
	{
		return "MessageSearchAutocomplete";
	}

	void print(std::ostream& os) const override
	{
		os << query << "[";
		std::vector<Id> nodeTypeIds = acceptedNodeTypes.getNodeTypeIds();
		for (size_t i = 0; i < nodeTypeIds.size(); i++)
		{
			if (i != 0)
			{
				os << ", ";
			}
			os << to_string(nodeTypeIds[i]);
		}
		os << "]";
	}

	const std::string query;
	const NodeTypeSet acceptedNodeTypes;
};

#endif	  // MESSAGE_SEARCH_AUTOCOMPLETE_H
