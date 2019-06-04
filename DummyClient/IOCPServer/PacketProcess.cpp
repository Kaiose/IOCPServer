#pragma once
#include "stdafx.h"
#include "PacketProcess.h"


HANDLE ContentsProcess::serverIOCPHandle = NULL;

// account Id 를 중복으로 인해 많은 세션이 접속하더라도 User의 수가 증가하지 않음.
int PacketProcess::virtualAccountId = 100;



CRITICAL_SECTION lock;

PacketProcess::PacketProcess()
{
	InitializeCriticalSection(&lock); // 더미클라이언트 번호부여시 동기화를 위한 객체
#define INSERT_PACKET_PROCESS(type) PacketFuncTable.insert(std::make_pair(E_##type, &PacketProcess::##type))
	INSERT_PACKET_PROCESS(C_REQ_EXIT);
	INSERT_PACKET_PROCESS(C_REQ_SIGNIN);
	INSERT_PACKET_PROCESS(C_REQ_SIGNUP);
	INSERT_PACKET_PROCESS(C_REQ_CONNECT);
	INSERT_PACKET_PROCESS(C_REQ_CONNECT_ROOM);
	INSERT_PACKET_PROCESS(C_NOTIFY_CONNECTED_ROOM);
	INSERT_PACKET_PROCESS(C_REQ_CHAT);
	INSERT_PACKET_PROCESS(C_REQ_MOVE);
	INSERT_PACKET_PROCESS(C_REQ_SHOT);
	INSERT_PACKET_PROCESS(C_REQ_EXIT_ROOM);
	INSERT_PACKET_PROCESS(S_ANS_CONNECT);
	INSERT_PACKET_PROCESS(S_ANS_CONNECT_ROOM);
	INSERT_PACKET_PROCESS(S_NOTIFY_USER_DATA);
	INSERT_PACKET_PROCESS(S_NOTIFY_OTHER_CLIENT);
	INSERT_PACKET_PROCESS(S_ANS_MOVE);
	INSERT_PACKET_PROCESS(S_ANS_CHAT); 
	INSERT_PACKET_PROCESS(S_ANS_SHOT);
	INSERT_PACKET_PROCESS(S_ANS_EXIT);

	INSERT_PACKET_PROCESS(S_NOTIFY_SERVER);
	className = "PacketProcess";


	for (int i = 0; i < 3; i++) {
		Room* room = new Room();
		room->SetRoomNumber(i + 1);
		RoomManager::getInstance().AddRoom(room);
	}
	
	// 서버들의 이름을 Key로 IP, ADDR을 Value로 관리하여 
	// 분산을 위한 XML Read;
	ReadXML();

}

PacketProcess::~PacketProcess() {
	DeleteCriticalSection(&lock);
}


void PacketProcess::ReadXML() {
	TiXmlDocument config;
	config.LoadFile("config.xml");
	TiXmlElement* server = config.FirstChildElement("Server");
	if (!server) {
		Log("config.xml is not exist!\n");
		return;
	}
	TiXmlElement* ChildServer, *IP, *Port;
	ChildServer = server->FirstChildElement();
	while (true) {
		IP = ChildServer->FirstChildElement("IP");
		Port = ChildServer->FirstChildElement("Port");

		ServerAddr* serverAddr = new ServerAddr();
		strcpy_s(serverAddr->ip, IP->GetText());
		serverAddr->port = atoi(Port->GetText());
		ServerAddrTable.insert(std::make_pair(ChildServer->ValueTStr().c_str(), serverAddr));

		if ((ChildServer = ChildServer->NextSiblingElement()) == 0)
			break;
	}
}


void PacketProcess::C_REQ_EXIT(Session* session, Packet* rowPacket) {
	PK_C_REQ_EXIT* packet = (PK_C_REQ_EXIT*)rowPacket;

	if (packet->roomNumber != -1) {
		Room* room = RoomManager::getInstance().GetRoom(packet->roomNumber);
		User* user = room->GetUser(packet->userNumber);

		std::unordered_map<int, User*>::iterator it;

		PK_S_ANS_EXIT ansPacket;
		ansPacket.accountId = packet->userNumber;

		Session* userSession = nullptr;
		for (it = room->GetMap().begin(); it != room->GetMap().end(); it++) {
			if (it->second == user) {
				continue;
			}


			userSession = it->second->GetSession();
			userSession->SendPacket(&ansPacket);

		}

		room->DeleteUser(packet->userNumber);
		
	}
	UserManager::getInstance().DeleteUser(packet->accountId); //User 동적 할당 해제까지 함

	PostQueuedCompletionStatus(serverIOCPHandle, -1, (ULONG_PTR)session, NULL);
	
	printf("C_REQ_EXIT Packet\n");


	delete packet;
	/*
	TODO : Find User , Delete Session;
	*/

	
}


