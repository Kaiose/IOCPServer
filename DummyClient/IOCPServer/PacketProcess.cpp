#pragma once
#include "stdafx.h"
#include "PacketProcess.h"


HANDLE ContentsProcess::serverIOCPHandle = NULL;

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
	UserManager::getInstance().DeleteUser(packet->accountId); //User ���� �Ҵ� �������� ��

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
		//TODO ����ȭ�� ������!
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



// Manager ���� ����� ���� �����Ҵ���� Ǯ���ִ� ��
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
