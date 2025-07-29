#include "StorageProvider.h"

#include "logging.h"

int StorageProvider::getStorageCount() const
{
	return static_cast<int>(m_storages.access()->size());
}

void StorageProvider::clear()
{
	m_storages.access()->clear();
}

void StorageProvider::insert(std::shared_ptr<IntermediateStorage> storage)
{
	aidkit::access([&storage](auto &storages)
	{
		const std::size_t storageSize = storage->getSourceLocationCount();

		std::list<std::shared_ptr<IntermediateStorage>>::iterator it;
		for (it = storages.begin(); it != storages.end(); it++)
		{
			if ((*it)->getSourceLocationCount() < storageSize)
			{
				break;
			}
		}
		storages.insert(it, storage);
	}, m_storages);
}

std::shared_ptr<IntermediateStorage> StorageProvider::consumeSecondLargestStorage()
{
	return aidkit::access([](auto &storages)
	{
		std::shared_ptr<IntermediateStorage> ret;

		if (storages.size() > 1)
		{
			std::list<std::shared_ptr<IntermediateStorage>>::iterator it = storages.begin();
			it++;
			ret = *it;
			storages.erase(it);
		}
		return ret;
	}, m_storages);
}

std::shared_ptr<IntermediateStorage> StorageProvider::consumeLargestStorage()
{
	return aidkit::access([](auto &storages)
	{
		std::shared_ptr<IntermediateStorage> ret;

		if (!storages.empty())
		{
			ret = storages.front();
			storages.pop_front();
		}
		return ret;
	}, m_storages);
}

void StorageProvider::logCurrentState() const
{
	aidkit::access([](auto &storages)
	{
		std::string logString = "Storages waiting for injection:";
		for (const std::shared_ptr<IntermediateStorage> &storage : storages)
		{
			logString += " " + std::to_string(storage->getSourceLocationCount()) + ";";
		}
		LOG_INFO(logString);
	}, m_storages);
}
