#pragma once
#include "stdafx.h"
#include "LogInPacketProcess.h"

// account Id �� �ߺ����� ���� ���� ������ �����ϴ��� User�� ���� �������� ����.
int PacketProcess::virtualAccountId = 100;


CRITICAL_SECTION lock;

PacketProcess::PacketProcess()
{

	InitializeCriticalSection(&lock); // ����Ŭ���̾�Ʈ ��ȣ�ο��� ����ȭ�� ���� ��ü

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
	// �������� �̸��� Key�� IP, ADDR�� Value�� �����Ͽ� 
	// �л��� ���� XML Read;
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
	//UserManager::getInstance().DeleteUser(packet->accountId); //User ���� �Ҵ� �������� ��

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
		//TODO ����ȭ�� ������!
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
		//TODO ����ȭ�� ������!

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



// Manager ���� ����� ���� �����Ҵ���� Ǯ���ִ� ��
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
����
*/

void PacketProcess::C_REQ_MOVE(Session* session, Packet* rowPacket) {
	PK_C_REQ_MOVE* packet = (PK_C_REQ_MOVE*)rowPacket;



	PK_S_ANS_MOVE ansPacket;
	ansPacket.userNumber = packet->userNumber;
	ansPacket.pos_X = packet->pos_X;
	ansPacket.pos_Y = packet->pos_Y;
	ansPacket.direction = packet->direction;

	//TODO :
	// �ʿ����� ������ SRW�� �Ѵٸ� ���? 
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
		SendPacket���ο��� printf �Ǵ� thread_sleep�� ���� ���� ���� �ʾ��� ���� �ӵ����̴� �ѷ��ϴ�.

		������ printf , thread_sleep�� ���� �ʾ��� ���� ������ Ŭ���̾�Ʈ ������ ������ �������� �������� ������ �߻��Ѵ�.

		1. ���������ΰ� => X;

		2. �ڵ� ���� ������ �ӵ��� �����غ��� �� stack overflow���� ���ϴ� �ʱ�ȭ���� ���� �������� �������� ���� ������ �ð��� �ʾ��� �� �ִٰ� �� �κ� => X


		�񵿱� Send ��û �� IOCP �� Send ó������ �ҿ�ð�
		case No Waiting :  : 1.54264e + 09
		case Waiting: 1.54264e + 09
		�񵿱� Send ��û �ҿ�ð�
		case No Waiting :
		case Waiting: Waiting �� �ɾ��� ���� Ȯ���� ������.

		�� �� ���� Case�� Send ��û�� ó�� �ð��� ���� ���� �� �Ȱ��� ��,

		���̳��� �κ��� Session�� ���� IO ó���� ����ϴ� ContentsProcess�� ���� ������ ���ð���

		No Waiting < Waiting ��� ��  ==> ������ �� ����� ���� ContentProcess ó�� �������� Session->SendPacket ���� ������ �ƴ� �ٸ� ��������

		Sleep �Ǵ� Waiting ������ �ƹ��� ȿ���� ������ �ʾ���.


		�� -> Ŭ���̾�Ʈ ����...?

		=> Ȯ���غ��� ������ ó���ӵ� ������� ���� Ŭ���̾�Ʈ ��Ŷ�� ���ĵ����� ������
		��Ŷ�� ���ĵ����鼭 Ŭ���̾�Ʈ ó���ӵ��� ������ ��ġ�� ����.. ���սô�..

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
