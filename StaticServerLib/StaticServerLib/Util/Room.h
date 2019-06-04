#pragma once

#include "stdafx.h"


class Room {


	
	int count; // unit count
	
	int roomNumber;


	// Room에서 부여해주는 Number 와 User 객체
	std::unordered_map<int,User*> UserMap;

	//CRITICAL_SECTION lock;

	SRWLOCK lock;

	bool shutdown;

public:
	int size; // balancing 시점 size

	int SendingPacketCount;
	int RecevingPacketCount;

	int SendingPacketQuantity;
	int RecevingPacketQuantity;

	int TransmittingPacketQuantity;


public:

	Room() {
		InitializeSRWLock(&lock);
		count = 0;
		size = 0;
		roomNumber = 0;
	
		shutdown = false;

		ZeroTransmittingPacket();
	}

	~Room() {

	}

public:

	void ZeroTransmittingPacket() {

		SendingPacketCount = 0;
		SendingPacketQuantity = 0;
		RecevingPacketCount = 0;
		RecevingPacketQuantity = 0;
	}

	LONG TransmittingPacketCheck() {

	
		Log("Room Number : %d	Sending Packet Count : %d		Sending Packet Quantity : %d	Receiving Packet Count : %d		Receiving Packet Quantity : %d		Total Quantity : %d\n",
			roomNumber, SendingPacketCount, SendingPacketQuantity, RecevingPacketCount, RecevingPacketQuantity, SendingPacketQuantity + RecevingPacketQuantity);
		TransmittingPacketQuantity = SendingPacketQuantity + RecevingPacketQuantity;
		ZeroTransmittingPacket();

		return TransmittingPacketQuantity;
	}
	
	bool UsersReadyAllGreen() {
		std::unordered_map<int, User*>::iterator it = UserMap.begin();

		while (it != UserMap.end()) {
			if (it->second->IsReady() == false) {
				Log("User is not Read roomNumber : %d userNumber : %d  is not set!", it->second->GetRoomNumber(), it->second->GetUserNumber());
	/*			if (it->second->tryCount >= 2) {
					User* user2Erase = it->second;
					UserMap.erase(it);
					userManager.DeleteUser(user2Erase);
					return false;
				}
				it->second->tryCount += 1;
	*/
				return false;
			}
			it++;
		}

		return true;
	}

	bool CheckUserMap() {

		std::unordered_map<int, User*>::iterator it = UserMap.begin();
		
		while (it != UserMap.end()) {
			if (it->second->GetSession() == nullptr) {
				Log("Check User Map Failed !!! roomNumber : %d userNumber : %d  is not set!", it->second->GetRoomNumber(), it->second->GetUserNumber());
				if (it->second->tryCount >= 2) {
					User* user2Erase = it->second;
					UserMap.erase(it);
					userManager.DeleteUser(user2Erase);
					return false;
				}
				it->second->tryCount += 1;

				return false;
			}
			it++;
		}

		return true;

	}

	bool BalanceAvailable() {
		if (size > UserMap.size()) {
			Log("TargetRoom not connected All UserMap size : %d GoalSize : %d\n", UserMap.size(), size);
			return false;
		}
		else
			return true;

	}

	int GetCount() { return count; }
	void SetCount(int count) { this->count = count; }
	int GetRoomNumber() { return roomNumber; }

	void SetRoomNumber(int roomNumber) {
		this->roomNumber = roomNumber;
	}

	int AddUser(User* user) {
		AcquireSRWLockExclusive(&lock);
		UserMap.insert(std::make_pair(++count, user));
		int userNumber = count;
		size += 1;
		ReleaseSRWLockExclusive(&lock);
	
	
		return userNumber;
	}

	int AddUser(User* user, int userNumber) { // not increase Count
		AcquireSRWLockExclusive(&lock);
		UserMap.insert(std::make_pair(userNumber, user));
		size += 1;
		ReleaseSRWLockExclusive(&lock);


		return userNumber;
	}

	void DeleteUser(User* user) {
		std::unordered_map<int, User*>::iterator it;


		AcquireSRWLockExclusive(&lock);
		if (UserMap.empty()) {
			ReleaseSRWLockExclusive(&lock);
			return;
		}
		for (it = UserMap.begin(); it != UserMap.end(); it++) {
			if (it->second == user) {
				UserMap.erase(it);
				size -= 1;
				break;
			}
		}
		ReleaseSRWLockExclusive(&lock);

	}


	void DeleteUser(int userNumber) {
		AcquireSRWLockExclusive(&lock);


		if (UserMap.empty()) {
			ReleaseSRWLockExclusive(&lock);
			return;
		}
		UserMap.erase(userNumber);
		size -= 1;
		ReleaseSRWLockExclusive(&lock);

	}

	User* GetUser(int userNumber) {

		std::unordered_map<int, User*>::iterator it;
		AcquireSRWLockShared(&lock);
		it = UserMap.find(userNumber);
		if (it == UserMap.end()) {
			ReleaseSRWLockShared(&lock);
			return nullptr;
		}
		User* user = it->second;
		ReleaseSRWLockShared(&lock);
		return user;
	}


	std::unordered_map<int, User*>& GetMap() {
		return UserMap;
	}
};