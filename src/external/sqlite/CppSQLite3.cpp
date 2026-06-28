////////////////////////////////////////////////////////////////////////////////
// CppSQLite3 - A C++ wrapper around the SQLite3 embedded database library.
//
// Copyright (c) 2004..2007 Rob Groves. All Rights Reserved. rob.groves@btinternet.com
//
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose, without fee, and without a written
// agreement, is hereby granted, provided that the above copyright notice,
// this paragraph and the following two paragraphs appear in all copies,
// modifications, and distributions.
//
// IN NO EVENT SHALL THE AUTHOR BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
// PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
// EVEN IF THE AUTHOR HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE AUTHOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF
// ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". THE AUTHOR HAS NO OBLIGATION
// TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//
// V3.0		03/08/2004	-Initial Version for sqlite3
//
// V3.1		16/09/2004	-Implemented getXXXXField using sqlite3 functions
//						-Added CppSQLiteDB3::tableExists()
//
// V3.2		01/07/2005	-Fixed execScalar to handle a NULL result
//			12/07/2007	-Added int64 functions to CppSQLite3Query
//						-Throw exception from CppSQLite3DB::close() if error
//						-Trap above exception in CppSQLite3DB::~CppSQLite3DB()
//						-Fix to CppSQLite3DB::compile() as provided by Dave Rollins.
//						-sqlite3_prepare replaced with sqlite3_prepare_v2
//						-Added Name based parameter binding to CppSQLite3Statement.
////////////////////////////////////////////////////////////////////////////////
#include "CppSQLite3.h"

#include <cstdlib>
#include <cstring>

using namespace std;

namespace
{

const char *errorCodeAsString(int nErrCode)
{
	switch (nErrCode)
	{
	case SQLITE_OK:
		return "SQLITE_OK";
	case SQLITE_ERROR:
		return "SQLITE_ERROR";
	case SQLITE_INTERNAL:
		return "SQLITE_INTERNAL";
	case SQLITE_PERM:
		return "SQLITE_PERM";
	case SQLITE_ABORT:
		return "SQLITE_ABORT";
	case SQLITE_BUSY:
		return "SQLITE_BUSY";
	case SQLITE_LOCKED:
		return "SQLITE_LOCKED";
	case SQLITE_NOMEM:
		return "SQLITE_NOMEM";
	case SQLITE_READONLY:
		return "SQLITE_READONLY";
	case SQLITE_INTERRUPT:
		return "SQLITE_INTERRUPT";
	case SQLITE_IOERR:
		return "SQLITE_IOERR";
	case SQLITE_CORRUPT:
		return "SQLITE_CORRUPT";
	case SQLITE_NOTFOUND:
		return "SQLITE_NOTFOUND";
	case SQLITE_FULL:
		return "SQLITE_FULL";
	case SQLITE_CANTOPEN:
		return "SQLITE_CANTOPEN";
	case SQLITE_PROTOCOL:
		return "SQLITE_PROTOCOL";
	case SQLITE_EMPTY:
		return "SQLITE_EMPTY";
	case SQLITE_SCHEMA:
		return "SQLITE_SCHEMA";
	case SQLITE_TOOBIG:
		return "SQLITE_TOOBIG";
	case SQLITE_CONSTRAINT:
		return "SQLITE_CONSTRAINT";
	case SQLITE_MISMATCH:
		return "SQLITE_MISMATCH";
	case SQLITE_MISUSE:
		return "SQLITE_MISUSE";
	case SQLITE_NOLFS:
		return "SQLITE_NOLFS";
	case SQLITE_AUTH:
		return "SQLITE_AUTH";
	case SQLITE_FORMAT:
		return "SQLITE_FORMAT";
	case SQLITE_RANGE:
		return "SQLITE_RANGE";
	case SQLITE_ROW:
		return "SQLITE_ROW";
	case SQLITE_DONE:
		return "SQLITE_DONE";
	case CPPSQLITE_ERROR:
		return "CPPSQLITE_ERROR";
	default:
		return "UNKNOWN_ERROR";
	}
}

inline string makeNonNullString(const char *s)
{
	return s != nullptr ? s : "";
}

} // namespace

CppSQLite3Exception::CppSQLite3Exception(int nErrCode, const char *szErrMess)
	: CppSQLite3Exception(nErrCode, makeNonNullString(szErrMess))
{
}

CppSQLite3Exception::CppSQLite3Exception(int nErrCode, const std::string &szErrMess)
{
	mnErrCode = nErrCode;
	
	char *pszErrMess = sqlite3_mprintf("%s[%d]: %s", errorCodeAsString(nErrCode), nErrCode, szErrMess.c_str());
	mszErrMess = pszErrMess;
	sqlite3_free(pszErrMess);
}

////////////////////////////////////////////////////////////////////////////////

CppSQLite3Query::CppSQLite3Query()
{
	mpVM = nullptr;
	mbEof = true;
	mnCols = 0;
	mbOwnVM = false;
}

CppSQLite3Query::CppSQLite3Query(sqlite3 *pDB, sqlite3_stmt *pVM, bool bEof, bool bOwnVM /*=true*/)
{
	mpDB = pDB;
	mpVM = pVM;
	mbEof = bEof;
	mnCols = sqlite3_column_count(mpVM);
	mbOwnVM = bOwnVM;
}

