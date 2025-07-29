#ifndef STORAGE_PROVIDER_H
#define STORAGE_PROVIDER_H

#include "IntermediateStorage.h"

#include <aidkit/thread_shared.hpp>

#include <list>
#include <memory>

class StorageProvider
{
public:
	int getStorageCount() const;

	void clear();

	void insert(std::shared_ptr<IntermediateStorage> storage);

	// returns empty shared_ptr if no storages available
	std::shared_ptr<IntermediateStorage> consumeSecondLargestStorage();

	// returns empty shared_ptr if no storages available
	std::shared_ptr<IntermediateStorage> consumeLargestStorage();

	void logCurrentState() const;

private:
	aidkit::thread_shared<std::list<std::shared_ptr<IntermediateStorage>>> m_storages; // larger storages are in front
};

#endif	  // STORAGE_PROVIDER_H
