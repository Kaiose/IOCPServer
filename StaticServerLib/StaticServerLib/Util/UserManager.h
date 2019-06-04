#pragma once

#include "stdafx.h"


#define userManager UserManager::getInstance()

class UserManager : public Singleton<UserManager> {


	friend Singleton;

	

	// 고유한 계정 ID 와 User객체
	std::unordered_map<int, User*> UserMap;
	
//	CRITICAL_SECTION lock;
	SRWLOCK lock;

	bool shutdown;
	std::thread bufferingThread;
	UserManager() {
		//InitializeCriticalSection(&lock);
		shutdown = false;
		InitializeSRWLock(&lock);

//		bufferingThread = std::thread{ &UserManager::Buffering, this };


	}

	~UserManager() {
		shutdown = true;
		bufferingThread.join();
		//DeleteCriticalSection(&lock);
	}
public:
	/*
	
			TODO :: SessionManager 에서 Buffering을 스레드로 실행
			Buffering 함수를 통해  UserMap을 돌면서 버퍼링 , 보간한다.

	*/

	void Buffering() {
		std::unordered_map<int, User*>::iterator mapIter = UserMap.begin();

		while (!shutdown) {
			AcquireSRWLockShared(&lock);
			if (mapIter == UserMap.end()) {
				mapIter = UserMap.begin();
				ReleaseSRWLockShared(&lock);
				Sleep(500);
				continue;
			}
			mapIter->second->Update();
			mapIter++;
			ReleaseSRWLockShared(&lock);
		}
	}


	User* GetUser(int accountId) {
		std::unordered_map<int, User*>::iterator it;
		//EnterCriticalSection(&lock);
		AcquireSRWLockExclusive(&lock);
		it = UserMap.find(accountId);
		if (it == UserMap.end()) {
			//Log("User is not exist\n");
			ReleaseSRWLockExclusive(&lock);
			return nullptr;
		}

		User* target = it->second;

		ReleaseSRWLockExclusive(&lock);
		//LeaveCriticalSection(&lock);
		return target;
	}

	bool IsEmpty() {
		if (UserMap.empty()) return true;
		return false;
	}
	

	void AddUser(User* user) {
		//EnterCriticalSection(&lock);
		AcquireSRWLockExclusive(&lock);
		UserMap.insert(std::make_pair(user->GetAccountId(), user));
		ReleaseSRWLockExclusive(&lock);
		//LeaveCriticalSection(&lock);
	}


	void DeleteUser(User* user) {
		//EnterCriticalSection(&lock);
		AcquireSRWLockExclusive(&lock);
		if (UserMap.empty()) {
			ReleaseSRWLockExclusive(&lock);
			return;
		}
		UserMap.erase(user->GetAccountId());
		
		//LeaveCriticalSection(&lock);
		SAFE_DELETE(user);
		ReleaseSRWLockExclusive(&lock);
	}

	void DeleteUser(int accountId) {
		//EnterCriticalSection(&lock);
		std::unordered_map<int, User*>::iterator deletingUserIter;
		AcquireSRWLockExclusive(&lock);

		Log("Userdelete\n");
		deletingUserIter = UserMap.find(accountId);
		if (deletingUserIter == UserMap.end()) {
			Log("Delete User not exist\n");
			ReleaseSRWLockExclusive(&lock);
			return;
		}

		User* user = deletingUserIter->second;
		UserMap.erase(user->GetAccountId());
		
		//LeaveCriticalSection(&lock);
		SAFE_DELETE(user);
		ReleaseSRWLockExclusive(&lock);
	}
};
