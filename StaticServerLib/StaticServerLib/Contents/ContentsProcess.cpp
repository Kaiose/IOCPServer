#include "stdafx.h"

#include "ContentsProcess.h"

HANDLE ContentsProcess::serverIOCPHandle = NULL;

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
	/*std::unordered_map<std::string, Terminal*>::iterator it;
	for (it = TerminalMap.begin(); it != TerminalMap.end(); it++) {
		Terminal* temp = it->second;
		delete temp;
	}*/

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
		

		Package* package = GetPackage();
		if (package == nullptr) {

				Sleep(1);
			/*thread의 실행을 다른 스레드로 넘기는 yield 구문은
			스레드 갯수에 비례하여 CPU사용량이 폭발적으로 증가하는 문제가 발생
			sleep_for로 변경..
			printf("package is null \n");
			
			*/
			//std::this_thread::yield();
			
			
			continue;
		}
		//반환 값은 iterator
		std::unordered_map<PacketType, PacketFunc*>::iterator it;
		it = PacketFuncTable.find(package->packet->GetType());
		//TODO : package 없애줘야함
		if (it == PacketFuncTable.end()) {
			printf(" Is not Regist PacketFunc in Table");
			SAFE_DELETE(package);
			continue;
		}

	//	Log("Receving!  packet Number : %d\n", package->packet->GetType());
		PacketFunc* func = it->second;
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

		//*병목현상 해결
		EnterCriticalSection(&writelock);
		Package_Queue[IO_READ].swap(Package_Queue[IO_WRITE]);
		LeaveCriticalSection(&writelock);


	}
	temp = Package_Queue[IO_READ].front();
	Package_Queue[IO_READ].pop();
	LeaveCriticalSection(&readlock);
	return temp;
}

