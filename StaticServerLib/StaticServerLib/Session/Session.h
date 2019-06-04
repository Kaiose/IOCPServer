#pragma once
#include "stdafx.h"



class Session
{
	
public:
	SOCKET socket;
	int accountId;
	char ip[20];
	int port;


	char mainBuffer[BUF_SIZE];
	int totalBytes;

	IO_Data* ioData;
	
	std::queue<Packet*> packetQueue;

	int recvCount;
	int sendCount;

	
public:

	Session();
	Session(SOCKET socket);
	virtual  ~Session()
	{
		Clear();

	}

	void Initialize();
	void SetType();
	void SendPacket(Packet* packet);
	void SyncSendPacket(Packet* packet);
	void Recv(int recvBytes);
	void Close();

	void Clear();

	void RecvStanby();
	void ZeroByteReceive();
	int getId();
	void setId(int id);
	
	void queueClear();

	Packet* GetPacketClass(PacketType packetType);
	Packet* GetPacket();
	bool PacketAnalyzer();


	void Connect();
};
