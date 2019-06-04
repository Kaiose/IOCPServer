#pragma once
#include "stdafx.h"
#include "GamePacketProcess.h"

//HANDLE ContentsProcess::serverIOCPHandle = NULL;



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

	INSERT_PACKET_PROCESS(T_ASK_LOAD);
	INSERT_PACKET_PROCESS(T_ANS_LOAD);

	INSERT_PACKET_PROCESS(T_NOTIFY_ROOM);
	INSERT_PACKET_PROCESS(T_NOTIFY_ROOM_GENERATE_COMPLETE);
	INSERT_PACKET_PROCESS(T_NOTIFY_USER);
	INSERT_PACKET_PROCESS(T_ANS_USER);




	className = "PacketProcess";
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

		roomManager.CreateRoom(roomNumber);

		RoomNumber = RoomNumber->NextSiblingElement();

	}

	for (int i = 0; i < terminalManager.TerminalMap.size() * roomManager.Count(); i++) {
		std::queue<Session*>* sessionQueue = new std::queue<Session*>();
		terminalManager.terminalSessions_To_Process.push_back(sessionQueue);
	}
}

void PacketProcess::C_REQ_EXIT(Session* session, Packet* rowPacket) {
	PK_C_REQ_EXIT* packet = (PK_C_REQ_EXIT*)rowPacket;
	EXCLUSIVE_LOCK(packet->roomNumber);
	

		if (packet->roomNumber != -1) {

			Room* room = roomManager.GetRoom(packet->roomNumber);
			Log("GET C_REQ_EXIT !! RoomNumber : %d , UserNumber : %d \n", packet->roomNumber, packet->userNumber);
			if (room != nullptr) {

				User* user = room->GetUser(packet->userNumber);

				std::unordered_map<int, User*>::iterator it;

				Session* userSession = nullptr;
				for (it = room->GetMap().begin(); it != room->GetMap().end(); it++) {
					if (it->second == user) {
						continue;
					}

					if (it->second->GetAccountId() >= 100)
						continue;

					PK_S_ANS_EXIT ansPacket2;
					ansPacket2.accountId = packet->userNumber;
					userSession = it->second->GetSession();
					userSession->SendPacket(&ansPacket2);
				}

				room->DeleteUser(packet->userNumber);
			}
		}

		UserManager::getInstance().DeleteUser(packet->accountId); //User ���� �Ҵ� �������� ��

																  /*if (!PostQueuedCompletionStatus(serverIOCPHandle, -1, (ULONG_PTR)session, NULL)) {
																  Log("PoseQueueComlpetionStatus Failed .... ErrCode : %d \n", WSAGetLastError());
																  }
																  */

		if (serverIOCPHandle == INVALID_HANDLE_VALUE || serverIOCPHandle == nullptr) {
			Log("serverIOCPHandle is INVALID_HANDLE_VALUE \n");
		}



	

	delete session;
	delete packet;

}





void PacketProcess::C_REQ_SIGNIN(Session* session, Packet* rowPacket) {

	PK_C_REQ_SIGNIN* packet = (PK_C_REQ_SIGNIN*)rowPacket;

	int userId = database.Match(packet->name, packet->password);

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
	}// if session id DummyClient

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

	User* isExistUser = userManager.GetUser(packet->accountId);
	if (isExistUser == nullptr) {
		printf(" Create User ! .. accountId : %d , Name : %s \n", packet->accountId, packet->name);

		User* user = new User(session, packet->accountId, packet->name);
		session->accountId = packet->accountId;

		UserManager::getInstance().AddUser(user);
	}
	else {
		session->accountId = packet->accountId;
		isExistUser->SetSession(session);
		Log("IsExistUser !! accountId : %d, roomNumber %d , userNumber : %d",
			session->accountId, isExistUser->GetRoomNumber(), isExistUser->GetUserNumber());

		Room* room = roomManager.GetRoom(isExistUser->GetRoomNumber());

	}
	PK_S_ANS_CONNECT ansPacket;

	session->SendPacket(&ansPacket);

	delete packet;
}



