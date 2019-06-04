#pragma once

#include "stdafx.h"

class UserManager : public Singleton<UserManager> {


	friend Singleton;

	

	// 고유한 계정 ID 와 User객체
	std::unordered_map<int, User*> UserMap;

	CRITICAL_SECTION lock;

	UserManager() {
		InitializeCriticalSection(&lock);
	}

	~UserManager() {
		DeleteCriticalSection(&lock);
	}
public:




	User* GetUser(int accountId) {
		if (UserMap.empty()) return nullptr;
		User* target = UserMap.find(accountId)->second;
		return target;
	}

	bool IsEmpty() {
		if (UserMap.empty()) return true;
		return false;
	}
	

	void AddUser(User* user) {
		UserMap.insert(std::make_pair(user->GetAccountId(), user));
	}


	void DeleteUser(User* user) {
		if (UserMap.empty()) return;



		UserMap.erase(user->GetAccountId());

		delete user;

	}

	void DeleteUser(int accountId) {

		if (UserMap.empty()) return;
		User* user = GetUser(accountId);


		UserMap.erase(user->GetAccountId());

		delete user;

	}
};
