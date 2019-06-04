#pragma once

#include "stdafx.h"
#include <sql.h>
#include <sqlext.h>


#define database Database::getInstance()

class Database : public Singleton<Database>
{

	friend Singleton;

public:
	int id;

	//SQLWCHAR name[20];
	//SQLWCHAR password[20];


	CHAR name[20];
	CHAR password[20];


	SQLLEN idLen;
	SQLLEN nameLen;
	SQLLEN passwordLen;


public:
	SQLHENV		hEnv; // ȯ�� �ڵ�
	SQLHDBC		hDbc; // ���� �ڵ��� ������
	SQLHSTMT	hStmt;

	SQLWCHAR* ODBC_NAME;
	SQLWCHAR* ODBC_ID;
	SQLWCHAR* ODBC_PW;

public:
	Database();
	~Database();

	bool DBConnect();
	bool DBDisConnect();

	void AttributeBind();

	bool Insert(int id, const WCHAR* name, const WCHAR* password);

	bool Insert(const CHAR* name, const CHAR* password);

	bool Delete(int id);

	int Match(const char* name, const char* password);

	void Show();
};

