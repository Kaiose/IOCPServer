#include "stdafx.h"
#include "Session.h"


#include <sqlext.h>

Session::Session(SOCKET socket) : socket(socket) {

	recvCount = 0;
	sendCount = 0;

	
	memset(ip, NULL, 20);
	ioData = new IO_Data[IO_ACCEPT];
	ioData[IO_READ].session = this;
	ioData[IO_WRITE].session = this;
	Initialize();
}

Session::Session() {

	recvCount = 0;
	sendCount = 0;



	memset(ip, NULL, 20);
	ioData = new IO_Data[IO_ACCEPT];
	ioData[IO_READ].session = this;
	ioData[IO_WRITE].session = this;
	Initialize();
}




void Session::Initialize() {
	memset(mainBuffer, NULL, sizeof(char)* BUF_SIZE);
	totalBytes = 0;
	memset(ip, NULL, 20);
	queueClear();
}

void Session::queueClear() {
	while (!packetQueue.empty()) {
		packetQueue.pop();
	}
}


void Session::SetType() {
	ioData[IO_READ].ioType = IO_READ;
	ioData[IO_WRITE].ioType = IO_WRITE;
}

void Session::SendPacket(Packet* packet) {
	__try {
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

		int ret = WSASend(socket, &wsaBuf, 1, &sendBytes, flags, &ioData[IO_WRITE], NULL);
		if (ret == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING) {
			printf("WSASend : %d  accountId : %d \n", WSAGetLastError(), accountId);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
			Log("Session Access Violation!!! Exception Code : %ld\n",GetExceptionCode());
	}
//	printf("Sending!! packet->type : %d  , stream Size : %d SendBytes : %d\n", packet->GetType(),packet->stream.offset, sendBytes);


}


void Session::SyncSendPacket(Packet* packet) {

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

	int ret = WSASend(socket, &wsaBuf, 1, &sendBytes, flags, NULL, NULL);
	if (ret == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING) {
		printf("Sync WSASend Error : %d  accountId : %d \n", WSAGetLastError(), accountId);
	}

	printf("SYNC Sending!! packet->type : %d  , stream Size : %d SendBytes : %d\n", packet->GetType(),packet->stream.offset, sendBytes);


}



void Session::Recv(int recvBytes) {

	memcpy(&mainBuffer[totalBytes], ioData[IO_READ].buffer, recvBytes);
	
	totalBytes += recvBytes;
	bool result = PacketAnalyzer();
	if (result == false) {
		printf("need more Data...");
	}
}



void Session::RecvStanby() {
	DWORD flags = 0;
	DWORD recvBytes;
	WSABUF wsabuf;
	wsabuf.buf = ioData[IO_READ].buffer;
//	ioData[IO_READ].Fill_Zero();

	wsabuf.len = BUF_SIZE - totalBytes;


	int ret = WSARecv(socket, &wsabuf, 1, &recvBytes, &flags, (LPOVERLAPPED)&ioData[IO_READ], NULL);
	if (ret == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING) {
		printf("WSARecv : %d  socket : %d accountId : %d\n", WSAGetLastError(),(int)socket,accountId);

		if (WSAGetLastError() == WSAECONNRESET) {
			printf("ALERT WSAECONNRESET!!  socket : %d accountId : %d\n", (int)socket, accountId);
			
			/*
				빈번한 로드 발생으로 인한 접속 재설정으로 인해 나타남
				소켓을 사용할수 없음.
				C_REQ_EXIT 함수가 처리하는 일을 해야함.
			*/
			
//			UserManager::getInstance().DeleteUser(accountId);
/*
			Initialize();
			ioData[IO_READ].Initialize();
			ioData[IO_WRITE].Initialize();
			*/
			
			LPFN_DISCONNECTEX pFnDisconnect = (LPFN_DISCONNECTEX)GetSockExtAPI(socket, WSAID_DISCONNECTEX);

			if (pFnDisconnect(socket, NULL, TF_REUSE_SOCKET, 0)) {
				Log("ALERT WSAECONNRESET!! Session Disconnect Request Success ==> %s , socket : %d accountId : %d \n", ip, socket, accountId);
			}
			else {
				Log("ALERT WSAECONNRESET!! Session Disconnect Request Failed ==> %s , socket : %d accountId : %d \n", ip, socket, accountId);
			}
			
		}
	}

	
}

bool Session::PacketAnalyzer() {

	// 11/19 과도한 메모리 카피로 인한 성능저하...  
	// pointer를 통한 개선..
	if (totalBytes <= 0)
		return false;

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
		currentOffset += sizeof(int);

		Packet* packet = GetPacketClass(*packetType);

		memcpy(packet->stream.buffer, &mainBuffer[currentOffset], *packetLen - currentOffset);

		totalBytes = totalBytes - *packetLen;
		
		if (totalBytes <= 0)
			memset(mainBuffer, NULL, BUF_SIZE);
		else
			memcpy(mainBuffer, &mainBuffer[*packetLen], totalBytes);
	
		packet->Decoding();

		// Session 으로부터 0.001 초마다 데이터가 들어오게 설정해놓으면
		// 패킷들이 뭉쳐서 들어오게 되는데
		// totalBytes가 >= 0 큰 상황에 대해서 처리를 안해주고 있는 상태임 현재.
		// 이걸 session 마다 pakcet Queue를 가져가고
		// server에서 recv 한 후 PacketQueue를 확인해서
		// Queue 가없을떄 까지 ContentesProcess에 Push하는 로직으로 만들자.

		packetQueue.push(packet);


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
	PK_S_ANS_EXIT packet;
	packet.accountId = accountId;
	SyncSendPacket(&packet);
	closesocket(socket);
	delete ioData;

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
	case E_S_ACCEPT_EXIT_ROOM: return new PK_S_ACCEPT_EXIT_ROOM();
	case E_C_REQ_CROSS_OVER_SERVER: return new PK_C_REQ_CROSS_OVER_SERVER();
	case E_T_ASK_HOLD_ROOM: return new PK_T_ASK_HOLD_ROOM();
	case E_T_ANS_HOLD_ROOM: return new PK_T_ANS_HOLD_ROOM();
	case E_T_REQ_CONNECT: return new PK_T_REQ_CONNECT();
	case E_T_ANS_CONNECT: return new PK_T_ANS_CONNECT();
	case E_T_ASK_LOAD: return new PK_T_ASK_LOAD();
	case E_T_ANS_LOAD: return new PK_T_ANS_LOAD();
	case E_T_NOTIFY_ROOM: return new PK_T_NOTIFY_ROOM();
	case E_T_NOTIFY_ROOM_GENERATE_COMPLETE: return new PK_T_NOTIFY_ROOM_GENERATE_COMPLETE();
	case E_T_NOTIFY_USER: return new PK_T_NOTIFY_USER();
	case E_T_ANS_USER: return new PK_T_ANS_USER();
	}
	Log("Is Not Regist Packey Type!! : %d \n", packetType);
	return nullptr;
}
