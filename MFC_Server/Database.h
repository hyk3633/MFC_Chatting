#pragma once
#pragma comment(lib, "odbc32.lib")
#include <Windows.h>
#include <sqlext.h>
#include <string>
#include <vector>

const int ACCOUNT_CHAR_SIZE = 100;

enum class EQueryType
{
	SIGNUP,
	LOGIN,
};

class Database
{
public:

	Database();
	virtual ~Database();

	bool Initialize();

	bool Connect();

	void Close();

	bool Login(const std::wstring& id, const std::wstring& pw);

	bool SignUp(const std::wstring& id, const std::wstring& pw);

	bool ExcuteQuery(const std::wstring& id, const std::wstring& pw, EQueryType queryType);

private:

	SQLHENV		henv;
	SQLHDBC		hdbc;
	SQLHSTMT	hstmt;

	// 쿼리 관련 변수
	bool isPrimaryKeyError;

};