#pragma once

#include "stdafx.h"


class Room {


	int count;

	int roomNumber;


	// Room���� �ο����ִ� Number �� User ��ü
	std::unordered_map<int,User*> UserMap;

	
	SRWLOCK lock;

public:

	Room() {
		InitializeSRWLock(&lock);
		count = 0;
		roomNumber;
	}

	~Room() {
	
	}

public:
	int GetCount() { return count; }

	int GetRoomNumber() { return roomNumber; }

	void SetRoomNumber(int roomNumber) {
		this->roomNumber = roomNumber;
	}

	int AddUser(User* user) {
		AcquireSRWLockExclusive(&lock);
		UserMap.insert(std::make_pair(++count, user));
		int userNumber = count;
		ReleaseSRWLockExclusive(&lock);
	
		return userNumber;
	}


	void DeleteUser(User* user) {
		// User��ü�� ����� ���� ��ȸ�� �ʿ��ϹǷ� Key�� ����� ���� ����.
		if (UserMap.empty()) return;

		std::unordered_map<int, User*>::iterator it;

		AcquireSRWLockExclusive(&lock);
		for (it = UserMap.begin(); it != UserMap.end(); it++) {
			if (it->second == user) {
				UserMap.erase(it);
				//count--; count�� ���� ��ȣ�� �ű���־��µ� ���Ұ��Ͼ�� ��ĥ ���ɼ��� ������.
			}
		}
		ReleaseSRWLockExclusive(&lock);
	}
	void DeleteUser(int userNumber) {
		if (UserMap.empty()) return;

		AcquireSRWLockExclusive(&lock);
		UserMap.erase(userNumber);
		//count--;
		ReleaseSRWLockExclusive(&lock);

	}

	User* GetUser(int userNumber) {
		AcquireSRWLockShared(&lock);
		User* user	=UserMap.find(userNumber)->second;

		ReleaseSRWLockShared(&lock);
		return user;
	}


	std::unordered_map<int, User*>& GetMap() {
		return UserMap;
	}
};