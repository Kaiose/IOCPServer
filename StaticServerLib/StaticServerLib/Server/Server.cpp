#include "stdafx.h"
#include "Server.h"

/*
	TODO::
		Socket Pool 관리에 대한 것은 포함하지 않았음.

*/
Server::Server()
{
	memset(this, NULL, sizeof(*this));
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	status = START;
	

	Initialize();
}

Server::Server(ContentsProcess* contentsProcess)  {

	className = "Server";
	
	sessionCount = 0;

	this->contentsProcess = contentsProcess;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	status = START;

	//lock = std::unique_lock<std::recursive_mutex>(mutex, std::defer_lock); 
	/*
		TODO:	적합한 더미클라이언트 제작후 테스트 요망
	*/

	InitializeCriticalSection(&lock);
	ReadXML();
	Initialize();

	ContentsProcess::serverIOCPHandle = hIOCP;

	Connect();
}

Server::~Server()
{
	status = STOP;
	WSACleanup();
}

void Server::ReadXML() {

	TiXmlDocument config;
	config.LoadFile("config.xml");

	TiXmlElement* server = config.FirstChildElement("Server")->FirstChildElement("CurrentServer");
	if (!server) {
		Log("config.xml is not exist!\n");
		return;
	}
	TiXmlElement* Name = server->FirstChildElement("NAME");

	TiXmlElement* Number = server->FirstChildElement("NUMBER");
	TiXmlElement* IP = server->FirstChildElement("IP");
	TiXmlElement* Port = server->FirstChildElement("Port");
	strcpy_s(serverName, Name->GetText());
	strcpy_s(ip, IP->GetText());
	port = atoi(Port->GetText());
	terminalManager.ServerNumber = atoi(Number->GetText());
	terminalManager.SetServerNumber(terminalManager.ServerNumber);
	//strcpy_s(terminalManager.serverName, Name->GetText());
	terminalManager.remove(serverName);
	return;
}

void Server::Initialize() {
	

	Create_Iocp_Handle();
	Listen();
	Create_Iocp_ThreadPool();
	//Accept();
	Create_Resources();
	
	
	return;

}

void Server::Create_Resources() {
	
	for (int i = 0; i < RESOURCE_COUNT; i++) {

		//다른 부분의 동적할당도 미리 할당 해놓을 수 있겠지만.
		// 이 부분은 단순히 동적할당이 아닌 HANDLE을 통해 접근하는
		// SOCKET에 대한 Pool을 위한 것임.
		// 다른 메모리에 대한 것은 나중에 Google의 tcmalloc 으로 메모리 풀 lib을 사용하면 됨
		SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		/*
		LINGER ling = { 0, };
		ling.l_onoff = 1;
		ling.l_linger = 0;

		setsockopt(sock, SOL_SOCKET, SO_LINGER, (CHAR*)&ling, sizeof(ling));
		*/
		Session* session = new Session(sock);

		if (!CreateIoCompletionPort((HANDLE)session->socket, hIOCP, (ULONG_PTR)session, 0)) {
			printf("Server::OnAccept : CreateIOCompletionPort Failed : %d socketNumber : %d\n", WSAGetLastError(), (int)session->socket);
			//return false;
			//session->SetType();
			//session->RecvStanby();
			return;
		}




		Prepare_Accept(session);
	}


}

void Server::Release_Resources() {
	if (Unused_SessionPool.size() < MIN) {
		printf("Release Failed !!  Unused_SessionPool Size : %zd \n ", Unused_SessionPool.size());
		return;
	}

	EnterCriticalSection(&lock);

	//mutex.lock();
//std::lock_guard<std::recursive_mutex> lock(mutex);
	session_map_it it = Unused_SessionPool.begin();
	
	int count = (int)Unused_SessionPool.size()/2;
	while (count > 0) {
		Session* session = it->second;
		session_map_it next_it = it;
		++next_it;
		Unused_SessionPool.erase(it);
		it = next_it;
		count--;
		SAFE_DELETE(session);
	}

	printf("Release Success !! Unused_SessionPool Size : %zd \n", Unused_SessionPool.size());

	LeaveCriticalSection(&lock);
	//mutex.unlock();
}

void Server::Listen() {
	lstnSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (lstnSocket == INVALID_SOCKET) {
		printf("lstnSocket Error");
		return;
	}

	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(port); // port
	//inet_pton(AF_INET, ip, &(serverAddr.sin_addr)); // ip;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	//serverAddr.sin_addr.s_addr = inet_addr(ip);
	int addrlen = sizeof(serverAddr);
	int ret = bind(lstnSocket, (struct sockaddr*)&serverAddr, addrlen);
	if (ret == SOCKET_ERROR) {
		printf("bind error");
		return;
	}

	ret = listen(lstnSocket, SOMAXCONN);
	if (ret == SOCKET_ERROR) {
		printf("listen failed");
		return;
	}

	
	CreateIoCompletionPort((HANDLE)lstnSocket, hIOCP, NULL, 0);

}