void PacketProcess::C_REQ_SIGNIN(Session* session, Packet* rowPacket) {

	PK_C_REQ_SIGNIN* packet = (PK_C_REQ_SIGNIN*)rowPacket;

	int userId = database.Match(packet->name, packet->password);

	if (userId > -1) {
		PK_S_NOTIFY_SERVER ansPacket;
		ServerAddr* serverAddr = ServerAddrTable.find("GameServer")->second;
		memcpy(ansPacket.ip, serverAddr->ip, NAME_SIZE);
		EnterCriticalSection(&lock);
		ansPacket.accountId = PacketProcess::virtualAccountId++;
		LeaveCriticalSection(&lock);
		//TODO 동기화를 해주자!
		ansPacket.port = serverAddr->port;

		session->SendPacket(&ansPacket);

	}
	else {
		PK_S_ANS_SIGNIN ansPacket;
		ansPacket.result = false;
		session->SendPacket(&ansPacket);

	}

	delete packet;
}


void PacketProcess::C_REQ_SIGNUP(Session* session, Packet* rowPacket) {
	PK_C_REQ_SIGNUP* packet = (PK_C_REQ_SIGNUP*)rowPacket;

	bool result = database.Insert(packet->name, packet->password);

	PK_S_ANS_SIGNUP ansPacket;
	ansPacket.result = result;

	session->SendPacket(&ansPacket);

	delete packet;
}



void PacketProcess::C_REQ_CONNECT(Session* session, Packet* rowPacket) {

	PK_C_REQ_CONNECT* packet = (PK_C_REQ_CONNECT*)rowPacket;

	//User* user = new User(session,packet->accountId, packet->name);
	
	printf("Create User ! .. accountId : %d , Name : %s \n", packet->accountId, packet->name);

	
	User* user = new User(session, packet->accountId, packet->name);


	UserManager::getInstance().AddUser(user);

	PK_S_ANS_CONNECT ansPacket;
	session->SendPacket(&ansPacket);
	delete packet;
}



// Manager 에서 지우는 것은 동적할당까지 풀어주는 것
void PacketProcess::C_REQ_CONNECT_ROOM(Session* session, Packet* rowPacket) {

	PK_C_REQ_CONNECT_ROOM* packet = (PK_C_REQ_CONNECT_ROOM*)rowPacket;

	User* user = UserManager::getInstance().GetUser(packet->accountId);
	Room* room = RoomManager::getInstance().GetRoom(packet->roomNumber);
	
	user->SetRoomNumber(room->GetRoomNumber());
	int userNumber = room->AddUser(user);
	user->SetUserNumber(userNumber);
	
	PK_S_ANS_CONNECT_ROOM ansPacket;
	ansPacket.userNumber = userNumber;
	
	session->SendPacket(&ansPacket);

	
	delete packet;
}



void PacketProcess::C_NOTIFY_CONNECTED_ROOM(Session* session, Packet* rowPacket) {
	PK_C_NOTIFY_CONNECTED_ROOM* packet = (PK_C_NOTIFY_CONNECTED_ROOM*)rowPacket;


	Room* room = RoomManager::getInstance().GetRoom(packet->roomNumber);
	User* user = room->GetUser(packet->userNumber);

	std::unordered_map<int, User*> userMap = room->GetMap();
	std::unordered_map<int, User*>::iterator it;


	PK_S_NOTIFY_USER_DATA ansPacket;
	user->GetPositition(ansPacket.pos_X, ansPacket.pos_Y);
	ansPacket.userNumber = packet->userNumber;

	session->SendPacket(&ansPacket);
	
	for (it = userMap.begin(); it != userMap.end(); it++) {
		if (it->second == user) {

			continue;
		}
		PK_S_NOTIFY_OTHER_CLIENT to_User;
		PK_S_NOTIFY_OTHER_CLIENT to_Other_User;
		to_Other_User.userNumber = packet->userNumber;
		User* Enemy = it->second;
		to_User.userNumber = Enemy->GetUserNumber();
		user->GetPositition(to_Other_User.pos_X, to_Other_User.pos_Y);

		Enemy->GetPositition(to_User.pos_X, to_User.pos_Y);


		session->SendPacket(&to_User);

	
		Enemy->GetSession()->SendPacket(&to_Other_User);
	}

	delete packet;

}


