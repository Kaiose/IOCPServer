#pragma once
#include "stdafx.h"


#define terminalManager TerminalManager::getInstance()

class TerminalManager : public Singleton<TerminalManager> {

private:
	friend Singleton;

	int terminalNumber;

	CRITICAL_SECTION lock;

	TerminalManager() {
		InitializeCriticalSection(&lock);
		terminalNumber = 1;
	}

	
	
public:

	char serverName[NAME_SIZE];
	int ServerNumber;
	Terminal * LoginTerminal;
	std::unordered_map<std::string, Terminal*> TerminalMap;

	std::thread connectThread;

	std::vector<std::queue<Session*>*> terminalSessions_To_Process;

public:

	~TerminalManager() {

		if (connectThread.joinable())
		{
			connectThread.join();
		}


		DeleteCriticalSection(&lock);

		std::unordered_map<std::string, Terminal*>::iterator it = TerminalMap.begin();
		std::unordered_map<std::string, Terminal*>::iterator next_it;
		for (; it != TerminalMap.end(); it++) {
			it = next_it++;
			SAFE_DELETE(it->second);

		}

		SAFE_DELETE(LoginTerminal);

		for (int i = 0; i < terminalSessions_To_Process.size(); i++) {
			SAFE_DELETE(terminalSessions_To_Process[i]);
		}
	}


	Terminal* LoadCheck() {

		EnterCriticalSection(&lock);
		
		std::unordered_map<std::string, Terminal*>::iterator it = TerminalMap.begin();
		std::unordered_map<std::string, Terminal*>::iterator target = TerminalMap.end();
		std::unordered_map<std::string, Terminal*>::iterator altTarget = TerminalMap.end();
		Terminal* resultTerminal = nullptr;
		for (; it != TerminalMap.end(); it++) {
			if (it->second->load == -1) {
				LeaveCriticalSection(&lock);
				return nullptr;
			}

			if (it->second->currentState == RECEIVER) {
				if (target == TerminalMap.end())
					target = it;
				else if (target != TerminalMap.end() && (target->second->load > it->second->load)) {
					target = it;
				}
			}

			/*
				if (it->second->currentState == SENDER) {
				if (altTarget == TerminalMap.end() || (altTarget->second->load > it->second->load))
					altTarget = it;
			}*/
		}

		int temp = roomManager.GetRoom(roomManager.loadRoomNumber)->TransmittingPacketQuantity;
		if (target != TerminalMap.end())// && (roomManager.lastTransmittingPacketQuantity - target->second->load) > temp )
			resultTerminal = target->second;

		TerminalLoadInitialize();

		LeaveCriticalSection(&lock);

		return resultTerminal;
	}

	void TerminalLoadInitialize() {

	
		std::unordered_map<std::string, Terminal*>::iterator it = TerminalMap.begin();
		for (; it != TerminalMap.end(); it++) {
			it->second->load = -1;
		}

		
	}

	void SetServerNumber(int serverNumber) {
		std::unordered_map<std::string, Terminal*>::iterator it = TerminalMap.begin();
		for (; it != TerminalMap.end(); it++) {
			Terminal* terminal = it->second;
			terminal->serverNumber = serverNumber;
		}

	}

	void multi_Connect(HANDLE iocp, char serverName[], char ip[], int port) {

		connectThread = std::thread([=]() {
			
			std::unordered_map<std::string, Terminal*>::iterator it = TerminalMap.begin();
			std::unordered_map<std::string, Terminal*>::iterator remove_it;
			for (; it != TerminalMap.end();) {
				Terminal* terminal = it->second;
				remove_it = it;
				it++;
				if (!terminal->Connect(iocp, serverName, ip, port)) {
					TerminalMap.erase(remove_it);
					SAFE_DELETE(terminal);
				
				}
			}
		}
		);

	}


	void PushSession(int roomNumber, Session* session) {
		EnterCriticalSection(&lock);
		terminalSessions_To_Process[roomNumber - 1]->push(session);
		LeaveCriticalSection(&lock);
	}

	std::queue<Session*>* GetSessionVector(int roomNumber) {

		return terminalSessions_To_Process[roomNumber - 1];

	}

	void SessionProcess(int roomNumber, Packet_Ex* packet) {

		EnterCriticalSection(&lock);
		while (!terminalSessions_To_Process[roomNumber - 1]->empty()) {
			Session* session = terminalSessions_To_Process[roomNumber-1]->front();
			
			packet->accountId = session->accountId;
			/*
				account id 는 있는데
				packet에다 넣을 방법이 없음.
				S_NOTIFY를 쓰기엔 너무 하드코딩같고.. 좋은 방법이 없을까

			*/

			
			terminalSessions_To_Process[roomNumber-1]->pop();
			session->SendPacket(packet);

		}
		LeaveCriticalSection(&lock);

	}
	/*
	void SessionProcess(int roomNumber, Packet* packet) {

		EnterCriticalSection(&lock);
		while (!terminalSessions_To_Process[roomNumber - 1]->empty()) {
			Session* session = terminalSessions_To_Process[roomNumber - 1]->front();
			terminalSessions_To_Process[roomNumber - 1]->pop();
			session->SendPacket(packet);
		}
		LeaveCriticalSection(&lock);

	}
	*/ // 여기도 오버로딩. 
	void insert(std::string terminalName, Terminal* terminal) {
		TerminalMap.insert(std::make_pair(terminalName, terminal));
		strcpy_s(terminal->terminalName, terminalName.c_str());
		terminal->terminalNumber = terminalNumber++;
	}

	void multi_SendPacket(Packet* packet) { // 로그인서버 제외 게임서버들에 패킷을 날림

		std::unordered_map<std::string, Terminal*>::iterator it = TerminalMap.begin();

		for (; it != TerminalMap.end(); it++) {
			Session* session = it->second;
			printf("multi send : socket : %d\n",(int) session->socket);
			session->SendPacket(packet);
		}


	}

	void remove(const char* ip, int port) {

		std::unordered_map<std::string, Terminal*>::iterator it = TerminalMap.begin();
		std::unordered_map<std::string, Terminal*>::iterator remove_it = TerminalMap.end();
		for (; it != TerminalMap.end(); it++) {

			Terminal* temp = it->second;

			if (strcmp(temp->ip, ip) == 0 && temp->port == port) {
				remove_it = it;
				SAFE_DELETE(temp);

			}

		}
		if( remove_it != TerminalMap.end())
			TerminalMap.erase(remove_it);

	}

	void remove(std::string serverName) {

		std::unordered_map<std::string, Terminal*>::iterator it;
		it = TerminalMap.find(serverName);
		if (it == TerminalMap.end())
			return;

		Terminal* temp = it->second;
		TerminalMap.erase(it);
		SAFE_DELETE(temp);

	}

	Terminal* getTerminal(std::string terminalName) {

		if (TerminalMap.find(terminalName) == TerminalMap.end()) {
			return nullptr;
		}
		
		return TerminalMap.find(terminalName)->second;
	}

	Terminal* getTerminal(const char* ip, int port) {

		std::unordered_map<std::string, Terminal*>::iterator it = TerminalMap.begin();

		for (; it != TerminalMap.end(); it++) {

			Terminal* temp = it->second;

			if (strcmp(temp->ip, ip) == 0 && temp->port == port) {
				return temp;
			}

		}

		return nullptr;


	}

};


