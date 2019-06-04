#include "stdafx.h"
#include "Session.h"


#include <sqlext.h>




Session::Session(SOCKET socket) : socket(socket) {

	recvCount = 0;
	sendCount = 0;

	InitializeCriticalSection(&critical_section);
	accountId = 0;
	memset(ip, NULL, 20);
	ioData = new IO_Data[IO_ACCEPT];
	ioData[IO_READ].session = this;
	ioData[IO_WRITE].session = this;
	roomNumber = -1;
	sessionStatus = false;
	Initialize();	

	threadStatus = false;

	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);



}


Session::~Session()
{
	DeleteCriticalSection(&critical_section);
	sessionStatus = false;
	threadStatus = false;
	if(move.joinable())
		move.join();
	
	Log("Session Close!!!");
	PK_C_REQ_EXIT packet;
	packet.accountId = accountId;
	packet.roomNumber = roomNumber;
	packet.userNumber = userNumber;
	
	SendPacket(&packet);

	Clear();
}

void Session::setSocket(SOCKET socket) {
	
	this->socket = socket;
	
}

void Session::Initialize() {
	memset(mainBuffer, NULL, sizeof(char)* BUF_SIZE);
	totalBytes = 0;
	memset(ip, NULL, 20);
}

void Session::Connect(const char* ip, int port) {

	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &(serverAddr.sin_addr));
	serverAddr.sin_port = htons(port);

	if (connect(socket, (sockaddr*)&serverAddr, sizeof(serverAddr))<0) {
		printf("connect error : %d , port : %d \n", WSAGetLastError(), port);
		Log("Connect Falied !! Ip : %s , Port : %d UserNumber: %d \n", this->ip, port, this->userNumber);
		if (WSAGetLastError() == 10060) {
			Connect(ip, port);
		}
	}
	else {
		int flag = 1; // 네이글 알고리즘 off
	/*	setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
		LINGER opt = { 1, 0 }; // 값을 설정
		setsockopt(socket, SOL_SOCKET, SO_LINGER, (char *)&opt, sizeof(opt));
		// close socket을 비동기로 하지 않기 위해 링거옵션 설정.
		*/

		memcpy(this->ip, ip, NAME_SIZE);
		this->port = port;
		Log("Connect Success !! Ip : %s , Port : %d UserNumber: %d \n", this->ip, this->port, this->userNumber);
		
	}
	
	/*
	sessionStatus = true;
	threadStatus = true;*/
	
	SetEvent(hEvent);
}

void Session::SetType() {
	ioData[IO_READ].ioType = IO_READ;
	ioData[IO_WRITE].ioType = IO_WRITE;
}

void Session::lock() {
	EnterCriticalSection(&critical_section);
}

void Session::unlock() {
	LeaveCriticalSection(&critical_section);
}


void Session::ChangeSocket() {


	SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	

	
	this->setSocket(sock);
	
	closesocket(socket2close);
	
}

void Session::ChangeConnect() {
	Session::Connect(nextIp, nextPort);
}

void Session::SendPacket(Packet* packet) {
	
	
	packet->stream.offset = 0;
	//printf("this thread ID : %d\n", std::this_thread::get_id());
	/*
		

	*/
	
	packet->Encoding();
	int headerSize = sizeof(int);
	int packetLen = headerSize + packet->stream.offset;

	memmove(&packet->stream.buffer[sizeof(int)], packet->stream.buffer, packet->stream.offset);
	memcpy(packet->stream.buffer, &packetLen, sizeof(int));

	WSABUF wsaBuf = { NULL, };

	wsaBuf.buf = (char*)packet->stream.buffer;
	wsaBuf.len = packetLen;

	DWORD sendBytes = 0;
	DWORD flags = 0;

	if (packet->GetType() == E_C_REQ_EXIT) {
		int ret = WSASend(socket, &wsaBuf, 1, &sendBytes, flags, NULL, NULL);
		if (ret == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING) {
			printf("Synchro Send : %d packet->type : %d \n", WSAGetLastError(), packet->GetType());
		}
		else {
			PK_C_REQ_EXIT* temp = (PK_C_REQ_EXIT*)packet;
			printf("C_REQ_EXIT Send Success !! Port : %d  accountId : %d  roomNumber : %d userNumber : %d \n", this->port, temp->accountId, temp->roomNumber, temp->userNumber);
		}
	}
	else {
		int ret = WSASend(socket, &wsaBuf, 1, &sendBytes, flags, &ioData[IO_WRITE], NULL);
		if (ret == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING) {
			printf("WSASend : %d packet->type : %d \n", WSAGetLastError(), packet->GetType());
		}
	}
	if(packet->GetType() != 112)
		printf("WSASend !! packet->type : %d  , accountId : %d roomNumber : %d userNumber : %d\n",  packet->GetType(), this->accountId, this->roomNumber,this->userNumber );

	
}

