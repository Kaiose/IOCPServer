#pragma once
#include "stdafx.h"



class PacketProcess : public ContentsProcess
{
public:


	

	std::string		className;
	PacketProcess();
	~PacketProcess();


	static int virtualAccountId;

	static void ReadXML();

	static void C_REQ_EXIT(Session* session, Packet* packet);


	static void C_REQ_SIGNIN(Session* session, Packet* packet);
	static void C_REQ_SIGNUP(Session* session, Packet* packet);

	static void C_REQ_CONNECT(Session* session, Packet* packet);
	static void C_REQ_CONNECT_ROOM(Session* session, Packet* packet);
	static void C_NOTIFY_CONNECTED_ROOM(Session* session, Packet* packet);
	

	static void C_REQ_CHAT(Session* session, Packet* packet);
	static void C_REQ_MOVE(Session* session, Packet* packet);
	
	static void C_REQ_SHOT(Session* session, Packet* packet);


	static void C_REQ_EXIT_ROOM(Session* session, Packet* packet);

	
	static void S_ANS_EXIT(Session* session, Packet* packet);

	static void S_ANS_CONNECT(Session* session, Packet* packet);

	static void S_ANS_CONNECT_ROOM(Session* session, Packet* packet);

	static void S_NOTIFY_USER_DATA(Session* session, Packet* packet);

	static void S_NOTIFY_OTHER_CLIENT(Session* session, Packet* packet);

	static void S_ANS_MOVE(Session* session, Packet* packet);


	static void S_ANS_CHAT(Session* session, Packet* packet);
	
	static void S_ANS_SHOT(Session* session, Packet* packet);

	static void S_NOTIFY_SERVER(Session* session, Packet* packet);

};