// Manager ���� ����� ���� �����Ҵ���� Ǯ���ִ� ��
void PacketProcess::C_REQ_CONNECT_ROOM(Session* session, Packet* rowPacket) {
	PK_C_REQ_CONNECT_ROOM* packet = (PK_C_REQ_CONNECT_ROOM*)rowPacket;
	SHARED_LOCK(packet->roomNumber);
	Room* room = RoomManager::getInstance().GetRoom(packet->roomNumber);
	if (room == nullptr) { // room �� ������ �ִ��� ������ �ι� Ȯ���ϰ� �Ǵµ�
						   // C_REQ_COONECT_ROOM ���� Ȯ�� �ϴ� ������ ó�� �� Ŭ���� �� ���θ� Ȯ��
						   // C_REQ_CROSS_OVER_ROOM �� �� ���ο��� �̵���

		terminalManager.PushSession(packet->roomNumber, session);
		PK_T_ASK_HOLD_ROOM askPacket;
		askPacket.roomNumber = packet->roomNumber;
		printf("multi send Ask Hold Room number is : %d \n", packet->roomNumber);
		terminalManager.multi_SendPacket(&askPacket);

		delete packet;
		return;
	}


	User* user = UserManager::getInstance().GetUser(packet->accountId);
	if (user == nullptr) {
		printf("C_REQ_CONNECT_ROOM user is not exist userAccountId : %d  \n", user->GetAccountId());
		delete packet;
		return;
	}

	user->SetRoomNumber(room->GetRoomNumber());


	int userNumber = user->GetUserNumber();


	// �̰� �س������� ���� �̵��Ҷ� ������ ������ ���� �뿡 ���� �ʱ����ؼ� �ϴ°ǵ�
	// �����̵��� �ƴ� ����̰��� �̵��ÿ� 
	/*
	ex) ��1 �� 133�� ��2�� 134�������� ������ �ִٰ�������
	��1�� 133�� ������ ��2�� �ű�ٰ� ����.
	�׷���  �� 1�� 133�� ������ ��2�� 133�� ������ �ٸ� �����ӿ��� �ұ��ϰ�
	133�� UserNumber�� �����Ƿ�  ��2�� ������ ������.

	������ ���۾��� ���������� �Ϸ�Ű ���ؼ���

	C_REQ_EXIT �۾��� CONNECTED �۾� ���̿��� room �� ������ �߰��ϴ� ���� ���

	*/
	if (room->GetUser(user->GetUserNumber()) == nullptr) {
		userNumber = room->AddUser(user);
		user->SetUserNumber(userNumber);
	}

	printf("C_REQ_CONNECT_ROOM roomNumber %d ,  userNumber : %d ` , account Id : %d \n", packet->roomNumber, userNumber, packet->accountId);

	PK_S_ANS_CONNECT_ROOM ansPacket;
	ansPacket.roomNumber = packet->roomNumber;
	ansPacket.userNumber = userNumber;

	session->SendPacket(&ansPacket);

	delete packet;
}

void PacketProcess::C_NOTIFY_CONNECTED_ROOM(Session* session, Packet* rowPacket) {
	PK_C_NOTIFY_CONNECTED_ROOM* packet = (PK_C_NOTIFY_CONNECTED_ROOM*)rowPacket;
	SHARED_LOCK(packet->roomNumber);

	Room* room = roomManager.GetRoom(packet->roomNumber);
	User* user = room->GetUser(packet->userNumber);
	if (user == nullptr) {
		printf("C_NOTIFY_COONECTED_ROOM user is not exist!!\n");
	}
	std::unordered_map<int, User*> userMap = room->GetMap();
	std::unordered_map<int, User*>::iterator it;

	PK_S_NOTIFY_USER_DATA ansPacket;

	user->GetPositition(ansPacket.pos_X, ansPacket.pos_Y);
	ansPacket.userNumber = packet->userNumber;

	printf("C_NOTIFY_CONNECTED_ROOM : sessionAccountID : %d , userNumber %d , account Id : %d \n", session->accountId, packet->userNumber, user->GetAccountId());


	session->SendPacket(&ansPacket);


	for (it = userMap.begin(); it != userMap.end(); it++) {
		if (it->second == user) {
			continue;
		}



		PK_S_NOTIFY_OTHER_CLIENT to_User;
		User* Enemy = it->second;
		to_User.userNumber = Enemy->GetUserNumber();
		Enemy->GetPositition(to_User.pos_X, to_User.pos_Y);

		if (user->GetAccountId() < 100) {
			if (Enemy->GetAccountId() < 100)
				to_User.isDummy = false;
			session->SendPacket(&to_User);
		}
		PK_S_NOTIFY_OTHER_CLIENT to_Other_User;
		to_Other_User.userNumber = packet->userNumber;
		user->GetPositition(to_Other_User.pos_X, to_Other_User.pos_Y);



		if (Enemy->GetAccountId() >= 100) // if Enemy is Dummy
			continue;
		Session* enemySession = Enemy->GetSession();
		if (enemySession != nullptr) {
			if (session->accountId < 100)
				to_Other_User.isDummy = false;

			enemySession->SendPacket(&to_Other_User);
		}
	}

	user->SetReady();


	delete packet;

}

