#pragma once
#include "stdafx.h"
#include "LogInPacketProcess.h"

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

	INSERT_PACKET_PROCESS(C_REQ_CROSS_OVER_SERVER);
	INSERT_PACKET_PROCESS(T_ASK_HOLD_ROOM);
	INSERT_PACKET_PROCESS(T_ANS_HOLD_ROOM);
	INSERT_PACKET_PROCESS(T_REQ_CONNECT);

	className = "PacketProcess";
	/*
	for (int i = 0; i < 4; i++) {
	Room* room = new Room();
	room->SetRoomNumber(i + 1);

	RoomManager::getInstance().AddRoom(room);
	}
	*/
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

		if (IP == NULL || Port == NULL)
			break;
		Terminal* terminal = new Terminal();
		terminal->registAddress(IP->GetText(), atoi(Port->GetText()));

		std::string serverName = ChildServer->ValueTStr().c_str();
		if (serverName == "LogInServer")
			terminalManager.LoginTerminal = terminal;
		else if (serverName == "CurrentServer") {

		}
		else {
			terminalManager.insert(ChildServer->ValueTStr().c_str(), terminal);
		}
		if ((ChildServer = ChildServer->NextSiblingElement()) == 0)
			break;
	}

	TiXmlElement* Room, *RoomNumber;

	Room = server->FirstChildElement("Room");
	RoomNumber = Room->FirstChildElement("RoomNumber");

	while (true) {

		if (RoomNumber == NULL) {
			break;
		}
		int roomNumber = atoi(RoomNumber->GetText());
		RoomManager::getInstance().CreateRoom(roomNumber);

		RoomNumber = RoomNumber->NextSiblingElement();

	}

	for (int i = 0; i < terminalManager.TerminalMap.size() * RoomManager::getInstance().Count(); i++) {
		std::queue<Session*>* sessionQueue = new std::queue<Session*>();
		terminalManager.terminalSessions_To_Process.push_back(sessionQueue);
	}
}


void PacketProcess::C_REQ_EXIT(Session* session, Packet* rowPacket) {
	PK_C_REQ_EXIT* packet = (PK_C_REQ_EXIT*)rowPacket;

	//if (packet->roomNumber != -1) {
	//	Room* room = RoomManager::getInstance().GetRoom(packet->roomNumber);
	//	User* user = room->GetUser(packet->userNumber);

	//	std::unordered_map<int, User*>::iterator it;

	//	PK_S_ANS_EXIT ansPacket;
	//	ansPacket.userNumber = packet->userNumber;

	//	Session* userSession = nullptr;
	//	for (it = room->GetMap().begin(); it != room->GetMap().end(); it++) {
	//		if (it->second == user) {
	//			continue;
	//		}

	//		userSession = it->second->GetSession();
	//		userSession->SendPacket(&ansPacket);

	//	}

	//	room->DeleteUser(packet->userNumber);

	//}
	//UserManager::getInstance().DeleteUser(packet->accountId); //User 동적 할당 해제까지 함

	//PostQueuedCompletionStatus(serverIOCPHandle, -1, (ULONG_PTR)session, NULL);

	delete session;

	printf("C_REQ_EXIT Packet\n");


	delete packet;
	/*
	TODO : Find User , Delete Session;
	*/


}


