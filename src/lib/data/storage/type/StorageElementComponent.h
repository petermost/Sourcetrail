#ifndef STORAGE_ELEMENT_COMPONENT_H
#define STORAGE_ELEMENT_COMPONENT_H

#include "ElementComponentKind.h"
#include "Id.h"

#include <string>

struct StorageElementComponent
{
	StorageElementComponent(): elementId(0), type(ELEMENT_COMPONENT_NONE) {}

	StorageElementComponent(Id elementId, ElementComponentKind type, std::string data)
		: elementId(elementId), type(type), data(std::move(data))
	{
	}

	bool operator<(const StorageElementComponent& other) const
	{
		if (elementId != other.elementId)
		{
			return elementId < other.elementId;
		}
		else if (type != other.type)
		{
			return type < other.type;
		}
		else
		{
			return data < other.data;
		}
	}

	Id elementId;
	ElementComponentKind type;
	std::string data;
};

#endif	  // STORAGE_ELEMENT_COMPONENT_H
