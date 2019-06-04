#pragma once

#include "stdafx.h"


static PVOID GetSockExtAPI(SOCKET sock, GUID guidFn);



class Server
{
	WSAData wsaData;
	SOCKET	lstnSocket;
	char	ip[20];
	int		port;
	std::thread hThread[THREAD_COUNT];

	std::unordered_set<Session*>	Using_SessionPool;
	std::unordered_set<Session*>	Unused_SessionPool;

	std::recursive_mutex mutex;
	

public:
	HANDLE hIOCP;


	//CRITICAL_SECTION lock;
	std::string		className;
	ContentsProcess * contentsProcess;

	Server();
	Server(ContentsProcess* contentsProcess);
	~Server();

	void Create_Resources();
	void Release_Resources();
	void Initialize();
	void Listen();
	void Accept();
	void Create_Iocp_ThreadPool();
	void Create_Iocp_Handle();
	void Prepare_Accept(Session* session);
	void Resource_Recycle(Session* session);
	void Run();

	void ShowPool();
	
	void GetAddress(Session* session);

	void OnAccept(Session* session);
	Package* Packaging(Session* session, Packet* packet);

	void ConnectingSessions(char* ip, int port);

	virtual void ReadXML();


	int virtualNumber;


};

static PVOID GetSockExtAPI(SOCKET sock, GUID guidFn) {
	PVOID pfnEx = nullptr;
	GUID guid = guidFn;
	DWORD dwBytes = 0;
	LONG lret = WSAIoctl(
		sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guid, sizeof(guid),
		&pfnEx, sizeof(pfnEx),
		&dwBytes, NULL, NULL
	);

	if (lret == SOCKET_ERROR) return NULL;

	return pfnEx;
}

