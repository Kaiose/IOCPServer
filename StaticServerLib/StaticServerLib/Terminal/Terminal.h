#include "stdafx.h"

class Terminal : public Session{

public:

	bool IsConnected;

	char terminalName[NAME_SIZE];

	int serverNumber;
	int terminalNumber;
	

	int load;

	STATE currentState;

	Terminal(){
		load = -1;
		serverNumber = 0;
		terminalNumber = 0;
		memset(terminalName, NULL, NAME_SIZE);
		currentState = RECEIVER;
	}

	~Terminal() {
		Clear();

		IsConnected = true;

	}

	bool Connect(HANDLE iocp, char serverName[], char ip[], int port) {
		socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

		SOCKADDR_IN remoteAddr;
		remoteAddr.sin_family = PF_INET;
		remoteAddr.sin_port = htons(this->port);
		inet_pton(AF_INET, this->ip, &remoteAddr.sin_addr);


		if (connect(socket, (sockaddr*)&remoteAddr, sizeof(remoteAddr)) != SOCKET_ERROR) {
					IsConnected = true;
					printf("Server: %s  Ip : %s , Port %d  ==>> terminal is connected!\n",terminalName ,ip, port);
					this->SetType();
					CreateIoCompletionPort((HANDLE)socket, iocp, (ULONG_PTR)this, 0);
					//this->ZeroByteReceive();
					this->RecvStanby();
					PK_T_REQ_CONNECT packet;
					packet.terminalNumber = serverNumber;
					memcpy(packet.serverName, serverName, 20);
					memcpy(packet.ip, ip, 20);
					packet.port = port;
					SendPacket(&packet);
					

					return true;
		}

		printf("Server: %s , Ip : %s , Port %d  ==>> terminal is not connected!\n",terminalName, ip, port);
		return false;
		
	}

	void Clear() {
		closesocket(socket);
		SAFE_DELETE(ioData);
	}

	void SendPacket(Packet* packet) {
		printf("socket : %d\n", (int)this->socket);
		Session::SendPacket(packet);

	}

	void registAddress(const char* ip, int port) {
		memcpy(this->ip, ip, 20);
		this->port = port;
	}

};