void Server::Accept() {
	
	new std::thread([&]() {
		while (true) {
			SOCKADDR_IN clientAddr;
			clientAddr.sin_family = PF_INET;
			int addrlen = sizeof(SOCKADDR_IN);

			SOCKET sock = accept(lstnSocket, (struct sockaddr*)&clientAddr, &addrlen);
			if (sock == INVALID_SOCKET) {
				printf("accept error : %d", WSAGetLastError());
			}
			Session* session = new Session(sock);

			OnAccept(session);
		}
	});
}

void Server::Prepare_Accept(Session* session) {
	LPFN_ACCEPTEX pfnAcceptEx = (LPFN_ACCEPTEX)GetSockExtAPI(lstnSocket, WSAID_ACCEPTEX);
	session->Initialize();
	session->ioData[IO_READ].Initialize();
	session->ioData[IO_WRITE].Initialize();
	char* buf = session->ioData[IO_READ].buffer;
	
	
	EnterCriticalSection(&lock);
	//mutex.lock();
	//session->setId(sessionCount++);
	
	//mutex.unlock();
	
	if (pfnAcceptEx(lstnSocket, session->socket, buf, 0,
		sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
		NULL, (LPOVERLAPPED)&session->ioData[IO_READ]) == FALSE) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			printf(" AcceptEx Failed : %d\n", WSAGetLastError());

			SAFE_DELETE(session);
			LeaveCriticalSection(&lock);
			return;
		}
	}

	
	Unused_SessionPool.insert(std::make_pair(session->socket, session));
	LeaveCriticalSection(&lock);
}  

void Server::Create_Iocp_Handle() {
	hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hIOCP == NULL) {
		printf("Ceate IOCP HANDLE Failed");
		return;
	}
	return;
}

void Server::Create_Iocp_ThreadPool() {

	//Log("Hardware Concurrency : %d", std::thread::hardware_concurrency());


	for (int i = 0; i < THREAD_COUNT; i++) {
		hThread[i] = std::thread{ &Server::Run, this };
	}
}

static int count = 1;
void Server::Run() {
	
	DWORD Bytes = 0;
	IO_Data* ioData = nullptr;
	Session* session = nullptr;
	Packet* packet = nullptr;
	int byte = 0;
	while (true) {
		try {
		//	Log("Server Thread id : %d\n", GetCurrentThreadId());
 			if (!GetQueuedCompletionStatus(hIOCP, &Bytes, (PULONG_PTR)&session, (LPOVERLAPPED*)&ioData, INFINITE)) {
				Log("GQCS Error %d" , WSAGetLastError());
				throw session;
			}
			/*
			if (Bytes == -1) {
				//Log("Resource_ Recycle executing !");
				//SAFE_DELETE(session);
				//Resource_Recycle(session); 

			
				if (session != nullptr)
					delete session;
				session = nullptr;
				continue;
			}
			*/
			

			byte += Bytes;

			session_map_it it;// = Unused_SessionPool.find(int(ioData->session->socket));
			switch (ioData->ioType) {
			case IO_READ:
				//ZeroByteRecv는 PageLocking에 대해 이점을 가져갈 수도 있겠지만
				//짧은 패킷을 무수히 많이 주고 받는 게임서버에서
				//한 패킷에 대한 비동기 요청을 두번 한다고 생각했을 때
				//그것은 이점으로 유리할 수 있는가?
		/*		if (Bytes == 0) {
					if (session == nullptr) {
						printf("Session is Null.. \n");
						continue;
					}

//					printf("Complete ZeroByte Receive..\n");
					session->RecvStanby();
					continue;
				}
		*/	
			
				if (Bytes == 0) {
					Log("receive 0 byte\n");
					continue;
				}

				
				session->Recv(Bytes);
				packet = nullptr;
				while ((packet = session->GetPacket()) != nullptr) {
					contentsProcess->AddPackage(Packaging(session, packet));
				}
				session->RecvStanby();
				//session->ZeroByteReceive();
				break;
			
			case IO_WRITE:
				

				std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;

			//	std::cout << "수행 시간 : " << sec.count() << std::endl;
				//memset(ioData->buffer, NULL, BUF_SIZE);
				break;
			
		
			case IO_ACCEPT: // session 내부 두 개의 ov가 type이 IO_ACCEPT로 되있다.
			
				if (!OnAccept(ioData->session)) {
					break;
				}
				GetAddress(ioData->session);

				// Pool Managed
				EnterCriticalSection(&lock);
				//	mutex.lock();

				it = Unused_SessionPool.find(int(ioData->session->socket));
				if (it != Unused_SessionPool.end()) {

					Using_SessionPool.insert(*it);

					Unused_SessionPool.erase(it);
					Log("IO_ACCEPT socket Number : %d, sessionId : %d \n", ioData->session->socket, ioData->session->accountId);


				}
				else
					Log("cant find socket");

			

				if (Unused_SessionPool.size() <= 200)
					Create_Resources();
				LeaveCriticalSection(&lock);


				//ShowPool();

				break;
			default:
				Log("Do not have any case! \n");
			}
		}catch (Session* session) {
			// 64 - ERROR_NETNAME_DELETED // 클라 강제종료시
			if (WSAGetLastError() == ERROR_NETNAME_DELETED) {

				ExceptionProcss(session);

			}
			else if (WSAGetLastError() == WSA_OPERATION_ABORTED) {
				printf("WSA_OPERATION_ABORTED \n");
			}
			else {
				Log("GQCS  Last Error : %d\n", WSAGetLastError());
			}

			// 995 - OVERLAPPED 연산 중지시.. ->  WSA_OPERATION_ABORTED

		}
	}
	printf("GQCS CLOSE");
}


