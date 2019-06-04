#include "stdafx.h"
#include "Server.h"

/*
	TODO::
		Socket Pool ������ ���� ���� �������� �ʾ���.
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
	virtualNumber = 100;
	this->contentsProcess = contentsProcess;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	status = START;

	/*
		TODO:	������ ����Ŭ���̾�Ʈ ������ �׽�Ʈ ���
	*/

	//InitializeCriticalSection(&lock);
	//ReadXML();
	memcpy(ip, "127.0.0.1",20);
	port = 9900;

	Initialize();

	this->contentsProcess->serverIOCPHandle = hIOCP;
}

Server::~Server()
{
	status = STOP;
	WSACleanup();
}

void Server::ReadXML() {

	TiXmlDocument config;
	config.LoadFile("config.xml");

	TiXmlElement* server = config.FirstChildElement("Server")->FirstChildElement("EchoServer");
	if (!server) {
		Log("config.xml is not exist!\n");
		return;
	}

	TiXmlElement* IP = server->FirstChildElement("IP");
	TiXmlElement* Port = server->FirstChildElement("Port");
	
	strcpy_s(ip, IP->GetText());
	port = atoi(Port->GetText());

	return;
}

void Server::Initialize() {
	

	Create_Iocp_Handle();
	//Listen();
	Create_Iocp_ThreadPool();
	//Accept();
	Create_Resources();
	ConnectingSessions(ip,port);

	return;

}

void Server::Create_Resources() {


	for (int i = 0; i < RESOURCE_COUNT*20; i++) {
		//�ٸ� �κ��� �����Ҵ絵 �̸� �Ҵ� �س��� �� �ְ�����.
		// �� �κ��� �ܼ��� �����Ҵ��� �ƴ� HANDLE�� ���� �����ϴ�
		// SOCKET�� ���� Pool�� ���� ����.
		// �ٸ� �޸𸮿� ���� ���� ���߿� Google�� tcmalloc ���� �޸� Ǯ lib�� ����ϸ� ��
		SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		Session* session = new Session(sock);

		mutex.lock();
		session->setId(sessionCount++);
		Unused_SessionPool.insert(session);
		mutex.unlock();

		//Prepare_Accept(session);
	}
}

void Server::ConnectingSessions(char* ip, int port) {

	session_it it = Unused_SessionPool.begin();

	srand((unsigned)time(NULL));

	for (; it != Unused_SessionPool.end(); it++) {

		
		Session* session = *it;

		session->Connect(ip, port);
		if (!CreateIoCompletionPort((HANDLE)session->socket, hIOCP, (ULONG_PTR)session, 0)) {
			printf("Session :: CreateIOCompletionPort Failed : %d\n", WSAGetLastError());
			//session->RecvStanby();
		}
		
		PK_C_REQ_CONNECT packet;

		packet.accountId = virtualNumber++;
		
		memcpy(packet.name, "DummyClient", NAME_SIZE);
		Log("PK_C_REQ_CONNECT : %d \n" ,packet.accountId);
		OnAccept(session);
		session->setId(packet.accountId);

		session->SendPacket(&packet);
		
		//closesocket(session->socket);

		int sleeptime = rand() % 1000;

		if (sleeptime < 100)
			sleeptime += 100;

	//	Sleep(sleeptime);
		

	}

		
	
	

	

}

void Server::Release_Resources() {
	if (Unused_SessionPool.size() < MIN) {
		printf("Release Failed !!  Unused_SessionPool Size : %zd \n ", Unused_SessionPool.size());
		return;
	}

	session_it it = Unused_SessionPool.begin();
	
	int count = (int)Unused_SessionPool.size()/2;
	while (count > 0) {
		Session* session = *it;
		session_it next_it = it;
		++next_it;
		Unused_SessionPool.erase(it);
		it = next_it;
		count--;
		delete session;
	}

	printf("Release Success !! Unused_SessionPool Size : %zd \n", Unused_SessionPool.size());

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
	inet_pton(AF_INET, ip, &(serverAddr.sin_addr)); // ip;
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
	if (pfnAcceptEx(lstnSocket, session->socket, buf, 0,
		sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
		NULL, (LPOVERLAPPED)&session->ioData[IO_READ]) == FALSE) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			printf(" AcceptEx Failed : %d\n", WSAGetLastError());
			SAFE_DELETE(session);
			return;
		}
	}
