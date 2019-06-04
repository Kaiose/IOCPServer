#pragma once

#include "stdafx.h"


static PVOID GetSockExtAPI(SOCKET sock, GUID guidFn);



//extern std::unordered_map<std::string, Terminal*> TerminalMap;
class Server
{
	WSAData wsaData;
	SOCKET	lstnSocket;
	char	serverName[NAME_SIZE];
	char	ip[20];
	int		port;
	std::thread hThread[THREAD_COUNT*2];
	
	/*
	std::unordered_set<Session*>	Using_SessionPool;
	std::unordered_set<Session*>	Unused_SessionPool;
	*/
	std::unordered_map<int, Session*> Using_SessionPool;
	std::unordered_map<int, Session*> Unused_SessionPool;

	//std::lock_guard �� ���� �ʴ� ������
	//std:: lock_guard�� ��������� ��� unlock�� ���ؼ� �Ҹ��ڰ� ȣ��Ǿ���..
	//static std::unique_lock<std::recursive_mutex> lock;

//	std::recursive_mutex mutex;

public:
	HANDLE hIOCP;


	CRITICAL_SECTION lock;
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

	void ExceptionProcss(Session* session);

	void Connect();
	void ShowPool();
	
	void GetAddress(Session* session);

	bool OnAccept(Session* session);
	Package* Packaging(Session* session, Packet* packet);

	virtual void ReadXML();


	void LoadCheck();

};
