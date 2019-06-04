#pragma once
#include "stdafx.h"


#define SHARED_LOCK(roomNumber)			SharedLock shared(roomNumber)
#define EXCLUSIVE_LOCK(roomNumber)		ExclusiveLock Exclusive(roomNumber)


class LockManager : public Singleton<LockManager> {

	friend Singleton;
	SRWLOCK lockArray[10];

	LockManager() {
		for(auto lock : lockArray){
			InitializeSRWLock(&lock);
		}
	}

public:

	void sharedLock(int lockNumber) {
		AcquireSRWLockShared(&lockArray[lockNumber]);
	}

	void sharedUnLock(int lockNumber) {
		ReleaseSRWLockShared(&lockArray[lockNumber]);
	}

	void ExclusiveLock(int lockNumber) {
		AcquireSRWLockExclusive(&lockArray[lockNumber]);
	}

	void ExclusiveUnLock(int lockNumber) {
		ReleaseSRWLockExclusive(&lockArray[lockNumber]);
	}
};

class SharedLock {
	int lockNumber;
public:

	SharedLock(int roomNumber) :lockNumber(roomNumber) {
		LockManager::getInstance().sharedLock(lockNumber);
	}

	~SharedLock() {
		LockManager::getInstance().sharedUnLock(lockNumber);
	}
};

class ExclusiveLock {
	int lockNumber;
public:
	ExclusiveLock(int roomNumber) :lockNumber(roomNumber) {
		LockManager::getInstance().ExclusiveLock(lockNumber);
	}

	~ExclusiveLock() {
		LockManager::getInstance().ExclusiveUnLock(lockNumber);
	}
};