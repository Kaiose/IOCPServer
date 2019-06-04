#pragma once
#include "stdafx.h"



class Package {
public:
	Session * session;
	Packet*		packet;
	
	
public:
	Package(Session* session, Packet* packet) :session(session), packet(packet) {};

	~Package() {
//		delete packet;
	}
};


class ContentsProcess
{
	//std::queue<Package*> Package_Queue;

	// 11/19 
	// 병목현상 해결을 위한 read write 분화 <- critical section 도 분화 
	// Package_Queue[0] : read Queue
	// Package_Queue[1] : write Queue;
	std::queue<Package*> Package_Queue[2];
	CRITICAL_SECTION readlock;
	CRITICAL_SECTION writelock;
	std::thread hThread[THREAD_COUNT];

protected:


public:

	typedef void PacketFunc(Session* session, Packet* packet);
	std::unordered_map<PacketType, PacketFunc*> PacketFuncTable;
	std::string		className;
	static HANDLE serverIOCPHandle;
public:
	ContentsProcess();
	~ContentsProcess();

	void Initialize();
	void Run();
	void AddPackage(Package* package);
	Package* GetPackage();


	
};

