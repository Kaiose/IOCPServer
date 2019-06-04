#include "stdafx.h"
#include "Database.h"


Database::Database()
{
	
	ODBC_NAME	= (SQLWCHAR*)L"mysql_game";
	ODBC_ID = (SQLWCHAR*)L"root";
	ODBC_PW = (SQLWCHAR*)L"bitnami";


	DBConnect();
	AttributeBind();

}


Database::~Database()
{
	DBDisConnect();
}

bool Database::DBConnect() {
	
	// 환경 핸들을 할당한다.
	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS) {
		Log("SQL Handel Allocate Failed \n");
		return false;
	}

	// 현재 환경에 대한 속성을 설정한다.
	if (SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER) != SQL_SUCCESS) {
		Log("SQL Set Env Attr Failed \n");
		return false;
	}

	// 연결핸들을 설정한다.
	if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) != SQL_SUCCESS) {
		Log("SQL Handel Allocate Failed \n");
		return false;
	}


	// 데이터에 연결한다.
	if (SQLConnect(hDbc, ODBC_NAME, SQL_NTS, ODBC_ID, SQL_NTS, ODBC_PW, SQL_NTS) != SQL_SUCCESS) {
		Log("SQL Handle Connect Failed \n");
		return false;
	}
	// 명령문핸들을 할당한다.
	if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS) {
		Log("SQL Handel Allocate Failed \n");
		return false;
	}

	return true;
}

bool Database::DBDisConnect() {

	if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	if (hDbc) SQLDisconnect(hDbc);
	if (hDbc) SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	if (hEnv) SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
	return true;
}

void Database::AttributeBind() {
	SQLBindCol(hStmt, 1, SQL_INTEGER, &id, sizeof(id), &idLen);
	SQLBindCol(hStmt, 2, SQL_CHAR, name, sizeof(CHAR) * 20, &nameLen);
	SQLBindCol(hStmt, 3, SQL_CHAR, password, sizeof(CHAR) * 20, &passwordLen);

}
/**
	SQL문이 만들어지고 인자들이 바인드되면
	명령문(statement)는 SQLExecDirect로 실행된다.
	만약 여러번 수행되는 구문은 SQLPrepare()로 준비하고,
	SQLExecute로 사용해야하는 것이 성능면에 유리하다.

	만약

	SQL문이 파라미터 마커들을 포함하면, 애플리케이션은
	SQLBindParameter() 를 호출하여 각 인자들을 애플리케이션 변수에
	Bind 시킨다.

	인자를 Bind 하기전에SQLPrepare()를 실행해야하며,
	바인딩 후에 SQLExecute()를 실행할 수 있다.

	만약 실행된 명령문(statement) 가 SELECT 문이라면 
	집합 calum Count를 알아보기 위해 SQLNumResultCols()를 호출한다.

	그 다음 SQLDescribeCol()로 각 결과 집합열의 데이터를 가져온다.


	그 후 애플리케이션 변수와 결과 집합의 열을 바인드하는
	SQLBindCol()에 이런 정보를 보낸다.
	(내 생각에는 db의 Attribute 를 알고 있기 때문에 
	가져온 집합의 attribute를 분석하기 보다는 그냥 bind로 묶으면됨)


	이제 SQLFetch() 를 호출하여 첫번째 행 데이터를 가져오고
	SQLBindCol() 로 바인드 된 변수에 데이터를 저장한다.
	행에 긴 데이터가 있으면 SQLGetData()를 할 수 있고 계속 호출할 수 도 있다.

	data를 fetch를 통하여 가져온 이후에는 다른 Statement를
	SQLPrepare() 과 SQLExecute()를 통해 실행하기전에

	반드시 SQLCloseCurser()를 호출하여 커서를 닫아줘야함.


	만일 전에 수행된 Statement 가 INSERT, UPDATE, DELETE 였다면
	SQLRowCount() 로 영향을 받은 행들의 갯수를 가져온다.

	
	*/