/*
����
*/

void PacketProcess::C_REQ_MOVE(Session* session, Packet* rowPacket) {
	PK_C_REQ_MOVE* packet = (PK_C_REQ_MOVE*)rowPacket;
	SHARED_LOCK(packet->roomNumber);
	PK_S_ANS_MOVE ansPacket;
	ansPacket.userNumber = packet->userNumber;
	ansPacket.pos_X = packet->pos_X;
	ansPacket.pos_Y = packet->pos_Y;
	ansPacket.direction = packet->direction;

	Room* room = roomManager.GetRoom(packet->roomNumber);

	if (room == nullptr) {
		Log("Usernumber(%d) req move but Room(%d) is not exist\n ", packet->userNumber, packet->roomNumber);
		delete packet;
		return;
	}


	/////Calculate Receiving Packet Count ///////////// 
	++room->RecevingPacketCount;
	room->RecevingPacketQuantity += packet->stream.offset;
	/////Ʈ���Ⱥл��� ���� ���ŷ� ���. /////////////////


	User* user = room->GetUser(packet->userNumber);
	if (user == nullptr) {
		printf("C_REQ_MOVE user is not exist!!\n");
		printf("C_REQ_MOVE roomNumber : %d userNumber : %d , Position x : %f , y : %f \n", packet->roomNumber, packet->userNumber, packet->pos_X, packet->pos_Y);

		delete packet;
		return;
	}

	if (session->accountId < 100)
		printf("C_REQ_MOVE roomNumber : %d userNumber : %d \n", packet->roomNumber, packet->userNumber);


	user->SetPosition(packet->pos_X, packet->pos_Y);
	user->SetDirection(packet->direction);
	float temp_x;
	float temp_y;

	std::unordered_map<int, User*> userMap = room->GetMap();

	std::unordered_map<int, User*>::iterator it;


	Session* userSession = nullptr;

	for (it = userMap.begin(); it != userMap.end(); it++) {
		if (it->second == user) {
			continue;
		}

		if (it->second->GetAccountId() >= 100)
			continue; // is need when target session is dummy

		if (!it->second->IsReady()) { // this mean that user is not connected room
			printf("User %d is not Ready\n", it->second->GetUserNumber());
			continue;
		}
		userSession = it->second->GetSession();
		if (userSession == nullptr)
			continue;
		userSession->SendPacket(&ansPacket);


		/////Calculate Sending Packet Count ///////////// 
		++room->SendingPacketCount;
		room->SendingPacketQuantity += sizeof(ansPacket) - sizeof(Packet) - 1;
		/////Ʈ���Ⱥл��� ���� �۽ŷ� ���. /////////////////



		user->GetPositition(temp_x, temp_y);

		/*
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


	Room* room = roomManager.GetRoom(packet->roomNumber);
	if (room == nullptr) {
		delete packet;
		return;
	}
	User* user = room->GetUser(packet->userNumber);

	PK_S_ANS_CHAT ansPacket;
	memcpy(ansPacket.name, user->GetName(), NAME_SIZE);
	ansPacket.chat = packet->chat;
	ansPacket.userNumber = packet->userNumber;

	std::unordered_map<int, User*>::iterator it;


	printf("chat : %s \n", packet->chat.c_str());

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

}

void PacketProcess::C_REQ_SHOT(Session* session, Packet* rowPacket) {
	PK_C_REQ_SHOT* packet = (PK_C_REQ_SHOT*)rowPacket;
	SHARED_LOCK(packet->roomNumber);
	Room* room = roomManager.GetRoom(packet->roomNumber);
	if (room == nullptr) {
		delete packet;
		return;
	}
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
	SHARED_LOCK(packet->roomNumber);

	Room* room = roomManager.GetRoom(packet->roomNumber);
	if (room == nullptr) {
		delete packet;
		return;
	}
	User* user = room->GetUser(packet->userNumber);
	user->ResetReady();

	user->SetUserNumber(-1);
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
	SHARED_LOCK(packet->roomNumber);
	Room* room = roomManager.GetRoom(packet->roomNumber);
	if (room == nullptr) {
		delete packet;
		return;
	}
	User* user = room->GetUser(packet->userNumber);

	static int roomInterval = int(std::sqrt(ROOMSIZE));
	int currentRoomNumber = packet->roomNumber;
	int nextRoomNumber = -1;

	if (packet->direction == 0)//TOP
	{
		if (currentRoomNumber > roomInterval) {
			nextRoomNumber = currentRoomNumber - roomInterval;
			user->SetPosition(1.66f, -3.67f);
		}
	}
	else if (packet->direction == 1)//Right
	{
		if ((currentRoomNumber % roomInterval) != 0) {
			nextRoomNumber = currentRoomNumber + 1;
			user->SetPosition(-2.4f, -1.1f);
		}
	}
	else if (packet->direction == 2)//Bot
	{
		if (currentRoomNumber <= (ROOMSIZE - roomInterval)) {
			nextRoomNumber = currentRoomNumber + roomInterval;
			user->SetPosition(1.64f, 1.63f);
		}
	}
	else if (packet->direction == 3)//Left
	{
		if ((currentRoomNumber % roomInterval) != 1) {
			nextRoomNumber = currentRoomNumber - 1;
			user->SetPosition(5.58f, -0.87f);
		}
	}

	if (nextRoomNumber == -1) {
		Log("Next Room is not exist!");
		delete packet;
		return;
	}


	Room* nextRoom = roomManager.GetRoom(nextRoomNumber);

	if (nextRoom != nullptr) {
		PK_C_REQ_EXIT_ROOM* firstEchoPacket = new PK_C_REQ_EXIT_ROOM();
		firstEchoPacket->roomNumber = user->GetRoomNumber();
		firstEchoPacket->userNumber = user->GetUserNumber();
		PacketProcess::C_REQ_EXIT_ROOM(session, firstEchoPacket);


		User* user = UserManager::getInstance().GetUser(session->accountId);

		user->SetRoomNumber(nextRoomNumber);
		user->SetUserNumber(nextRoom->AddUser(user));



		PK_C_REQ_CONNECT_ROOM* secondEchoPacket = new PK_C_REQ_CONNECT_ROOM();
		secondEchoPacket->accountId = user->GetAccountId();
		//	printf("CROSS_OVER_PK_C_REQ_CONNECT_ROOM user AccountId : %d", user->GetAccountId());
		secondEchoPacket->roomNumber = nextRoomNumber;
		PacketProcess::C_REQ_CONNECT_ROOM(session, secondEchoPacket);


	}
	else {


		PK_T_ASK_HOLD_ROOM askPacket;
		askPacket.roomNumber = nextRoomNumber;
		terminalManager.PushSession(nextRoomNumber, session);
		terminalManager.multi_SendPacket(&askPacket);

	}

	delete packet;



}



void PacketProcess::T_ASK_HOLD_ROOM(Session* session, Packet* rowPacket) {

	PK_T_ASK_HOLD_ROOM* packet = (PK_T_ASK_HOLD_ROOM*)rowPacket;
	SHARED_LOCK(packet->roomNumber);

	Room* room = roomManager.GetRoom(packet->roomNumber);
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
	SHARED_LOCK(packet->roomNumber);
	Terminal* terminal = (Terminal*)session;


	std::queue<Session*>* sessionQueue = terminalManager.GetSessionVector(packet->roomNumber);
	while (!sessionQueue->empty()) {

		Session* session = sessionQueue->front();
		sessionQueue->pop();

		PK_T_NOTIFY_USER ansPacket;

		User* user = userManager.GetUser(session->accountId);
		ansPacket.accountId = session->accountId;
		ansPacket.direction = user->GetDirection();
		ansPacket.isReady = false;
		memcpy(ansPacket.name, user->GetName(), NAME_SIZE);
		user->GetPositition(ansPacket.pos_X, ansPacket.pos_Y);
		ansPacket.roomNumber = packet->roomNumber;
		ansPacket.userNumber = -2;

		terminal->SendPacket(&ansPacket);

	}



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

	//	Terminal* targetTerminal = terminalManager.getTerminal(serverName);
	//Terminal* temp = nullptr;
	//temp = terminalManager.TerminalMap.find(serverName)->second;
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


void PacketProcess::T_ASK_LOAD(Session* session, Packet* rowPacket) {
	PK_T_ASK_LOAD* packet = (PK_T_ASK_LOAD*)rowPacket;

	PK_T_ANS_LOAD ansPacket;
	//�ڽ��� ��Ŷ�ۼ��ŷ��� ��û�̿� ���ǿ� ����
	ansPacket.load = roomManager.lastTransmittingPacketQuantity;
	ansPacket.state = (int)Monitor::getInstance().GetState();
	Log("RECEIVE T_ASK_LOAD\n");
	session->SendPacket(&ansPacket);

	delete packet;

}

void PacketProcess::T_ANS_LOAD(Session* session, Packet* rowPacket) {
	PK_T_ANS_LOAD* packet = (PK_T_ANS_LOAD*)rowPacket;
	Terminal* terminal = (Terminal*)session;
	terminal->load = packet->load;
	terminal->currentState = STATE(packet->state);
	//��û�� �� �����߿� ���� ���� ���Ϸ��� ���� �͹̳��� ����
	Terminal* targetTerminal = terminalManager.LoadCheck();


	if (targetTerminal != nullptr) {
		//���É� �͹̳ο� ���ϰ� �ɸ� ���� �˷��ִ� ��Ŷ�� ����.
		//		terminalManager.TerminalLoadInitialize();
		Room* room = roomManager.GetRoom(roomManager.loadRoomNumber);
		if (room->CheckUserMap() == false) {
			Monitor::getInstance().ChangeState(BLOCK);
			delete packet;
			return;
		}


		PK_T_NOTIFY_ROOM ansPacket;

		//� �뿡 ���ϰ� ���峪.
		Log("RECEIVE T_ANS_LOAD\n");

		ansPacket.roomNumber = roomManager.loadRoomNumber;
		ansPacket.roomSize = room->size;
		ansPacket.roomCount = room->GetCount();
		targetTerminal->SendPacket(&ansPacket);
	}

	//terminal�� load�� �����ϴ°� �����ؾ��ұ�

	delete packet;
}

void PacketProcess::T_NOTIFY_ROOM(Session* session, Packet* rowPacket) {
	PK_T_NOTIFY_ROOM* packet = (PK_T_NOTIFY_ROOM*)rowPacket;

	if (Monitor::getInstance().GetState() == BLOCK) {
		delete packet;
		return;
	}
	// RECEIVER �� �ް� �ִٴ� ���� ��Ÿ���� ���ؼ� BLOCK ���� ���� �� �� �� �ִ�.
	Monitor::getInstance().ChangeState(BLOCK);

	EXCLUSIVE_LOCK(packet->roomNumber);
	PK_T_NOTIFY_ROOM_GENERATE_COMPLETE ansPacket;
	int roomNumber = packet->roomNumber;
	if (roomManager.CreateRoom(roomNumber)) {
		Log("RECEIVE T_NOTIFY_ROOM : %d\n", packet->roomNumber);

		Room* room = roomManager.GetRoom(roomNumber);

		room->size = packet->roomSize;
		room->SetCount(packet->roomCount);

		ansPacket.roomNumber = packet->roomNumber;

	}


	session->SendPacket(&ansPacket);



	delete packet;
}



void PacketProcess::T_NOTIFY_ROOM_GENERATE_COMPLETE(Session* session, Packet* rowPacket) {
	PK_T_NOTIFY_ROOM_GENERATE_COMPLETE* packet = (PK_T_NOTIFY_ROOM_GENERATE_COMPLETE*)rowPacket;
	EXCLUSIVE_LOCK(packet->roomNumber);

	Room* room = roomManager.GetRoom(packet->roomNumber);

	std::unordered_map<int, User*>::iterator it = room->GetMap().begin();

	for (; it != room->GetMap().end(); ++it) {
		PK_T_NOTIFY_USER ansPacket;
		ansPacket.accountId = it->second->GetAccountId();
		memcpy(ansPacket.name, it->second->GetName(), NAME_SIZE);
		ansPacket.roomNumber = it->second->GetRoomNumber();
		ansPacket.userNumber = it->second->GetUserNumber();
		ansPacket.direction = it->second->GetDirection();
		it->second->GetPositition(ansPacket.pos_X, ansPacket.pos_Y);
		ansPacket.isReady = it->second->IsReady();
		Log("SEND T_NOTIFY_USER !! roomNumber : %d userNumber : %d", ansPacket.roomNumber, ansPacket.userNumber);

		session->SendPacket(&ansPacket);

	}
	roomManager.DeleteRoom(room);
	roomManager.loadRoomNumber = -1;
	roomManager.lastTransmittingPacketQuantity = 0;

	delete packet;
}


void PacketProcess::T_NOTIFY_USER(Session* session, Packet* rowPacket) {
	PK_T_NOTIFY_USER* packet = (PK_T_NOTIFY_USER*)rowPacket;
	EXCLUSIVE_LOCK(packet->roomNumber);
	Room* room = roomManager.GetRoom(packet->roomNumber);

	User* user = new User(packet->accountId, packet->name);
	user->SetAddress((int)&(*user));
	user->SetRoomNumber(packet->roomNumber);
	user->SetUserNumber(packet->userNumber);
	user->SetDirection(packet->direction);
	user->SetPosition(packet->pos_X, packet->pos_Y);
	// user->SetReady;
	userManager.DeleteUser(packet->accountId);
	userManager.AddUser(user);

	Log("T_NOTIFY_USE::  accountId : %d  roomNumber : %d  userNumber : %d",
		packet->accountId, packet->roomNumber,
		packet->userNumber);

	if (packet->userNumber != -2)
		room->AddUser(user, packet->userNumber);
	else {
		user->SetUserNumber(room->AddUser(user));
	}
	PK_T_ANS_USER ansPacket;
	ansPacket.accountId = packet->accountId;
	ansPacket.roomNumber = packet->roomNumber;
	ansPacket.userNumber = packet->userNumber;
	session->SendPacket(&ansPacket);

	delete packet;
}


// userData �� ������ �������� ������ ���� user���� ������ �˷���.
void PacketProcess::T_ANS_USER(Session* session, Packet* rowPacket) {
	PK_T_ANS_USER* packet = (PK_T_ANS_USER*)rowPacket;

	User* user = userManager.GetUser(packet->accountId);

	Session* userSession = nullptr;
	userSession = user->GetSession();

	/*
	User������ ���� ���ߴµ� room migration�� �������� �Ͼ���
	user�� ������ ���.. ������

	���ɶ� �ɸ� �����尡 �����ؼ� ContentsProcess�� ���ߴ� ������ ������������

	*/

	PK_S_NOTIFY_SERVER sendingPacket;
	sendingPacket.accountId = user->GetAccountId();
	memcpy(sendingPacket.ip, session->ip, NAME_SIZE);
	sendingPacket.port = session->port;
	sendingPacket.roomNumber = packet->roomNumber;

	userSession->SendPacket(&sendingPacket);

	Log("T_ANS_USER::  accountId : %d  roomNumber : %d  userNumber : %d", packet->accountId, packet->roomNumber,
		packet->userNumber);

	delete packet;
}
