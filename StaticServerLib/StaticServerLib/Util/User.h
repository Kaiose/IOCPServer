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

	int	direction;

	bool isReady;

	const int timeout = 1000;
	std::chrono::system_clock::time_point checkPoint;
	const float speed = 1.0f;

	float var;
	float crossVar;
	int memoryAddress;

	

public:
	int tryCount;
	
	User(Session* session,int id, const char* name) :session(session), accountId(id) {
		roomNumber = -1;
		userNumber = -1;
		pos_X = 1.0f;
		pos_Y = -1.0f;
		memoryAddress = 0;
		memcpy(this->name, name, NAME_SIZE);
		//memcpy(nextIp, NULL, NAME_SIZE);

		isReady = false;
		direction = 0;

		var = speed / timeout;
		crossVar = sqrtf(2.0f);


		tryCount = 0;
	}

	User(int id, const char* name) : accountId(id) {
		roomNumber = -1;
		userNumber = -1;
		pos_X = 1.0f;
		pos_Y = -1.0f;

		memcpy(this->name, name, NAME_SIZE);
	//	memcpy(nextIp, NULL, NAME_SIZE);

		isReady = false;
		direction = 0;

		var = speed / timeout;
		crossVar = sqrtf(2.0f);

		tryCount = 0;
	}

public:

	void SetAddress(int address) {
		memoryAddress = address;
	}

	void Update() {
		if (std::chrono::duration_cast<std::chrono::milliseconds>(checkPoint - std::chrono::system_clock::now()).count() > timeout) 
			return;
		
		float pos_x, pos_y;
		GetPositition(pos_x, pos_y);
		switch (direction) {
			case 1:
				pos_y += var;
				break;
			case 2:
				pos_x += (var / crossVar);
				pos_y += (var / crossVar);
				break;
			case 3:
				pos_x += var;
				break;
			case 4:
				pos_x += (var / crossVar);
				pos_y -= (var / crossVar);
				break;

			case 5:
				pos_y -= var;
				break;
			case 6:
				pos_x -= (var / crossVar);
				pos_y -= (var / crossVar);
				break;
			case 7:
				pos_x -= var;
				break;
			case 8:
				pos_x -= (var / crossVar);
				pos_y += (var / crossVar);
			case 0:
			default:
				break;
		}
		SetPosition(pos_x, pos_y);


		return;
	}

	int GetAccountId() {
		return accountId;
	}

	void SetAccountId(int accountId) {
		this->accountId = accountId;
	}

	void SetDirection(int direction) {
		checkPoint = std::chrono::system_clock::now();
		this->direction = direction;
	}

	int GetDirection() {
		return direction;
	}

	void SetReady() {
		isReady = true;
	}

	bool IsReady() {
		return isReady;
	}

	void ResetReady() {
		isReady = false;
	}
	const char* GetName() { return name; }

	void SetUserNumber(int userNumber) {
		this->userNumber = userNumber;
	}

	int GetUserNumber() { return this->userNumber; }


	void SetPosition(float pos_X, float pos_Y) { 
		this->pos_X = pos_X;
		this->pos_Y = pos_Y;
		checkPoint = std::chrono::system_clock::now();
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

	void SetSession(Session* session) {
		this->session = session;
	}
};