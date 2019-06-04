#pragma once


#include "stdafx.h"


class RoomManager : public Singleton<RoomManager> {

	friend Singleton;
	
	RoomManager() {

	}

	std::unordered_map<int, Room*> RoomMap;

public:

	void AddRoom(Room* room) {
		RoomMap.insert(std::make_pair(room->GetRoomNumber(), room));
	}

	int Count() {
		return (int)RoomMap.size();
	}

	void DeleteRoom(Room* room) {
		if (RoomMap.empty()) return;

		RoomMap.erase(room->GetRoomNumber());
		
		delete room;
	}


	Room* GetRoom(int roomNumber) {
		if (RoomMap.empty()) return nullptr;

		return RoomMap.find(roomNumber)->second;
	}





};
