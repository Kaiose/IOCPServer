#include "stdafx.h"

#include "ContentsProcess.h"


ContentsProcess::ContentsProcess()
{

	className = "ContentsProcess";

	Initialize();
}


ContentsProcess::~ContentsProcess()
{

	for (int i = 0; i < THREAD_COUNT; i++)
		hThread[i].join();
	DeleteCriticalSection(&readlock);
	DeleteCriticalSection(&writelock);
	std::unordered_map<std::string, ServerAddr*>::iterator it;
	for (it = ServerAddrTable.begin(); it != ServerAddrTable.end(); it++) {
		ServerAddr* temp = it->second;
		delete temp;
	}

}


void ContentsProcess::Initialize() {
	InitializeCriticalSection(&writelock);
	InitializeCriticalSection(&readlock);
	status = START;
	for (int i = 0; i < THREAD_COUNT; i++) {
		hThread[i] = std::thread( &ContentsProcess::Run, this );
	}

}

void ContentsProcess::Run() {
	
	while (status != STOP) {
		//Context switch  
		// is increase CPU Usage but need
		
		Package* package = GetPackage();
		if (package == nullptr) {

		//	std::chrono::milliseconds duration(1);
		//	std::this_thread::sleep_for(duration);
			//thread�� ������ �ٸ� ������� �ѱ�� yield ������
			//������ ������ ����Ͽ� CPU��뷮�� ���������� �����ϴ� ������ �߻�
			//sleep_for�� ����..
			Sleep(1);
			//std::this_thread::yield();
			//	printf("package is null \n");
			continue;
		}
		//��ȯ ���� iterator
		std::unordered_map<PacketType, PacketFunc*>::iterator it;
		it = PacketFuncTable.find(package->packet->GetType());
		//TODO : package ���������
		if (it == PacketFuncTable.end()) {
			printf(" Is not Regist PacketFunc in Table");
			SAFE_DELETE(package);
			continue;
		}

		PacketFunc* func = it->second;

		//Log("Receving!  packet Number : %d\n", package->packet->GetType());
		func(package->session, package->packet);
		SAFE_DELETE(package);

	}
	
}


void ContentsProcess::AddPackage(Package* package) {
	EnterCriticalSection(&writelock);
	if (package == nullptr) {
		printf("package is null");
	}

	Package_Queue[IO_WRITE].push(package);
	LeaveCriticalSection(&writelock);
}


inline Package* ContentsProcess::GetPackage() {
	Package* temp = nullptr;
	EnterCriticalSection(&readlock);
	if (Package_Queue[IO_READ].empty() == true) {
		if (Package_Queue[IO_WRITE].empty() == true) {
			LeaveCriticalSection(&readlock);
			return temp;
		}

		//�������� �ذ�
		EnterCriticalSection(&writelock);
		//Log("PackageQueue Swap");
		//Log("PackageQueue Swap");
		Package_Queue[IO_READ].swap(Package_Queue[IO_WRITE]);
		LeaveCriticalSection(&writelock);


	}
	temp = Package_Queue[IO_READ].front();
	Package_Queue[IO_READ].pop();
	LeaveCriticalSection(&readlock);
	return temp;
}