void Server::ExceptionProcss(Session* session) {
	__try {
		Log("Server ExceptionProcess!!");
		int accountId = session->accountId;
		User* user = userManager.GetUser(accountId);
		if (user != nullptr) {
			int roomNumber = user->GetRoomNumber();
			int userNumber = user->GetUserNumber();
			Room* room = roomManager.GetRoom(roomNumber);
			userManager.DeleteUser(accountId);

			if (room != nullptr)
				room->DeleteUser(userNumber);

			delete session;
			
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
			Log("Server Access Violation!!!  Exception Code : %ld\n", GetExceptionCode());

	}
}

Package* Server::Packaging(Session* session, Packet* packet) {
	Package* package = new Package(session, packet);
	return package;
}


void Server::Resource_Recycle(Session* session) {

	LPFN_DISCONNECTEX pFnDisconnect = (LPFN_DISCONNECTEX)GetSockExtAPI(session->socket, WSAID_DISCONNECTEX);
	
	if (pFnDisconnect(session->socket, NULL, TF_REUSE_SOCKET, 0)) {
		Log("Session Disconnect Request Success ==> %s , socket : %d accountId : %d \n", session->ip, session->socket,session->accountId);
	//	UserManager::getInstance().DeleteUser(session->accountId); //User 동적 할당 해제까지 함

	}
	else {
		Log("Session Disconnect Request Failed ==> %s , socket : %d accountId : %d \n", session->ip, session->socket, session->accountId);
	}



	/*
	https://rurury.tistory.com/entry/DynamicLink-Library-Best-Practices 
	의도치 않은 락 에러 
	*/
	EnterCriticalSection(&lock);
	//mutex.lock();

	session->Initialize();
	session->ioData[IO_READ].Initialize();
	session->ioData[IO_WRITE].Initialize();
	Unused_SessionPool.insert(std::make_pair(session->socket, session));
	Using_SessionPool.erase((int)session->socket);

	LPFN_ACCEPTEX pfnAcceptEx = (LPFN_ACCEPTEX)GetSockExtAPI(lstnSocket, WSAID_ACCEPTEX);
	char* buf = session->ioData[IO_READ].buffer;
	if (pfnAcceptEx(lstnSocket, session->socket, buf, 0,
		sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
		NULL, (LPOVERLAPPED)&session->ioData[IO_READ]) == FALSE) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			printf(" AcceptEx Failed : %d   socketNumber accountId : %d \n", WSAGetLastError(),  session->accountId);
			
			LeaveCriticalSection(&lock);
			return;
		}
	}
	
	
	LeaveCriticalSection(&lock);

}

void Server::Connect() {
	terminalManager.remove(ip, port);
	terminalManager.multi_Connect(hIOCP,serverName, ip, port);
}

bool Server::OnAccept(Session* session) {
	
	
	session->SetType();
	session->RecvStanby();
	//session->ZeroByteReceive();
	return true;
}

void Server::ShowPool() {

	printf("Using Pool Count : %zd , Unused Pool Count %zd \n", Using_SessionPool.size(), Unused_SessionPool.size());

}


void Server::GetAddress(Session* session) {

	LPFN_GETACCEPTEXSOCKADDRS pfnGetAcceptSOCKADDRS = (LPFN_GETACCEPTEXSOCKADDRS)GetSockExtAPI(session->socket,WSAID_GETACCEPTEXSOCKADDRS);

	SOCKADDR* localAddr, *remoteAddr;
	int localLen, remoteLen;


	pfnGetAcceptSOCKADDRS(session->ioData[IO_READ].buffer, 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
		&localAddr, &localLen,
		&remoteAddr, &remoteLen);
	SOCKADDR_IN* localAddr_in, *remoteAddr_in;
	localAddr_in = (SOCKADDR_IN*)localAddr;
	remoteAddr_in = (SOCKADDR_IN*)remoteAddr;
	char localBuf[20] = { 0, };
	char remoteBuf[20] = { 0, };
	inet_ntop(AF_INET, &localAddr_in->sin_addr, localBuf, 20);
	inet_ntop(AF_INET, &remoteAddr_in->sin_addr, remoteBuf, 20);
	Log("Local Address : %s   Remote Address : %s", localBuf, remoteBuf);
	memcpy(session->ip, localBuf, NAME_SIZE);

}