CppSQLite3Query::~CppSQLite3Query()
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
}

int CppSQLite3Query::numFields()
{
	checkVM();
	return mnCols;
}

CppSQLite3Integer CppSQLite3Query::getIntField(int nField, int nNullValue /*=0*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return nNullValue;
	}
	else
	{
		return sqlite3_column_int64(mpVM, nField);
	}
}

string CppSQLite3Query::getStringField(int nField, const string &szNullValue /*=""*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return szNullValue;
	}
	else
	{
		return string(reinterpret_cast<const char *>(sqlite3_column_text(mpVM, nField)), sqlite3_column_bytes(mpVM, nField));
	}
}

int CppSQLite3Query::fieldDataType(int nCol)
{
	checkVM();

	if (nCol < 0 || nCol > mnCols - 1)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR, "Invalid field index requested");
	}

	return sqlite3_column_type(mpVM, nCol);
}

bool CppSQLite3Query::eof()
{
	checkVM();
	return mbEof;
}

void CppSQLite3Query::nextRow()
{
	checkVM();

	int nRet = sqlite3_step(mpVM);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		mbEof = true;
	}
	else if (nRet == SQLITE_ROW)
	{
		// more rows, nothing to do
	}
	else
	{
		nRet = sqlite3_finalize(mpVM);
		mpVM = nullptr;
		const char *szError = sqlite3_errmsg(mpDB);
		throw CppSQLite3Exception(nRet, szError);
	}
}

void CppSQLite3Query::finalize()
{
	if (mpVM && mbOwnVM)
	{
		int nRet = sqlite3_finalize(mpVM);
		mpVM = nullptr;
		if (nRet != SQLITE_OK)
		{
			const char *szError = sqlite3_errmsg(mpDB);
			throw CppSQLite3Exception(nRet, szError);
		}
	}
}

void CppSQLite3Query::checkVM()
{
	if (mpVM == nullptr)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR, "Null Virtual Machine pointer");
	}
}

////////////////////////////////////////////////////////////////////////////////

CppSQLite3Statement::CppSQLite3Statement()
{
	mpDB = nullptr;
	mpVM = nullptr;
}

CppSQLite3Statement::CppSQLite3Statement(sqlite3 *pDB, sqlite3_stmt *pVM)
{
	mpDB = pDB;
	mpVM = pVM;
}

CppSQLite3Statement::CppSQLite3Statement(CppSQLite3Statement &&rStatement)
{
	mpDB = rStatement.mpDB;
	mpVM = rStatement.mpVM;
	// Only one object can own VM
	rStatement.mpVM = nullptr;
}

CppSQLite3Statement &CppSQLite3Statement::operator = (CppSQLite3Statement &&rStatement)
{
	mpDB = rStatement.mpDB;
	mpVM = rStatement.mpVM;
	// Only one object can own VM
	rStatement.mpVM = nullptr;
	
	return *this;
}

CppSQLite3Statement::~CppSQLite3Statement()
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
}

int CppSQLite3Statement::execDML()
{
	checkDB();
	checkVM();

	const char *szError = nullptr;

	int nRet = sqlite3_step(mpVM);

	if (nRet == SQLITE_DONE)
	{
		int nRowsChanged = sqlite3_changes(mpDB);

		nRet = sqlite3_reset(mpVM);

		if (nRet != SQLITE_OK)
		{
			szError = sqlite3_errmsg(mpDB);
			throw CppSQLite3Exception(nRet, szError);
		}

		return nRowsChanged;
	}
	else
	{
		nRet = sqlite3_reset(mpVM);
		szError = sqlite3_errmsg(mpDB);
		throw CppSQLite3Exception(nRet, szError);
	}
}

CppSQLite3Query CppSQLite3Statement::execQuery()
{
	checkDB();
	checkVM();

	int nRet = sqlite3_step(mpVM);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		return CppSQLite3Query(mpDB, mpVM, true /*eof*/, false);
	}
	else if (nRet == SQLITE_ROW)
	{
		// at least 1 row
		return CppSQLite3Query(mpDB, mpVM, false /*eof*/, false);
	}
	else
	{
		nRet = sqlite3_reset(mpVM);
		const char *szError = sqlite3_errmsg(mpDB);
		throw CppSQLite3Exception(nRet, szError);
	}
}

void CppSQLite3Statement::bind(int nParam, const string_view szValue)
{
	checkVM();
	int nRes = sqlite3_bind_text(mpVM, nParam, szValue.data(), szValue.length(), SQLITE_TRANSIENT);

	if (nRes != SQLITE_OK)
	{
		throw CppSQLite3Exception(nRes, "Error binding string param");
	}
}

void CppSQLite3Statement::bind(int nParam, const CppSQLite3Integer nValue)
{
	checkVM();
	int nRes = sqlite3_bind_int64(mpVM, nParam, nValue);

	if (nRes != SQLITE_OK)
	{
		throw CppSQLite3Exception(nRes, "Error binding int param");
	}
}