void PacketProcess::C_REQ_SIGNIN(Session* session, Packet* rowPacket) {

	PK_C_REQ_SIGNIN* packet = (PK_C_REQ_SIGNIN*)rowPacket;
	Log("Sign IN\n");

	int userId = database.Match(packet->name, packet->password);

	if (strcmp(packet->name, "master") == 0) {
		userId = 99;
	}

	if (userId > 0) {
		PK_S_NOTIFY_SERVER ansPacket;

		Terminal* terminal = terminalManager.getTerminal("FirstGameServer");
		memcpy(ansPacket.ip, terminal->ip, NAME_SIZE);
		ansPacket.accountId = userId;
		ansPacket.roomNumber = -1;
		session->accountId = userId;
		//TODO 동기화를 해주자!
		ansPacket.port = terminal->port;
		session->SendPacket(&ansPacket);
	}/* if session id DummyClient
	else if (userId == -1) {
		PK_S_NOTIFY_SERVER ansPacket;

		ansPacket.roomNumber = -1;
		session->accountId = PacketProcess::virtualAccountId;
		EnterCriticalSection(&lock);

		ansPacket.accountId = PacketProcess::virtualAccountId++;
		LeaveCriticalSection(&lock);
		//TODO 동기화를 해주자!

		int tempNumber = ansPacket.accountId%terminalManager.TerminalMap.size();
		Terminal* terminal = nullptr;
		switch (tempNumber) {
		case 0:
			terminal = terminalManager.getTerminal("FirstGameServer");
			memcpy(ansPacket.ip, terminal->ip, NAME_SIZE);
			ansPacket.port = terminal->port;
			break;
		case 1:
			terminal = terminalManager.getTerminal("SecondGameServer");
			memcpy(ansPacket.ip, terminal->ip, NAME_SIZE);
			ansPacket.port = terminal->port;
			break;
		case 3:
			terminal = terminalManager.getTerminal("ThirdGameServer");
			memcpy(ansPacket.ip, terminal->ip, NAME_SIZE);
			ansPacket.port = terminal->port;
			break;
		}

		session->SendPacket(&ansPacket);

	}
	*/
	else {
		PK_S_ANS_SIGNIN ansPacket;
		ansPacket.result = false;
		session->SendPacket(&ansPacket);

	}
	
	delete packet;
}


void PacketProcess::C_REQ_SIGNUP(Session* session, Packet* rowPacket) {
	PK_C_REQ_SIGNUP* packet = (PK_C_REQ_SIGNUP*)rowPacket;

	WCHAR name[NAME_SIZE] = { 0, };

	WCHAR password[NAME_SIZE] = { 0, };
	MultiByteToWideChar(CP_ACP, 0, packet->name, -1, name, NAME_SIZE);
	MultiByteToWideChar(CP_ACP, 0, packet->password, -1, password, NAME_SIZE);

	bool result = database.Insert(name, password);

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
	Room* room = RoomManager::getInstance().GetRoom(packet->roomNumber);
	if (room == nullptr) {

		terminalManager.PushSession(packet->roomNumber, session);
		PK_T_ASK_HOLD_ROOM askPacket;
		askPacket.roomNumber = packet->roomNumber;
		printf("multi send Ask Hold Room   number is : %d \n", packet->roomNumber);
		terminalManager.multi_SendPacket(&askPacket);


		delete packet;
		return;
	}


	User* user = UserManager::getInstance().GetUser(packet->accountId);

	user->SetRoomNumber(room->GetRoomNumber());

	int userNumber = room->AddUser(user);

	user->SetUserNumber(userNumber);

	printf("C_REQ_CONNECT_ROOM roomNumber %d ,  userNumber : %d  \n", packet->roomNumber, userNumber);

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

	printf("C_NOTIFY_CONNECTED_ROOM : AccountID : %d , userNumber %d\n", session->accountId, packet->userNumber);

	session->SendPacket(&ansPacket);

	for (it = userMap.begin(); it != userMap.end(); it++) {
		if (it->second == user) {
			// user->GetAccountId() >= 100   if user is Dummy
			continue;

		}



		PK_S_NOTIFY_OTHER_CLIENT to_User;
		User* Enemy = it->second;
		to_User.userNumber = Enemy->GetUserNumber();
		Enemy->GetPositition(to_User.pos_X, to_User.pos_Y);
		printf("PK_S_NOTIFY_OTHER_CLIENT Enemy Number : %d \n ", to_User.userNumber);

		if (user->GetAccountId() < 100) {
			session->SendPacket(&to_User);
		}
		PK_S_NOTIFY_OTHER_CLIENT to_Other_User;
		to_Other_User.userNumber = packet->userNumber;
		user->GetPositition(to_Other_User.pos_X, to_Other_User.pos_Y);



		if (Enemy->GetAccountId() >= 100) // if Enemy is Dummy
			continue;

		Enemy->GetSession()->SendPacket(&to_Other_User);
	}

	user->SetReady();
	delete packet;

}

/*
서버
*/

