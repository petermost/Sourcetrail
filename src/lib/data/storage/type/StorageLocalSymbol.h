#ifndef STORAGE_LOCAL_SYMBOL_H
#define STORAGE_LOCAL_SYMBOL_H

#include <string>

#include "types.h"

struct StorageLocalSymbolData
{
	StorageLocalSymbolData() = default;

	StorageLocalSymbolData(std::string name): name(std::move(name)) {}

	bool operator<(const StorageLocalSymbolData& other) const
	{
		return name < other.name;
	}

	std::string name;
};

struct StorageLocalSymbol: public StorageLocalSymbolData
{
	StorageLocalSymbol():  id(0) {}

	StorageLocalSymbol(Id id, const StorageLocalSymbolData& data)
		: StorageLocalSymbolData(data), id(id)
	{
	}

	StorageLocalSymbol(Id id, std::string name): StorageLocalSymbolData(std::move(name)), id(id) {}

	Id id;
};

#endif	  // STORAGE_LOCAL_SYMBOL_H
