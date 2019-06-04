#pragma once
#include "stdafx.h"



enum Direction {
	N,
	T,
	TR,
	R,
	TB,
	B,
	BL,
	L,
	LT,
};

static float distance = 0.1;

class Session
{
	
public:
	bool sessionStatus;

	SOCKET socket;
	char ip[20];
	int port;
	CRITICAL_SECTION critical_section;

	char mainBuffer[BUF_SIZE];
	int totalBytes;

	IO_Data* ioData;
	
	std::queue<Packet*> packetQueue;

	int recvCount;
	int sendCount;
	bool threadStatus;
	std::thread move;


	HANDLE hEvent;

	std::queue<SOCKET> sockQueue;

public:
	int accountId;
	int roomNumber;
	int userNumber;

	float pos_X;
	float pos_Y;


	char nextIp[20];
	int nextPort;
	SOCKET socket2close;


	Session(SOCKET socket);
	~Session();

	void Initialize();
	void SetType();
	void SendPacket(Packet* packet);
	void Recv(int recvBytes);
	void Close();

	void Clear();

	void RecvStanby();
	void ZeroByteReceive();
	int getId();
	void setId(int id);
	void setSocket(SOCKET socket);
	Packet* GetPacketClass(PacketType packetType);
	Packet* GetPacket();
	bool PacketAnalyzer();

	
	void Connect(const char* ip, int port);
	void ChangeConnect();
	void randomMoving();

	void lock();
	void unlock();

	void ChangeSocket();
};