bool Database::Insert(int id , const WCHAR* name, const WCHAR* password) {
	WCHAR InsertCommand[100] = { 0, };
	wsprintf(InsertCommand, L"INSERT INTO Account (id, name, password) VALUES (%d, '%s', '%s');", id, name, password);
	printf("%ws\n", InsertCommand);
	int ret = SQLExecDirect(hStmt, (SQLWCHAR*)InsertCommand, SQL_NTS);
	if (ret != SQL_SUCCESS) {
		Log("InsertFailed\n");
		if (hStmt) SQLCloseCursor(hStmt);
		return false;
	}

	if (hStmt) SQLCloseCursor(hStmt);

	return true;
}

bool Database::Insert(const WCHAR* name, const WCHAR* password) {
	WCHAR InsertCommand[100] = { 0, };
	wsprintf(InsertCommand, L"INSERT INTO Account (name, password) VALUES ('%s', '%s');", name, password);
	printf("%ws\n", InsertCommand);
	int ret = SQLExecDirect(hStmt, (SQLWCHAR*)InsertCommand, SQL_NTS);
	if (ret != SQL_SUCCESS) {
		Log("InsertFailed\n");
		if (hStmt) SQLCloseCursor(hStmt);
		return false;
	}

	if (hStmt) SQLCloseCursor(hStmt);

	return true;
}

bool Database::Insert(const CHAR* name, const CHAR* password) {
	WCHAR InsertCommand[100] = { 0, };
	wsprintf(InsertCommand, L"INSERT INTO Account (name, password) VALUES ('%s', '%s');", name, password);
	printf("%ws\n", InsertCommand);
	int ret = SQLExecDirect(hStmt, (SQLWCHAR*)InsertCommand, SQL_NTS);
	if (ret != SQL_SUCCESS) {
		Log("InsertFailed\n");
		if (hStmt) SQLCloseCursor(hStmt);
		return false;
	}

	if (hStmt) SQLCloseCursor(hStmt);

	return true;
}



bool Database::Delete(int id) {
	WCHAR DeleteCommand[100] = { 0, };
	wsprintf(DeleteCommand, L"DELETE FROM Account WHERE id=%d",id);
	printf("%ws\n", DeleteCommand);
	int ret = SQLExecDirect(hStmt, (SQLWCHAR*)DeleteCommand, SQL_NTS);
	if (ret != SQL_SUCCESS) {
		Log("DeleteFailed\n");
		if (hStmt) SQLCloseCursor(hStmt);
		return false;
	}

	if (hStmt) SQLCloseCursor(hStmt);
	return true;
}

int Database::Match(const char* name, const char* password) {
	
	WCHAR MatchCommand[100] = { 0, };
	wsprintf(MatchCommand, L"Select * FROM Account");
	printf("%ws\n", MatchCommand);
	int ret = SQLExecDirect(hStmt, (SQLWCHAR*)MatchCommand, SQL_NTS);
	if (ret != SQL_SUCCESS) {
		Log("MatchFailed\n");
		if (hStmt) SQLCloseCursor(hStmt);
		return 0;
	}

	printf("User name : %s\t Password : %s\n",  name, password);
	
	while (SQLFetch(hStmt) != SQL_NO_DATA) {
		printf("%d\t%s\t%s\n", id, this->name, this->password);
		if (strcmp(this->name, name) == 0 && strcmp(this->password, password) == 0) {
			if (hStmt) SQLCloseCursor(hStmt);
			printf("Match Success! \n");
			return id;
		}
	}
	
	printf("Match Failed! \n");
	if (hStmt) SQLCloseCursor(hStmt);

	return -1;
}


void Database::Show() {


	WCHAR ShowCommand[100] = { 0, };
	wsprintf(ShowCommand, L"Select * FROM Account");
	printf("%ws\n", ShowCommand);
	int ret = SQLExecDirect(hStmt, (SQLWCHAR*)ShowCommand, SQL_NTS);
	if (ret != SQL_SUCCESS) {
		Log("ShowFailed\n");
		if (hStmt) SQLCloseCursor(hStmt);
		return;
	}
	while (SQLFetch(hStmt) != SQL_NO_DATA) {
		printf("%d\t %s\t %s \n", id, name, password);
	}


	if (hStmt) SQLCloseCursor(hStmt);
}