void PacketProcess::C_REQ_MOVE(Session* session, Packet* rowPacket) {
	PK_C_REQ_MOVE* packet = (PK_C_REQ_MOVE*)rowPacket;



	PK_S_ANS_MOVE ansPacket;
	ansPacket.userNumber = packet->userNumber;
	ansPacket.pos_X = packet->pos_X;
	ansPacket.pos_Y = packet->pos_Y;
	ansPacket.direction = packet->direction;
	

	Room* room = RoomManager::getInstance().GetRoom(packet->roomNumber);
	User* user = room->GetUser(packet->userNumber);
	user->SetPosition(packet->pos_X, packet->pos_Y);


	std::unordered_map<int, User*> userMap = room->GetMap();

	std::unordered_map<int, User*>::iterator it;

	Session* userSession = nullptr;
	for (it = userMap.begin(); it != userMap.end(); it++) {
		if (it->second == user)
			continue;

		userSession = it->second->GetSession();

		

		userSession->SendPacket(&ansPacket);

		

		/*
			SendPacket내부에서 printf 또는 thread_sleep을 했을 때와 하지 않았을 때의 속도차이는 뚜렷하다.

			하지만 printf , thread_sleep을 걸지 않았을 때에 오히려 클라이언트 측에서 유닛의 움직임이 느려지는 현상이 발생한다.

			1. 병목현상인가 => X;
			
			2. 코드 수행 구간의 속도를 측정해봤을 때 stack overflow에서 말하는 초기화되지 않은 페이지에 접근으로 인한 오류로 시간이 늦어질 수 있다고 한 부분 => X
			
			
			비동기 Send 요청 후 IOCP 로 Send 처리까지 소요시간
			case No Waiting :  : 1.54264e + 09 
			case Waiting: 1.54264e + 09
			비동기 Send 요청 소요시간
			case No Waiting : 
			case Waiting: Waiting 을 걸어준 쪽이 확연히 느렸음.

			즉 두 가지 Case는 Send 요청과 처리 시간을 생각 했을 때 똑같은 점,

			차이나는 부분은 Session에 대한 IO 처리를 담당하는 ContentsProcess에 대한 스레드 대기시간이

			No Waiting < Waiting 라는 것  ==> 하지만 이 결과에 대해 ContentProcess 처리 구간에서 Session->SendPacket 내부 스택이 아닌 다른 곳에서의

			Sleep 또는 Waiting 구문은 아무런 효과도 나오지 않았음.


			즉 -> 클라이언트 문제...?

			=> 확인해보니 서버의 처리속도 향상으로 인해 클라이언트 패킷이 뭉쳐들어오기 시작함
			패킷이 뭉쳐들어오면서 클라이언트 처리속도에 영향을 끼치는 듯함.. 낼합시다..
		
		*/

	
	}
	delete packet;
}


void PacketProcess::C_REQ_CHAT(Session* session, Packet* rowPacket) {

	PK_C_REQ_CHAT* packet = (PK_C_REQ_CHAT*)rowPacket;
	
	
	Room* room = RoomManager::getInstance().GetRoom(packet->roomNumber);
	User* user = room->GetUser(packet->userNumber);

	PK_S_ANS_CHAT ansPacket;
	memcpy(ansPacket.name, user->GetName(), NAME_SIZE);
	//ansPacket.chat = packet->chat;
	memcpy(ansPacket.chat, packet->chat, CHAT_SIZE);
	ansPacket.userNumber = packet->userNumber;
	
	std::unordered_map<int, User*>::iterator it;


	Session* userSession = nullptr;
	for (it = room->GetMap().begin(); it != room->GetMap().end(); it++) {
		if (it->second == user) {
			continue;
		}

		printf("chat : %s \n", packet->chat);

		userSession = it->second->GetSession();

		userSession->SendPacket(&ansPacket);

	}
	

	delete rowPacket;

	//return;

}

void PacketProcess::C_REQ_SHOT(Session* session, Packet* rowPacket) {
	PK_C_REQ_SHOT* packet = (PK_C_REQ_SHOT*)rowPacket;
	Room* room = RoomManager::getInstance().GetRoom(packet->roomNumber);
	User* user = room->GetUser(packet->userNumber);

	PK_S_ANS_SHOT ansPacket;
	ansPacket.userNumber = packet->userNumber;
	ansPacket.direction = packet->direction;
	ansPacket.targetNumber = packet->targetNumber;
	std::unordered_map<int, User*>::iterator it;


	Session* userSession = nullptr;
	for (it = room->GetMap().begin(); it != room->GetMap().end(); it++) {
		if (it->second == user) {
			continue;
		}

		userSession = it->second->GetSession();
		userSession->SendPacket(&ansPacket);
	
	}


	delete rowPacket;

}