void Session::Recv(int recvBytes) {

	memcpy(&mainBuffer[totalBytes], ioData[IO_READ].buffer, recvBytes);
	
	//memset(ioData[IO_READ].buffer, NULL, recvBytes);
	//printf("before totalBytes ===>>>> %d \n", totalBytes);
	totalBytes += recvBytes;
	//printf("totalBytes %d , recvBytes : %d \n", totalBytes , recvBytes);
	bool result = PacketAnalyzer();
	if (result == false) {
		printf("need more Data...");
		//RecvStanby();
	}
}


int Session::getId() {
	return accountId;
}

void Session::setId(int id) {
	this->accountId = id;
}


void Session::RecvStanby() {
	DWORD flags = 0;
	DWORD recvBytes;
	WSABUF wsabuf;
	wsabuf.buf = ioData[IO_READ].buffer;
	wsabuf.len = BUF_SIZE - totalBytes;


	int ret = WSARecv(socket, &wsabuf, 1, &recvBytes, &flags, (LPOVERLAPPED)&ioData[IO_READ], NULL);
	if (ret == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING) {
		printf("WSARecv : %d \n", WSAGetLastError());
	}

	
}

bool Session::PacketAnalyzer() {

	// 11/19 과도한 메모리 카피로 인한 성능저하...  
	// pointer를 통한 개선..
	do {
		int* packetLen = nullptr;
		int currentOffset = 0;

		//memcpy(&packetLen, mainBuffer, sizeof(int));

		packetLen = (int*)mainBuffer;

		if (*packetLen > totalBytes) {
			printf("data is not enough to Analyzer! \n");
			return false;
		}

		currentOffset += sizeof(int);

		PacketType* packetType = nullptr;
		packetType = (PacketType*)&mainBuffer[currentOffset];
		//memcpy(&packetType, &mainBuffer[currentOffset], sizeof(int));
		currentOffset += sizeof(int);

		Packet* packet = GetPacketClass(*packetType);
		if (packet == nullptr) {
			memset(mainBuffer, NULL, BUF_SIZE);
			totalBytes = 0;
			return true;
		}
		//printf("PacketLen : %d \n", packetLen);
		memcpy(packet->stream.buffer, &mainBuffer[currentOffset], *packetLen - currentOffset);

		//Log("Before totalBytes : %d \n", totalBytes);
		totalBytes = totalBytes - *packetLen;
		//Log("after totalBytes : %d \n", totalBytes);
		
		if (totalBytes <= 0)
			memset(mainBuffer, NULL, BUF_SIZE);
		//else if (totalBytes - *packetLen < 0) {
		//	memcpy(mainBuffer, &mainBuffer[*packetLen], totalBytes);
		//	memset(&mainBuffer[totalBytes], NULL, BUF_SIZE - totalBytes);
		//}
		else
			memcpy(mainBuffer, &mainBuffer[*packetLen], totalBytes);
	
		packet->Decoding();

		// Session 으로부터 0.001 초마다 데이터가 들어오게 설정해놓으면
		// 패킷들이 뭉쳐서 들어오게 되는데
		// totalBytes가 >= 0 큰 상황에 대해서 처리를 안해주고 있는 상태임 현재.
		// 이걸 session 마다 pakcet Queue를 가져가고
		// server에서 recv 한 후 PacketQueue를 확인해서
		// Queue 가없을떄 까지 ContentesProcess에 Push하는 로직으로 만들자.

		// 아무리커도 wsabbuf 58512 로 보임

		// 버티도록 만들어야 하나?
		packetQueue.push(packet);


	//	printf("받은 패킷 타입 : %d ,  %d번째\n", packet->GetType(), recvCount++);

	} while (totalBytes > 0);
	return true;

}

Packet* Session::GetPacket() {

	if (packetQueue.empty() == true) return nullptr;

	Packet* retPacket = packetQueue.front();
	packetQueue.pop();

	return retPacket;

}


void Session::ZeroByteReceive() {
	

	DWORD flags = 0;
	DWORD recvBytes;
	WSABUF wsabuf;
	wsabuf.buf = NULL;
	wsabuf.len = 0;
	int ret = WSARecv(socket, &wsabuf, 1, &recvBytes,&flags, &ioData[IO_READ], NULL);
	if (ret == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING) {
	//	printf("WSARecv : %d \n", WSAGetLastError());
	}
}

