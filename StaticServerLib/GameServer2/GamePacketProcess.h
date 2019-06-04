#pragma once
#include "stdafx.h"


static SRWLOCK roomMigration;


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

	static void C_REQ_CROSS_OVER_SERVER(Session* session, Packet* packet);

	static void T_ASK_HOLD_ROOM(Session* session, Packet* packet);

	static void T_ANS_HOLD_ROOM(Session* session, Packet* packet);

	static void T_REQ_CONNECT(Session* session, Packet* packet);

	static void T_ASK_LOAD(Session* session, Packet* packet);

	static void T_ANS_LOAD(Session* session, Packet* packet);

	static void T_NOTIFY_ROOM(Session* session, Packet* packet);

	static void T_NOTIFY_ROOM_GENERATE_COMPLETE(Session* session, Packet* packet);

	static void T_NOTIFY_USER(Session* session, Packet* packet);

	static void T_ANS_USER(Session* session, Packet* packet);


	//static void TransmittingPacketCheck();
};