void PacketProcess::C_REQ_MOVE(Session* session, Packet* rowPacket) {
	PK_C_REQ_MOVE* packet = (PK_C_REQ_MOVE*)rowPacket;



	PK_S_ANS_MOVE ansPacket;
	ansPacket.userNumber = packet->userNumber;
	ansPacket.pos_X = packet->pos_X;
	ansPacket.pos_Y = packet->pos_Y;
	ansPacket.direction = packet->direction;

	//TODO :
	// 맵에대한 접근을 SRW로 한다면 어떨까? 
	Room* room = RoomManager::getInstance().GetRoom(packet->roomNumber);
	User* user = room->GetUser(packet->userNumber);
	user->SetPosition(packet->pos_X, packet->pos_Y);
	user->SetDirection(packet->direction);
	float temp_x;
	float temp_y;

	printf("C_REQ_MOVE roomNumber : %d userNumber : %d , Position x : %f , y : %f \n", packet->roomNumber, packet->userNumber, packet->pos_X, packet->pos_Y);

	std::unordered_map<int, User*> userMap = room->GetMap();

	std::unordered_map<int, User*>::iterator it;

	Session* userSession = nullptr;
	for (it = userMap.begin(); it != userMap.end(); it++) {
		if (it->second == user) {
			continue;
		}


		start = std::chrono::system_clock::now();

		if (it->second->GetAccountId() >= 100)
			continue; // is need when target session is dummy

		if (!it->second->IsReady())
			continue;
		userSession = it->second->GetSession();
		userSession->SendPacket(&ansPacket);

		user->GetPositition(temp_x, temp_y);
		printf("DummySession : %d targetSession : %d \n , DummyPosition x : %f , y : %f", user->GetAccountId(), it->second->GetAccountId()
			, temp_x, temp_y);


		/**
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
	ansPacket.chat = packet->chat;
//	memcpy(ansPacket.chat, packet->chat, CHAT_SIZE);
	ansPacket.userNumber = packet->userNumber;

	std::unordered_map<int, User*>::iterator it;


	Session* userSession = nullptr;
	for (it = room->GetMap().begin(); it != room->GetMap().end(); it++) {
		if (it->second == user) {
			continue;
		}
		ansPacket.stream.offset = 0;

	

		if (it->second->GetAccountId() >= 100)
			continue; // is need when session was dummy

		userSession = it->second->GetSession();
		userSession->SendPacket(&ansPacket);

	}


	delete packet;

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
		ansPacket.stream.offset = 0;
		userSession = it->second->GetSession();
		userSession->SendPacket(&ansPacket);

	}

	delete packet;
	
}

void PacketProcess::C_REQ_EXIT_ROOM(Session* session, Packet* rowPacket) {
	PK_C_REQ_EXIT_ROOM* packet = (PK_C_REQ_EXIT_ROOM*)rowPacket;


	Room* room = RoomManager::getInstance().GetRoom(packet->roomNumber);
	User* user = room->GetUser(packet->userNumber);
	user->ResetReady();
	room->DeleteUser(packet->userNumber);


	std::unordered_map<int, User*> userMap = room->GetMap();
	std::unordered_map<int, User*>::iterator it;



	Session* userSession = nullptr;
	for (it = userMap.begin(); it != userMap.end(); it++) {

		PK_S_ANS_EXIT_ROOM ansPacket;
		ansPacket.userNumber = packet->userNumber;

		if (it->second->GetAccountId() >= 100)
			continue; // is need when session was dummy

		userSession = it->second->GetSession();
		userSession->SendPacket(&ansPacket);
	}

	delete packet;
}

#define ROOMSIZE 9

void PacketProcess::C_REQ_CROSS_OVER_SERVER(Session* session, Packet* rowPacket) {

	PK_C_REQ_CROSS_OVER_SERVER* packet = (PK_C_REQ_CROSS_OVER_SERVER*)rowPacket;

	Room* room = RoomManager::getInstance().GetRoom(packet->roomNumber);
	User* user = room->GetUser(packet->userNumber);

	static int roomInterval = int(std::sqrt(ROOMSIZE));
	int currentRoomNumber = packet->roomNumber;
	int nextRoomNumber = -1;

	if (packet->direction == 0)//TOP
	{
		if (currentRoomNumber > roomInterval) {
			nextRoomNumber = currentRoomNumber - roomInterval;
		}
	}
	else if (packet->direction == 1)//Right
	{
		if ((currentRoomNumber % roomInterval) != 0) {
			nextRoomNumber = currentRoomNumber + 1;
		}
	}
	else if (packet->direction == 2)//Bot
	{
		if (currentRoomNumber <= (ROOMSIZE - roomInterval)) {
			nextRoomNumber = currentRoomNumber + roomInterval;
		}
	}
	else if (packet->direction == roomInterval)//Left
	{
		if ((currentRoomNumber % roomInterval) != 1) {
			nextRoomNumber = currentRoomNumber - 1;
		}
	}

	if (nextRoomNumber == -1) {
		delete packet;
		return;
	}
	Room* nextRoom = RoomManager::getInstance().GetRoom(nextRoomNumber);

	if (nextRoom != nullptr) {
		PK_C_REQ_EXIT_ROOM* firstEchoPacket = new PK_C_REQ_EXIT_ROOM();
		firstEchoPacket->roomNumber = user->GetRoomNumber();
		firstEchoPacket->userNumber = user->GetUserNumber();
		PacketProcess::C_REQ_EXIT_ROOM(session, firstEchoPacket);

		PK_C_REQ_CONNECT_ROOM* secondEchoPacket = new PK_C_REQ_CONNECT_ROOM();
		secondEchoPacket->accountId = user->GetAccountId();
		secondEchoPacket->roomNumber = nextRoomNumber;
		PacketProcess::C_REQ_CONNECT_ROOM(session, secondEchoPacket);

	}
	else {

		PK_T_ASK_HOLD_ROOM askPacket;
		askPacket.roomNumber = nextRoomNumber;

		terminalManager.multi_SendPacket(&askPacket);

	}

	delete packet;



}



void PacketProcess::T_ASK_HOLD_ROOM(Session* session, Packet* rowPacket) {

	PK_T_ASK_HOLD_ROOM* packet = (PK_T_ASK_HOLD_ROOM*)rowPacket;


	Room* room = RoomManager::getInstance().GetRoom(packet->roomNumber);
	if (room != nullptr) {

		PK_T_ANS_HOLD_ROOM ansPacket;
		printf("receiv Ask Hold room : %d\n", packet->roomNumber);
		ansPacket.roomNumber = packet->roomNumber;
		session->SendPacket(&ansPacket);
	}

	delete packet;
}

void PacketProcess::T_ANS_HOLD_ROOM(Session* session, Packet* rowPacket) {
	PK_T_ANS_HOLD_ROOM* packet = (PK_T_ANS_HOLD_ROOM*)rowPacket;

	Terminal* terminal = (Terminal*)session;
	PK_S_NOTIFY_SERVER ansPacket;
	ansPacket.accountId = session->accountId;
	ansPacket.roomNumber = packet->roomNumber;
	memcpy(ansPacket.ip, terminal->ip, 20);
	ansPacket.port = terminal->port;
	printf("receive ans hold room : %d \n", packet->roomNumber);
	terminalManager.SessionProcess(packet->roomNumber, &ansPacket);

	delete packet;
}

void PacketProcess::T_REQ_CONNECT(Session* session, Packet* rowPacket) {
	PK_T_REQ_CONNECT* packet = (PK_T_REQ_CONNECT*)rowPacket;
	const char* serverName = packet->serverName;
	int terminalNumber = packet->terminalNumber;
	const char* ip = packet->ip;
	int port = packet->port;

	printf("Server: %s, terminalNumber: %d , Ip : %s , Port %d  ==>> terminal request connect!\n", serverName, terminalNumber, ip, port);

	session->port = port;
	memcpy(session->ip, packet->ip, NAME_SIZE);


	if (strcmp(serverName, "LogInServer") == 0) {
		Terminal* temp = terminalManager.LoginTerminal;
		terminalManager.LoginTerminal = (Terminal*)session;
		delete temp;
		delete packet;
		return;

	}

	std::unordered_map<std::string, Terminal*>::iterator it = terminalManager.TerminalMap.find(serverName);
	if (it == terminalManager.TerminalMap.end()) {
		Terminal* terminal = (Terminal*)session;
		terminal->terminalNumber = terminalNumber;
		memcpy(terminal->ip, ip, NAME_SIZE);
		terminal->port = port;
		memcpy(terminal->terminalName, serverName, NAME_SIZE);

		terminalManager.TerminalMap.insert(std::make_pair(serverName, terminal));

	}
	else
		terminalManager.TerminalMap.find(serverName)->second = (Terminal*)session;

	delete packet;
}
