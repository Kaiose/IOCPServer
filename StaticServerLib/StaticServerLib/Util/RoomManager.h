#pragma once


#include "stdafx.h"

#define roomManager RoomManager::getInstance()

class RoomManager : public Singleton<RoomManager> {

	friend Singleton;
	
	SRWLOCK lock;

	RoomManager() {
		
		lastTransmittingPacketQuantity = 0;
		InitializeSRWLock(&lock);
	}

	
	~RoomManager() {
		
		
	}

public:

	std::unordered_map<int, Room*> RoomMap;

	int lastTransmittingPacketQuantity;
	int loadRoomNumber;
public:

	void AddRoom(Room* room) {
		AcquireSRWLockExclusive(&lock);
		RoomMap.insert(std::make_pair(room->GetRoomNumber(), room));
		ReleaseSRWLockExclusive(&lock);
	}

	bool CreateRoom(int roomNumber) {

		if (RoomMap.find(roomNumber) != RoomMap.end()) {
			Log("Create Room Failed !!! Room : %d is already exist! \n", roomNumber);
			return false;
		}

		Room* room = new Room();
		room->SetRoomNumber(roomNumber);
		//LockManager::getInstance().CreateLock(roomNumber);
		AddRoom(room);
		// check transmitting packet quantity;
		
		return true;
	}

	


	int Count() {
		return (int)RoomMap.size();
	}

	void DeleteRoom(Room* room) {

		AcquireSRWLockExclusive(&lock);
		if (RoomMap.empty()) {
			ReleaseSRWLockExclusive(&lock);
			return;
		}
		RoomMap.erase(room->GetRoomNumber());

		
		SAFE_DELETE(room);
		ReleaseSRWLockExclusive(&lock);
	}


	Room* GetRoom(int roomNumber) {

		AcquireSRWLockShared(&lock);
		if (RoomMap.empty() || (RoomMap.find(roomNumber) == RoomMap.end())) {
			ReleaseSRWLockShared(&lock);
			return nullptr;
		}
		Room *room = RoomMap.find(roomNumber)->second;
		ReleaseSRWLockShared(&lock);

		return room;
	}






};