void PacketProcess::C_REQ_EXIT_ROOM(Session* session, Packet* rowPacket) {
	PK_C_REQ_EXIT_ROOM* packet = (PK_C_REQ_EXIT_ROOM*)rowPacket;


	Room* room = RoomManager::getInstance().GetRoom(packet->roomNumber);
	User* user = room->GetUser(packet->userNumber);

	room->DeleteUser(packet->userNumber);


	std::unordered_map<int, User*> userMap = room->GetMap();
	std::unordered_map<int, User*>::iterator it;


	
	Session* userSession = nullptr;
	for (it = userMap.begin(); it != userMap.end(); it++) {

		PK_S_ANS_EXIT_ROOM ansPacket;
		ansPacket.userNumber = packet->userNumber;

		userSession = it->second->GetSession();
		userSession->SendPacket(&ansPacket);
	}



	delete packet;

}

void PacketProcess::S_ANS_EXIT(Session* session, Packet* rowPacket) {
	PK_S_ANS_EXIT* packet = (PK_S_ANS_EXIT*)rowPacket;


	session->ChangeSocket();

	
	if (!CreateIoCompletionPort((HANDLE)session->socket, serverIOCPHandle, (ULONG_PTR)session, 0)) {
		printf("Session :: CreateIOCompletionPort Failed : %d\n", WSAGetLastError());
	}

	session->ChangeConnect();

	PK_C_REQ_CONNECT ansPacket;
	ansPacket.accountId = packet->accountId;
	memcpy(ansPacket.name, "DummyClient", NAME_SIZE);

	session->SetType();
	session->SendPacket(&ansPacket);
	session->unlock();
	session->RecvStanby();

	SAFE_DELETE(packet);

}

void PacketProcess::S_NOTIFY_SERVER(Session* session, Packet* rowPacket) {
	PK_S_NOTIFY_SERVER* packet = (PK_S_NOTIFY_SERVER*)rowPacket;
	
	session->socket2close = session->socket;
	memcpy(session->nextIp, packet->ip, 20);
	session->nextPort = packet->port;


	session->lock();
	PK_C_REQ_EXIT exitPacket;
	exitPacket.accountId = packet->accountId;
	exitPacket.roomNumber = packet->roomNumber;
	exitPacket.userNumber = session->userNumber;
	session->SendPacket(&exitPacket);

	SAFE_DELETE(packet);
}



void PacketProcess::S_ANS_CONNECT(Session* session, Packet* rowPacket) {

	PK_C_REQ_CONNECT_ROOM ansPacket;
	ansPacket.accountId = session->getId();
	if (session->roomNumber == -1) {
		ansPacket.roomNumber = session->getId() % 3 + 1;
		session->roomNumber = ansPacket.roomNumber;
	}
	else
		ansPacket.roomNumber = session->roomNumber;
	session->SendPacket(&ansPacket);

	Log("Receive S_ANS_CONNECT !! RoomNumber : %d , userNumver : %d \n", session->roomNumber, session->userNumber);
	delete rowPacket;
}


void PacketProcess::S_ANS_CONNECT_ROOM(Session* session, Packet* rowPacket) {

	PK_S_ANS_CONNECT_ROOM* packet = (PK_S_ANS_CONNECT_ROOM*)rowPacket;


	printf("Receive S_ANS_CONNECT_ROOM userNumber : %d \n ", session->userNumber);

	session->userNumber = packet->userNumber;

	PK_C_NOTIFY_CONNECTED_ROOM ansPacket;
	ansPacket.roomNumber = session->roomNumber;
	ansPacket.userNumber = session->userNumber;

	session->SendPacket(&ansPacket);

	delete rowPacket;
}

void PacketProcess::S_NOTIFY_USER_DATA(Session* session, Packet* rowPacket) {
	PK_S_NOTIFY_USER_DATA* packet = (PK_S_NOTIFY_USER_DATA*)rowPacket;
	
	float boundary_top = 2.0 * 10;
	float boundary_bottom = -4.0 * 10;
	float boundary_left = -3.0 * 10;
	float boundary_right = 6.3 * 10;

	int rangeOf_X = int(boundary_right - boundary_left);
	int rangeOf_Y = int(boundary_top - boundary_right);

	float pos_X = float(((rand() % rangeOf_X) + boundary_left) / 10);
	float pos_Y = float(((rand() % rangeOf_Y) + boundary_bottom) / 10);
	printf("S_NOTIFY_USER_DATA : roomNumber : %d \n userNumber : %d \n ",session->roomNumber, session->userNumber);

	session->pos_X = pos_X;
	session->pos_Y = pos_Y;
	start = std::chrono::system_clock::now();
	session->randomMoving();

	delete rowPacket;
}

void PacketProcess::S_NOTIFY_OTHER_CLIENT(Session* session, Packet* rowPacket) {

	delete rowPacket;
}

void PacketProcess::S_ANS_MOVE(Session* session, Packet* rowPacket) {

	delete rowPacket;
}

void PacketProcess::S_ANS_SHOT(Session* session, Packet* packet) {

}

void PacketProcess::S_ANS_CHAT(Session* session, Packet* packet) {

}
