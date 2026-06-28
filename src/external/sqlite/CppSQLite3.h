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
//			12/07/2007	-Added CppSQLiteDB::IsAutoCommitOn()
//						-Added int64 functions to CppSQLite3Query
//						-Added Name based parameter binding to CppSQLite3Statement.
////////////////////////////////////////////////////////////////////////////////
#ifndef _CppSQLite3_H_
#define _CppSQLite3_H_

#include <sqlite3.h>

#include <string>
#include <string_view>

using CppSQLite3Integer = sqlite3_int64;

constexpr int CPPSQLITE_ERROR = 1000;

class CppSQLite3Exception
{
public:
	CppSQLite3Exception(int nErrCode, const char *szErrMess);
	CppSQLite3Exception(int nErrCode, const std::string &szErrMess);
	
	CppSQLite3Exception(const CppSQLite3Exception &) = default;
	CppSQLite3Exception &operator = (const CppSQLite3Exception &) = default;
	
	int errorCode() const
	{
		return mnErrCode;
	}
	
	std::string errorMessage()
	{
		return mszErrMess;
	}
	
private:
	int mnErrCode;
	std::string mszErrMess;
};

class CppSQLite3Query
{
public:
	CppSQLite3Query();
	CppSQLite3Query(sqlite3 *pDB, sqlite3_stmt *pVM, bool bEof, bool bOwnVM = true);
	
	CppSQLite3Query(const CppSQLite3Query &) = delete;
	CppSQLite3Query &operator = (const CppSQLite3Query &) = delete;
	
	virtual ~CppSQLite3Query();
	
	int numFields();
	
	int fieldDataType(int nCol);
	
	CppSQLite3Integer getIntField(int nField, int nNullValue = 0);
	std::string getStringField(int nField, const std::string &szNullValue = "");

	bool eof();
	
	void nextRow();
	
	void finalize();

private:
	void checkVM();

	sqlite3 *mpDB;
	sqlite3_stmt *mpVM;
	bool mbEof;
	int mnCols;
	bool mbOwnVM;
};

class CppSQLite3Statement
{
public:
	CppSQLite3Statement();
	CppSQLite3Statement(sqlite3 *pDB, sqlite3_stmt *pVM);

	CppSQLite3Statement(CppSQLite3Statement &&);
	CppSQLite3Statement &operator = (CppSQLite3Statement &&);
		
	virtual ~CppSQLite3Statement();
	
	int execDML();
	
	CppSQLite3Query execQuery();
	
	void bind(int nParam, const std::string_view szValue);
	void bind(int nParam, const CppSQLite3Integer nValue);

	void reset();
	
	void finalize();

private:
	void checkDB();
	void checkVM();
	
	sqlite3 *mpDB;
	sqlite3_stmt *mpVM;
};

class CppSQLite3DB
{
public:
	CppSQLite3DB();
	
	virtual ~CppSQLite3DB();
	
	void open(std::string_view szFile);
	
	void close();

	int execDML(std::string_view szSQL);
	
	CppSQLite3Query execQuery(std::string_view szSQL);
	
	CppSQLite3Integer execScalar(std::string_view szSQL, int nNullValue = 0);
	
	CppSQLite3Statement compileStatement(std::string_view szSQL);
	
	CppSQLite3Integer lastRowId();
	
	void interrupt();
	
private:
	CppSQLite3DB(const CppSQLite3DB &db);
	CppSQLite3DB &operator=(const CppSQLite3DB &db);
	
	void setBusyTimeout(int nMillisecs);
	
	sqlite3_stmt *compile(std::string_view szSQL);
	
	void checkDB();
	
	sqlite3 *mpDB;
	int mnBusyTimeoutMs;
};

#endif
