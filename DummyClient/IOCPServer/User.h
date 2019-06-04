#pragma once

#include "stdafx.h"

class User {

	int accountId;
	char name[NAME_SIZE];

	float pos_X;
	float pos_Y;

	int roomNumber;

	int userNumber;

	Session* session;
	//CRITICAL_SECTION lock;

public:

	User(Session* session,int id, const char* name) :session(session), accountId(id) {
		roomNumber = -1;
		userNumber = -1;
		pos_X = 1.0f;
		pos_Y = -1.0f;

		memcpy(this->name, name, NAME_SIZE);
		
	}

public:

	int GetAccountId() {
		return accountId;
	}

	const char* GetName() { return name; }

	void SetUserNumber(int userNumber) {
		this->userNumber = userNumber;
	}

	int GetUserNumber() { return this->userNumber; }


	void SetPosition(float pos_X, float pos_Y) { 
		this->pos_X = pos_X;
		this->pos_Y = pos_Y;
	}

	void GetPositition(float &pos_X, float &pos_Y) {
		pos_X = this->pos_X;
		pos_Y = this->pos_Y;
	}

	int GetRoomNumber() {
		return roomNumber;
	}

	void SetRoomNumber(int roomNumber) {
		this->roomNumber = roomNumber;
	}

	Session* GetSession() {
		return session;
	}
};