//	EnterCriticalSection(&lock);
	mutex.lock();
	session->setId(sessionCount++);
	Unused_SessionPool.insert(session);
	mutex.unlock();
//	LeaveCriticalSection(&lock);
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
	for (int i = 0; i < THREAD_COUNT; i++) {
		hThread[i] = std::thread{ &Server::Run, this };
	}
}

void Server::Run() {
	
	DWORD Bytes = 0;
	IO_Data* ioData = nullptr;
	Session* session = nullptr;
	Packet* packet = nullptr;
	while (true) {
		try {
 			if (!GetQueuedCompletionStatus(hIOCP, &Bytes, (PULONG_PTR)&session, (LPOVERLAPPED*)&ioData, INFINITE)) {
				printf("ioData->type : %d", ioData->ioType);
				throw session;
			}


			//if (Bytes == 0 && session == NULL) {
			//	printf("Request Distribute!!! \n");
			//	/*	
			//		�� ������ ��������.
			//		RoomList���� Room ������ �ٸ������� �ѱ�

			// Accept�� �������� ���� ������ �߻�, �ٸ� Key�� �־���ҵ�
			// if(Bytes == 0 && session == DISTRIBUTE(0x12345L)
			// ���� ��� �л길 ����
				
			//	*/
			//	continue;
			//}

			if (Bytes == -1) {
				Resource_Recycle(session); 
				continue;
			}
			
			switch (ioData->ioType) {
			case IO_READ:
				//ZeroByteRecv�� PageLocking�� ���� ������ ������ ���� �ְ�����
				//ª�� ��Ŷ�� ������ ���� �ְ� �޴� ���Ӽ�������
				//�� ��Ŷ�� ���� �񵿱� ��û�� �ι� �Ѵٰ� �������� ��
				//�װ��� �������� ������ �� �ִ°�?
				if (Bytes == 0) {
					if (session == nullptr) {
						printf("Session is Null.. \n");
						continue;
					}

//					printf("Complete ZeroByte Receive..\n");
					session->RecvStanby();
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
//				std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;
				if (Bytes == 0) {
					Log("IO_WRITE !!! Byte zero");
				}
			//	std::cout << "���� �ð� : " << sec.count() << std::endl;
				//memset(ioData->buffer, NULL, BUF_SIZE);
				break;
			
			case IO_ACCEPT: // session ���� �� ���� ov�� type�� IO_ACCEPT�� ���ִ�.
				OnAccept(ioData->session);
				GetAddress(ioData->session);
				// Pool Managed
				session_it it = Unused_SessionPool.find(ioData->session);
				
				mutex.lock();
				//EnterCriticalSection(&lock);
				Using_SessionPool.insert(*it);
				Unused_SessionPool.erase(it);
				//LeaveCriticalSection(&lock);
				mutex.unlock();
				if (Unused_SessionPool.size() <= 0)
					Create_Resources();
				ShowPool();
				break;
			}
		}catch (Session* session) {
			printf("GQCS Error : %d IP : %s  Port : %d UserNumber: %d\n",  WSAGetLastError(), session->ip, session->port, session->userNumber);
			// 64 - ERROR_NETNAME_DELETED // Ŭ�� ���������
			if (WSAGetLastError() == WSA_OPERATION_ABORTED) {
				printf("WSA_OPERATION_ABORTED \n");
			}
			if (WSAGetLastError() == ERROR_NETNAME_DELETED) {
				//session->~Session();
				
			}
			// 995 - OVERLAPPED ���� ������.. ->  WSA_OPERATION_ABORTED
		}
	}
	printf("GQCS CLOSE");
}


Package* Server::Packaging(Session* session, Packet* packet) {
	Package* package = new Package(session, packet);
	return package;
}


void Server::Resource_Recycle(Session* session) {
	LPFN_DISCONNECTEX pFnDisconnect = (LPFN_DISCONNECTEX)GetSockExtAPI(session->socket, WSAID_DISCONNECTEX);
	Log("Session Disconnect ==> %s", session->ip);
	pFnDisconnect(session->socket, NULL, TF_REUSE_SOCKET, 0);
	Prepare_Accept(session);
	mutex.lock();
	//EnterCriticalSection(&lock);
	Using_SessionPool.erase(session);
	mutex.unlock();
//	LeaveCriticalSection(&lock);
//	Unused_SessionPool.insert(session);
}


void Server::OnAccept(Session* session) {
	session->SetType();

	session->RecvStanby();
	//session->ZeroByteReceive();
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