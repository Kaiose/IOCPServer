#pragma once
#include "stdafx.h"
#include "GamePacketProcess.h"

//HANDLE ContentsProcess::serverIOCPHandle = NULL;



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

		UserManager::getInstance().DeleteUser(packet->accountId); //User 동적 할당 해제까지 함

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
		//TODO 동기화를 해주자!
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



// Manager 에서 지우는 것은 동적할당까지 풀어주는 것
void PacketProcess::C_REQ_CONNECT_ROOM(Session* session, Packet* rowPacket) {
	PK_C_REQ_CONNECT_ROOM* packet = (PK_C_REQ_CONNECT_ROOM*)rowPacket;
	SHARED_LOCK(packet->roomNumber);
	Room* room = RoomManager::getInstance().GetRoom(packet->roomNumber);
	if (room == nullptr) { // room 을 가지고 있는지 없는지 두번 확인하게 되는데
						   // C_REQ_COONECT_ROOM 에서 확인 하는 이유는 처음 룸 클릭시 룸 여부를 확인
						   // C_REQ_CROSS_OVER_ROOM 은 맵 내부에서 이동시

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


	// 이걸 해논이유가 룸이 이동할때 유저가 있으면 새로 룸에 넣지 않기위해서 하는건데
	// 서버이동이 아닌 룸사이간의 이동시에 
	/*
	ex) 방1 에 133번 방2에 134번까지의 유저가 있다고했을때
	방1의 133번 유저가 방2로 옮긴다고 하자.
	그럴때  방 1의 133번 유저와 장2의 133번 유저는 다른 유저임에도 불구하고
	133번 UserNumber가 있으므로  방2의 유저에 덮어씌운다.

	때문에 이작업을 성공적으로 완료키 위해서는

	C_REQ_EXIT 작업과 CONNECTED 작업 사이에서 room 에 유저를 추가하는 것을 어떨까

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
서버
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
	/////트래픽분산을 위한 수신량 계산. /////////////////


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
		/////트래픽분산을 위한 송신량 계산. /////////////////



		user->GetPositition(temp_x, temp_y);

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
	//자신의 패킷송수신량을 요청이온 세션에 응답
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
	//요청이 온 세션중에 가장 낮은 부하량을 가진 터미널을 선택
	Terminal* targetTerminal = terminalManager.LoadCheck();


	if (targetTerminal != nullptr) {
		//선택됀 터미널에 부하가 걸린 룸을 알려주는 패킷을 전송.
		//		terminalManager.TerminalLoadInitialize();
		Room* room = roomManager.GetRoom(roomManager.loadRoomNumber);
		if (room->CheckUserMap() == false) {
			Monitor::getInstance().ChangeState(BLOCK);
			delete packet;
			return;
		}


		PK_T_NOTIFY_ROOM ansPacket;

		//어떤 룸에 부하가 생겼나.
		Log("RECEIVE T_ANS_LOAD\n");

		ansPacket.roomNumber = roomManager.loadRoomNumber;
		ansPacket.roomSize = room->size;
		ansPacket.roomCount = room->GetCount();
		targetTerminal->SendPacket(&ansPacket);
	}

	//terminal의 load를 취합하는걸 언제해야할까

	delete packet;
}

void PacketProcess::T_NOTIFY_ROOM(Session* session, Packet* rowPacket) {
	PK_T_NOTIFY_ROOM* packet = (PK_T_NOTIFY_ROOM*)rowPacket;

	if (Monitor::getInstance().GetState() == BLOCK) {
		delete packet;
		return;
	}
	// RECEIVER 가 받고 있다는 것을 나타내기 위해서 BLOCK 으로 변경 됄 수 는 있다.
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


// userData 를 보내는 서버에서 응답이 오면 user에게 서버를 알려줌.
void PacketProcess::T_ANS_USER(Session* session, Packet* rowPacket) {
	PK_T_ANS_USER* packet = (PK_T_ANS_USER*)rowPacket;

	User* user = userManager.GetUser(packet->accountId);

	Session* userSession = nullptr;
	userSession = user->GetSession();

	/*
	User응답을 전부 못했는데 room migration이 연속으로 일어날경우
	user의 세션이 없어서.. 에러남

	스핀락 걸면 스레드가 부족해서 ContentsProcess가 멈추는 현상이 나오지않을까

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
