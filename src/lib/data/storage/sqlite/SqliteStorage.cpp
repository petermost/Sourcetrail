#include "SqliteStorage.h"

#include "FileSystem.h"
#include "TimeStamp.h"
#include "logging.h"

SqliteStorage::SqliteStorage(const FilePath& dbFilePath): m_dbFilePath(dbFilePath.getCanonical())
{
	if (!m_dbFilePath.getParentDirectory().empty() && !m_dbFilePath.getParentDirectory().exists())
	{
		FileSystem::createDirectories(m_dbFilePath.getParentDirectory());
	}

	try
	{
		m_database.open(m_dbFilePath.str());
	}
	catch (CppSQLite3Exception& e)
	{
		LOG_ERROR("Failed to load database file \"" + m_dbFilePath.str() + "\" with message: " + e.errorMessage());
		throw;
	}

	enablePragmas();
}

SqliteStorage::~SqliteStorage()
{
	try
	{
		m_database.close();
	}
	catch (CppSQLite3Exception &e)
	{
		LOG_ERROR(e.errorMessage());
	}
}

void SqliteStorage::setup()
{
	enablePragmas();

	setupMetaTable();

	if (isEmpty() || !isIncompatible())
	{
		setupTables();

		if (!m_precompiledStatementsInitialized)
		{
			setupPrecompiledStatements();
			m_precompiledStatementsInitialized = true;
		}
	}
}

void SqliteStorage::clear()
{
	disablePragmas();

	clearMetaTable();
	clearTables();

	setup();
}

size_t SqliteStorage::getVersion() const
{
	std::string storageVersionStr = getMetaValue("storage_version");

	if (!storageVersionStr.empty())
	{
		return std::stoi(storageVersionStr);
	}

	return 0;
}

void SqliteStorage::setVersion(size_t version)
{
	insertOrUpdateMetaValue("storage_version", std::to_string(version));
}

void SqliteStorage::beginTransaction()
{
	executeStatement("BEGIN TRANSACTION;");
}

void SqliteStorage::commitTransaction()
{
	executeStatement("COMMIT TRANSACTION;");
}

void SqliteStorage::rollbackTransaction()
{
	executeStatement("ROLLBACK TRANSACTION;");
}

void SqliteStorage::optimizeMemory() const
{
	executeStatement("VACUUM;");
}

FilePath SqliteStorage::getDbFilePath() const
{
	return m_dbFilePath;
}

bool SqliteStorage::isEmpty() const
{
	return getVersion() <= 0;
}

bool SqliteStorage::isIncompatible() const
{
	size_t storageVersion = getVersion();
	return isEmpty() || storageVersion != getStaticVersion();
}

void SqliteStorage::setTime()
{
	insertOrUpdateMetaValue("timestamp", TimeStamp::now().toString());
}

TimeStamp SqliteStorage::getTime() const
{
	return TimeStamp(getMetaValue("timestamp"));
}

void SqliteStorage::setupMetaTable()
{
	try
	{
		m_database.execDML(
			"CREATE TABLE IF NOT EXISTS meta("
			"id INTEGER, "
			"key TEXT, "
			"value TEXT, "
			"PRIMARY KEY(id)"
			");");
	}
	catch (CppSQLite3Exception& e)
	{
		LOG_ERROR(std::to_string(e.errorCode()) + ": " + e.errorMessage());

		throw(std::exception());
	}
}

void SqliteStorage::clearMetaTable()
{
	try
	{
		m_database.execDML("DROP TABLE IF EXISTS main.meta;");
	}
	catch (CppSQLite3Exception& e)
	{
		LOG_ERROR(std::to_string(e.errorCode()) + ": " + e.errorMessage());
	}
}

bool SqliteStorage::executeStatement(const std::string& statement) const
{
	try
	{
		m_database.execDML(statement);
	}
	catch (CppSQLite3Exception &e)
	{
		LOG_ERROR(std::to_string(e.errorCode()) + ": " + e.errorMessage());
		return false;
	}
	return true;
}

bool SqliteStorage::executeStatement(CppSQLite3Statement& statement) 
{
	try
	{
		statement.execDML();
	}
	catch (CppSQLite3Exception &e)
	{
		LOG_ERROR(std::to_string(e.errorCode()) + ": " + e.errorMessage());
		return false;
	}

	statement.reset();
	return true;
}

long long SqliteStorage::executeStatementScalar(const std::string& statement, const int nullValue) const
{
	long long ret = 0;
	try
	{
		ret = m_database.execScalar(statement, nullValue);
	}
	catch (CppSQLite3Exception &e)
	{
		LOG_ERROR(std::to_string(e.errorCode()) + ": " + e.errorMessage());
	}
	return ret;
}

long long SqliteStorage::executeStatementScalar(CppSQLite3Statement& statement, const int nullValue) 
{
	long long ret = 0;
	try
	{
		CppSQLite3Query q = executeQuery(statement);

		if (q.eof() || q.numFields() < 1)
		{
			throw CppSQLite3Exception(CPPSQLITE_ERROR, "Invalid scalar query");
		}

		ret = q.getIntField(0, nullValue);
	}
	catch (CppSQLite3Exception &e)
	{
		LOG_ERROR(std::to_string(e.errorCode()) + ": " + e.errorMessage());
	}

	return ret;
}

CppSQLite3Query SqliteStorage::executeQuery(const std::string& statement) const
{
	try
	{
		return m_database.execQuery(statement);
	}
	catch (CppSQLite3Exception &e)
	{
		LOG_ERROR(std::to_string(e.errorCode()) + ": " + e.errorMessage());
	}
	return CppSQLite3Query();
}

CppSQLite3Query SqliteStorage::executeQuery(CppSQLite3Statement& statement) 
{
	try
	{
		return statement.execQuery();
	}
	catch (CppSQLite3Exception &e)
	{
		LOG_ERROR(std::to_string(e.errorCode()) + ": " + e.errorMessage());
	}
	return CppSQLite3Query();
}

bool SqliteStorage::hasTable(const std::string& tableName) const
{
	CppSQLite3Query q = executeQuery("SELECT name FROM sqlite_master WHERE type='table' AND name='" + tableName + "';");

	if (!q.eof())
	{
		return q.getStringField(0, "") == tableName;
	}

	return false;
}

std::string SqliteStorage::getMetaValue(const std::string& key) const
{
	if (hasTable("meta"))
	{
		CppSQLite3Query q = executeQuery("SELECT value FROM meta WHERE key = '" + key + "';");

		if (!q.eof())
		{
			return q.getStringField(0, "");
		}
	}

	return "";
}

void SqliteStorage::insertOrUpdateMetaValue(const std::string& key, const std::string& value)
{
	CppSQLite3Statement stmt = m_database.compileStatement("INSERT OR REPLACE INTO meta(id, key, value) VALUES((SELECT id FROM meta WHERE key = ?), ?, ?);");

	stmt.bind(1, key);
	stmt.bind(2, key);
	stmt.bind(3, value);
	executeStatement(stmt);
}

void SqliteStorage::enablePragmas() const
{
	executeStatement("PRAGMA FOREIGN_KEYS=ON;");

	executeStatement("PRAGMA SYNCHRONOUS=OFF;");
}

void SqliteStorage::disablePragmas() const
{
	executeStatement("PRAGMA FOREIGN_KEYS=OFF;");
}