void Session::Close() {
	closesocket(socket);
}

void Session::Clear() {
	closesocket(socket);
	SAFE_DELETE(ioData);
}



#define random std::chrono::system_clock::now().time_since_epoch().count()
void Session::randomMoving() {

	if (threadStatus == false)
		threadStatus = true;
	else
		return;
//	std::chrono::duration<double> sec = std::chrono::system_clock::now().time_since_epoch;
	srand(std::chrono::system_clock::now().time_since_epoch().count());
#define COUNT 3
	move = std::thread(
		[&]() {  while (threadStatus) {
		
		PK_C_REQ_MOVE packet;
		packet.direction = random % 9;
		packet.roomNumber = roomNumber;
		packet.userNumber = userNumber;
		for (int i = 0; i < COUNT; i++) {
			packet.stream.offset = 0;
			switch (packet.direction) {
			case 0:
				break;
			case 1:
				pos_Y += distance;
				break;
			case 2:
				pos_X += distance;
				pos_Y += distance;
				break;
			case 3:
				pos_X += distance;
				break;
			case 4:
				pos_X += distance;
				pos_Y -= distance;
				break;
			case 5:
				pos_Y -= distance;
				break;
			case 6:
				pos_X -= distance;
				pos_Y -= distance;
				break;
			case 7:
				pos_X -= distance;

				break;
			case 8:
				pos_X -= distance;
				pos_Y += distance;
				break;
			}

			packet.pos_X = pos_X;
			packet.pos_Y = pos_Y;

			

		//	std::chrono::milliseconds duration(100);
		//	std::this_thread::sleep_for(duration);
		}
		EnterCriticalSection(&critical_section);
		SendPacket(&packet);
		int timer = rand() % 5000;
		if (timer < 3000)
			timer += 3000;
		std::chrono::milliseconds duration(timer);
		std::this_thread::sleep_for(duration);
		packet.direction = 0;
		SendPacket(&packet);
		LeaveCriticalSection(&critical_section);
		timer = rand() % 5000;
		if (timer < 3000)
			timer += 3000;
		std::chrono::milliseconds duration2(timer);
		std::this_thread::sleep_for(duration2);
		Log("C_REQ_MOVE !! Room Number : %d ,  User Number :  %d \n", roomNumber, userNumber);

		}
		

	}
	

	);
}


Packet* Session::GetPacketClass(PacketType packetType) {
	switch (packetType) {
	case E_C_REQ_EXIT: return new PK_C_REQ_EXIT();
	case E_S_ANS_EXIT: return new PK_S_ANS_EXIT();
	case E_C_REQ_SIGNUP: return new PK_C_REQ_SIGNUP();
	case E_S_ANS_SIGNUP: return new PK_S_ANS_SIGNUP();
	case E_C_REQ_SIGNIN: return new PK_C_REQ_SIGNIN();
	case E_S_ANS_SIGNIN: return new PK_S_ANS_SIGNIN();
	case E_S_NOTIFY_SERVER: return new PK_S_NOTIFY_SERVER();
	case E_C_REQ_CONNECT: return new PK_C_REQ_CONNECT();
	case E_S_ANS_CONNECT: return new PK_S_ANS_CONNECT();
	case E_C_REQ_CONNECT_ROOM: return new PK_C_REQ_CONNECT_ROOM();
	case E_S_ANS_CONNECT_ROOM: return new PK_S_ANS_CONNECT_ROOM();
	case E_S_NOTIFY_OTHER_CLIENT: return new PK_S_NOTIFY_OTHER_CLIENT();
	case E_C_REQ_CHAT: return new PK_C_REQ_CHAT();
	case E_S_ANS_CHAT: return new PK_S_ANS_CHAT();
	case E_C_REQ_MOVE: return new PK_C_REQ_MOVE();
	case E_S_ANS_MOVE: return new PK_S_ANS_MOVE();
	case E_C_NOTIFY_CONNECTED_ROOM: return new PK_C_NOTIFY_CONNECTED_ROOM();
	case E_S_NOTIFY_USER_DATA: return new PK_S_NOTIFY_USER_DATA();
	case E_C_REQ_SHOT: return new PK_C_REQ_SHOT();
	case E_S_ANS_SHOT: return new PK_S_ANS_SHOT();
	case E_C_REQ_EXIT_ROOM: return new PK_C_REQ_EXIT_ROOM();
	case E_S_ANS_EXIT_ROOM: return new PK_S_ANS_EXIT_ROOM();
	}

	Log("Is Not Regist Packey Type!! : %d \n", packetType);
	return nullptr;
}
