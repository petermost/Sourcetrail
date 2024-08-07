#include "SqliteStorageMigration.h"

SqliteStorageMigration::~SqliteStorageMigration() = default;

bool SqliteStorageMigration::executeStatementInStorage(
	SqliteStorage* storage, const std::string& statement) 
{
	return storage->executeStatement(statement);
}
