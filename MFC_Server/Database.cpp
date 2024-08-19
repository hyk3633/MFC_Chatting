#include "pch.h"
#include "Database.h"
#include <iostream>

using namespace std;

Database::Database()
{
}

Database::~Database()
{
}

bool Database::Initialize()
{
	setlocale(LC_ALL, "korean");
	//std::wcout.imbue(std::locale("korean"));

	SQLRETURN retcode;

	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			{
				return true;
			}
		}
	}

	return false;
}

bool Database::Connect()
{
	SQLRETURN retcode;
	wstring odbc = L"mfc_chatting";

	SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (void*)5, 0);

	// Windows 통합인증 경우
	retcode = SQLConnect(hdbc, (wchar_t*)odbc.c_str(), SQL_NTS, nullptr, 0, nullptr, 0);

	// Allocate statement handle  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
		return true;
	}
	else
	{
		return false;
	}
}

void Database::Close()
{
	SQLCancel(hstmt);
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

bool Database::Login(const wstring& id, const wstring& pw)
{
	return ExcuteQuery(id, pw, EQueryType::LOGIN);
}

bool Database::SignUp(const wstring& id, const wstring& pw)
{
	return ExcuteQuery(id, pw, EQueryType::SIGNUP);
}

bool Database::ExcuteQuery(const wstring& id, const wstring& pw, EQueryType queryType)
{
	// 쿼리문
	wstring query;
	if (queryType == EQueryType::LOGIN)
	{
		query = L"SELECT * FROM MFC_Chatting.dbo.AccountInfo WHERE (id = ? AND pw = ?)";
	}
	else if (queryType == EQueryType::SIGNUP)
	{
		query = L"INSERT INTO MFC_Chatting.dbo.AccountInfo(id, pw) VALUES (?, ?)";
	}
	else
	{
		cout << "[DB Error] : Invalid query type." << endl;
		return false;
	}

	// 쿼리 매개변수
	SQLLEN param1 = SQL_NTS, param2 = SQL_NTS;
	SQLRETURN retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, id.length(), 0, (SQLPOINTER)id.c_str(), 0, &param1);
	retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, pw.length(), 0, (SQLPOINTER)pw.c_str(), 0, &param2);

	// 쿼리 실행 
	retcode = SQLExecDirect(hstmt, (wchar_t*)query.c_str(), SQL_NTS);

	if (queryType == EQueryType::LOGIN)
	{
		if (retcode == SQL_SUCCESS)
		{
			char ret_id[ACCOUNT_CHAR_SIZE], ret_pw[ACCOUNT_CHAR_SIZE];
			ZeroMemory(&ret_id, ACCOUNT_CHAR_SIZE);
			ZeroMemory(&ret_pw, ACCOUNT_CHAR_SIZE);
			SQLLEN slen_id = 0, slen_pw = 0;

			retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, ret_id, ACCOUNT_CHAR_SIZE, &slen_id);
			retcode = SQLBindCol(hstmt, 2, SQL_C_WCHAR, ret_pw, ACCOUNT_CHAR_SIZE, &slen_pw);

			do {
				retcode = SQLFetch(hstmt);
			} while (retcode != SQL_NO_DATA);

			SQLCloseCursor(hstmt);

			if (strlen(ret_id) == 0 || strlen(ret_pw) == 0)
				return false;
			else
				return true;
		}
	}
	else if (queryType == EQueryType::SIGNUP)
	{
		if (retcode != SQL_SUCCESS)
		{
			if (isPrimaryKeyError)
				isPrimaryKeyError = false;
			return false;
		}
		return true;
	}
	return false;
}