void CppSQLite3Statement::reset()
{
	if (mpVM)
	{
		int nRet = sqlite3_reset(mpVM);

		if (nRet != SQLITE_OK)
		{
			const char *szError = sqlite3_errmsg(mpDB);
			throw CppSQLite3Exception(nRet, szError);
		}
	}
}

void CppSQLite3Statement::finalize()
{
	if (mpVM)
	{
		int nRet = sqlite3_finalize(mpVM);
		mpVM = nullptr;

		if (nRet != SQLITE_OK)
		{
			const char *szError = sqlite3_errmsg(mpDB);
			throw CppSQLite3Exception(nRet, szError);
		}
	}
}

void CppSQLite3Statement::checkDB()
{
	if (mpDB == nullptr)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR, "Database not open");
	}
}

void CppSQLite3Statement::checkVM()
{
	if (mpVM == nullptr)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR, "Null Virtual Machine pointer");
	}
}

////////////////////////////////////////////////////////////////////////////////

CppSQLite3DB::CppSQLite3DB()
{
	mpDB = nullptr;
	mnBusyTimeoutMs = 60000; // 60 seconds
}

CppSQLite3DB::CppSQLite3DB(const CppSQLite3DB &db)
{
	mpDB = db.mpDB;
	mnBusyTimeoutMs = 60000; // 60 seconds
}

CppSQLite3DB::~CppSQLite3DB()
{
	try
	{
		close();
	}
	catch (...)
	{
	}
}

CppSQLite3DB &CppSQLite3DB::operator= (const CppSQLite3DB &db)
{
	mpDB = db.mpDB;
	mnBusyTimeoutMs = 60000; // 60 seconds
	return *this;
}

void CppSQLite3DB::open(string_view szFile)
{
	int nRet = sqlite3_open(szFile.data(), &mpDB);

	if (nRet != SQLITE_OK)
	{
		const char *szError = sqlite3_errmsg(mpDB);
		throw CppSQLite3Exception(nRet, szError);
	}

	setBusyTimeout(mnBusyTimeoutMs);
}

void CppSQLite3DB::close()
{
	if (mpDB)
	{
		if (sqlite3_close(mpDB) == SQLITE_OK)
		{
			mpDB = nullptr;
		}
		else
		{
			throw CppSQLite3Exception(CPPSQLITE_ERROR, "Unable to close database");
		}
	}
}

CppSQLite3Statement CppSQLite3DB::compileStatement(string_view szSQL)
{
	checkDB();

	sqlite3_stmt *pVM = compile(szSQL.data());
	return CppSQLite3Statement(mpDB, pVM);
}

int CppSQLite3DB::execDML(string_view szSQL)
{
	checkDB();

	char *szError = nullptr;

	int nRet = sqlite3_exec(mpDB, szSQL.data(), nullptr, nullptr, &szError);
	
	if (nRet == SQLITE_OK)
	{
		return sqlite3_changes(mpDB);
	}
	else
	{
		string szErrMess = makeNonNullString(szError);
		sqlite3_free(szError);
		throw CppSQLite3Exception(nRet, szErrMess);
	}
}

CppSQLite3Query CppSQLite3DB::execQuery(string_view szSQL)
{
	checkDB();

	sqlite3_stmt *pVM = compile(szSQL);

	int nRet = sqlite3_step(pVM);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		return CppSQLite3Query(mpDB, pVM, true /*eof*/);
	}
	else if (nRet == SQLITE_ROW)
	{
		// at least 1 row
		return CppSQLite3Query(mpDB, pVM, false /*eof*/);
	}
	else
	{
		nRet = sqlite3_finalize(pVM);
		const char *szError = sqlite3_errmsg(mpDB);
		throw CppSQLite3Exception(nRet, szError);
	}
}

CppSQLite3Integer CppSQLite3DB::execScalar(string_view szSQL, int nNullValue /*=0*/)
{
	CppSQLite3Query q = execQuery(szSQL);

	if (q.eof() || q.numFields() < 1)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR, "Invalid scalar query");
	}

	return q.getIntField(0, nNullValue);
}

CppSQLite3Integer CppSQLite3DB::lastRowId()
{
	return sqlite3_last_insert_rowid(mpDB);
}

void CppSQLite3DB::interrupt()
{
	sqlite3_interrupt(mpDB);
}

void CppSQLite3DB::setBusyTimeout(int nMillisecs)
{
	mnBusyTimeoutMs = nMillisecs;
	sqlite3_busy_timeout(mpDB, mnBusyTimeoutMs);
}

void CppSQLite3DB::checkDB()
{
	if (!mpDB)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR, "Database not open");
	}
}

sqlite3_stmt *CppSQLite3DB::compile(string_view szSQL)
{
	checkDB();

	const char *szTail = nullptr;
	sqlite3_stmt *pVM;

	int nRet = sqlite3_prepare_v2(mpDB, szSQL.data(), szSQL.length(), &pVM, &szTail);

	if (nRet != SQLITE_OK)
	{
		const char *szError = sqlite3_errmsg(mpDB);
		throw CppSQLite3Exception(nRet, szError);
	}

	return pVM;
}
