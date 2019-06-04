#pragma once


#include "stdafx.h"
#include "Util\Profile.h"


class Monitor : public Singleton<Monitor> {

	friend Singleton;

	std::thread		monitor;
	SOCKET		socket;
	SOCKET		listenSocket;
	int			serverNumber;

	STATE		currentState;
	
	CRITICAL_SECTION	cs;

	Monitor() {
		InitializeCriticalSection(&cs);
		currentState = RECEIVER;
		monitor = std::thread { &Monitor::Execute, this };
		
	}

	~Monitor() {
		DeleteCriticalSection(&cs);
		monitor.join();
	
	}

public:

	void Initialize(const char* ip, int port) {
		socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0 );
		SOCKADDR_IN serverAddr;
		serverAddr.sin_family = AF_INET;
		inet_pton(AF_INET, ip, &(serverAddr.sin_addr));
		serverAddr.sin_port = htons(port);

		if (connect(socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
			printf("connect error : %d , port : %d \n", WSAGetLastError(), port);
		}

	}

	/*
		packet = [Server Number][TransmittingPacketQuantity]
	*/
	void Send(int serverNumber, int transmittingPacketQuantity) {
		if (socket == INVALID_SOCKET)
			return;
		char buf[100] = { 0, };
		int intSize = sizeof(int);
		memcpy(buf, &serverNumber, intSize);
		memcpy(&buf[intSize], &transmittingPacketQuantity, intSize);
		if (!send(socket, buf, 8, 0)) {
			printf("Monitor send error!");
		}
	}

	STATE GetState() {

		EnterCriticalSection(&cs);
		STATE temp = currentState;
		LeaveCriticalSection(&cs);
		return temp;
	}

	void ChangeState(STATE state) {
		EnterCriticalSection(&cs);
		currentState = state;
		LeaveCriticalSection(&cs);
	}


	void Execute() {

		while (true) {
			std::unordered_map<int, Room*>::iterator it = roomManager.RoomMap.begin();
		//	Work::getInstance().getSystemInfo();
			if (it == roomManager.RoomMap.end()) {
				ChangeState(RECEIVER);
				continue;
			}
		
			int TransmittingPacketQuantity = 0;
			int load_roomNumber = -1;
			int load_max = 0;

			load_roomNumber = it->second->GetRoomNumber();
			load_max = it->second->TransmittingPacketQuantity;

			for (; it != roomManager.RoomMap.end(); it++) {
				TransmittingPacketQuantity += it->second->TransmittingPacketCheck();

				if (load_max < it->second->TransmittingPacketQuantity) {
					load_max = it->second->TransmittingPacketQuantity;
					load_roomNumber = it->second->GetRoomNumber();
				}
			}

			roomManager.lastTransmittingPacketQuantity = TransmittingPacketQuantity;
			roomManager.loadRoomNumber = load_roomNumber;

			Room* targetRoom = roomManager.GetRoom(load_roomNumber);

			if (TransmittingPacketQuantity > MAX_TRAFFIC &&  targetRoom->CheckUserMap()&& targetRoom->UsersReadyAllGreen() && currentState != BLOCK) {
				ChangeState(SENDER);

				Log("Load Generation!! RoomNumber : %d	  Higher Room Packet Quantity  : %d	 Total Packet Quantity : %d \n"
					, load_roomNumber, load_max, TransmittingPacketQuantity);


				PK_T_ASK_LOAD packet;
				packet.roomNumber = load_roomNumber;
				packet.packetQuantity = load_max;
				terminalManager.multi_SendPacket(&packet);

			}
			else if (TransmittingPacketQuantity > MAX_TRAFFIC)// 20초 동안 로드가 끝나지 않아서  서버가 터진다면 알고리즘이 잘못됀것
				ChangeState(SENDER);
			else if (TransmittingPacketQuantity < MAX_TRAFFIC)
				ChangeState(RECEIVER);
			Send(serverNumber, TransmittingPacketQuantity);

			Sleep(25000);

		}
	}


};