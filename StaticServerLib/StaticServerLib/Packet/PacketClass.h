#pragma once
#include "stdafx.h"

enum PacketType {
	E_T_REQ_CONNECT = 80,
	E_T_ANS_CONNECT = 81,
	E_C_REQ_EXIT = 90,
	E_S_ANS_EXIT = 91,
	PACKET_TYPE_ERROR = 99,
	E_C_REQ_SIGNUP = 100,
	E_S_ANS_SIGNUP = 101,
	E_C_REQ_SIGNIN = 102,
	E_S_ANS_SIGNIN = 103,
	E_S_NOTIFY_SERVER = 104,
	E_C_REQ_CONNECT = 105,
	E_S_ANS_CONNECT = 106,
	E_C_REQ_CONNECT_ROOM = 107,
	E_S_ANS_CONNECT_ROOM = 108,
	E_S_NOTIFY_OTHER_CLIENT = 109,
	E_C_REQ_CHAT = 110,
	E_S_ANS_CHAT = 111,
	E_C_REQ_MOVE = 112,
	E_S_ANS_MOVE = 113,
	E_C_NOTIFY_CONNECTED_ROOM = 114,
	E_S_NOTIFY_USER_DATA = 115,
	E_C_REQ_SHOT = 116,
	E_S_ANS_SHOT = 117,
	E_C_REQ_EXIT_ROOM = 118,
	E_S_ANS_EXIT_ROOM = 119,
	E_S_ACCEPT_EXIT_ROOM = 120,
	E_C_REQ_CROSS_OVER_SERVER = 121,
	E_T_ASK_HOLD_ROOM = 122,
	E_T_ANS_HOLD_ROOM = 123,
	E_T_ASK_LOAD = 124,
	E_T_ANS_LOAD = 125,
	E_T_NOTIFY_ROOM = 126,
	E_T_NOTIFY_ROOM_GENERATE_COMPLETE = 127,
	E_T_NOTIFY_USER = 128,
	E_T_ANS_USER = 129,
};


class Stream {

public:
	int offset;
	BYTE buffer[BUF_SIZE];

	void operator<< (int Value) {
		memcpy(buffer + offset, &Value, sizeof(int));
		offset += sizeof(int);
	};

	void operator<< (float Value) {

		memcpy(buffer + offset, &Value, sizeof(float));
		offset += sizeof(float);

	}

	void operator<< (char Value) {
		memcpy(buffer + offset, &Value, sizeof(char));
		offset += sizeof(char);
	};

	void operator<< (short Value) {
		memcpy(buffer + offset, &Value, sizeof(short));
		offset += sizeof(short);
	};


	void operator<< (wchar_t Value) {
		memcpy(buffer + offset, &Value, sizeof(wchar_t));
		offset += sizeof(wchar_t);
	}

	void operator<< (bool Value) {
		memcpy(buffer + offset, &Value, sizeof(bool));
		offset += sizeof(bool);
	}

	void operator<< (char* Value) {
		
		int stringSize = int(sizeof(char)*strlen(Value));
		*this << stringSize;

		memcpy(buffer + offset, Value ,stringSize);
		offset += stringSize;
	};

	void operator<< (std::string Value) {

		int stringSize = int(Value.length());
		*this << stringSize;

		
	

		memcpy(buffer + offset, Value.c_str(), stringSize);
		offset += stringSize;

	};



//-----------------------------------------------------------------------------------------------//
	void operator>> (int& Value) {
		memcpy(&Value, buffer + offset, sizeof(int));
		offset += sizeof(int);
	};


	void operator>> (short& Value) {
		memcpy(&Value, buffer + offset, sizeof(short));
		offset += sizeof(short);
	};


	void operator>> (bool& Value) {
		memcpy(&Value, buffer + offset, sizeof(bool));
		offset += sizeof(bool);
	}

	void operator>> (char* Value) {
		int stringSize;
		*this >> stringSize;
		memcpy(Value, buffer + offset, stringSize);
		offset += sizeof(char) * stringSize;
	};

	void operator>> (char& Value) {

		memcpy(&Value, buffer + offset, sizeof(char));
		offset += sizeof(char);
		
	}

	void operator>> (wchar_t* Value) {
		int stringSize;

		*this >> stringSize;

		memcpy(Value, buffer + offset, sizeof(wchar_t) * stringSize);

		offset += sizeof(wchar_t) * stringSize;

	}

	void operator>> (wchar_t& Value) {

		memcpy(&Value, buffer + offset, sizeof(wchar_t));
		offset += sizeof(wchar_t);

	}

	void operator>> (float& Value) {

		memcpy(&Value, buffer + offset, sizeof(float));
		offset += sizeof(float);
	}

	void operator>> (std::string& Value) {
		int stringSize;

		*this >> stringSize;

		for (int i = 0; i < stringSize; i++) {
			Value.push_back(buffer[offset]);
			offset += 1;
		}
	}


};  

class Packet
{

public:
	Stream stream;

public:

//	int accountId;

public:
	Packet() {
		memset(this, NULL, sizeof(*this));
	//	accountId = -1;
	}
	~Packet() { };
	virtual PacketType GetType() { return PACKET_TYPE_ERROR; };
	virtual void Encoding() {};
	virtual void Decoding() {};
};

//
//E_C_REQ_EXIT = 90,

class Packet_Ex : public Packet {
public:
	int accountId;


public:
	

};




class PK_C_REQ_EXIT : public Packet {
public:
	int accountId;
	int roomNumber;
	int userNumber;
public:

	PK_C_REQ_EXIT() {
		accountId = -1;
	}

	PacketType GetType() { return E_C_REQ_EXIT; }

	void Encoding() {

		stream << (int)GetType();
		stream << accountId;
		stream << roomNumber;
		stream << userNumber;
	}


	void Decoding() {
		stream >> accountId;
		stream >> roomNumber;
		stream >> userNumber;
	}


};


class PK_S_ANS_EXIT : public Packet {
public:
	int accountId;
public:

	PK_S_ANS_EXIT() {
	
	}

	PacketType GetType() { return E_S_ANS_EXIT; }

	void Encoding() {

		stream << (int)GetType();

		stream << accountId;
	}


	void Decoding() {
	
		stream >> accountId;
	}


};

class PK_C_REQ_CHAT : public Packet {

public:
	int roomNumber;
	int userNumber;
	//char chat[CHAT_SIZE];
	std::string chat;
public:

	PK_C_REQ_CHAT() {
	//	memset(name, NULL, sizeof(char)*NAME_SIZE);
	//	memset(chat, NULL, sizeof(char)*CHAT_SIZE);
	}

	PacketType GetType() { return E_C_REQ_CHAT; }


	void Encoding() {
		stream << (int)GetType();
		stream << roomNumber;
		stream << userNumber;
		stream << chat;
	}
	void Decoding() {
		stream >> roomNumber;
		stream >> userNumber;
		stream >> chat;
	}


};

class PK_S_ANS_CHAT : public Packet {
public:
	char name[NAME_SIZE];
	int userNumber;
	std::string chat;
//	char chat[CHAT_SIZE];
public:
	PK_S_ANS_CHAT() {
//		memset(name, NULL, sizeof(char)*NAME_SIZE);
	}
	PacketType GetType() { return E_S_ANS_CHAT; }

	void Encoding() {
		stream << (int)GetType();
		stream << name;
		stream << userNumber;
		stream << chat;
	}

	void Decoding() {
		stream >> name;
		stream >> userNumber;
		stream >> chat;
	}
};


class PK_C_REQ_SIGNUP : public Packet {
public:
	char name[NAME_SIZE];
	char password[NAME_SIZE];
public:
	PK_C_REQ_SIGNUP() {
		memset(name, NULL, NAME_SIZE);
		memset(password, NULL, NAME_SIZE);
	}
	PacketType GetType() { return E_C_REQ_SIGNUP; }

	void Encoding() {
		stream << (int)GetType();
		stream << name;
		stream << password;
	}

	void Decoding() {
		stream >> name;
		stream >> password;
	}
};

class PK_S_ANS_SIGNUP : public Packet {
public:
	bool result;
public:
	PK_S_ANS_SIGNUP() {
	}
	PacketType GetType() { return E_S_ANS_SIGNUP; }

	void Encoding() {
		stream << (int)GetType();
		stream << result;

	}

	void Decoding() {
		stream >> result;
	}
};


class PK_C_REQ_SIGNIN : public Packet {
public:
	char name[NAME_SIZE];
	char password[NAME_SIZE];
public:
	PK_C_REQ_SIGNIN() {
		memset(name, NULL, NAME_SIZE);
		memset(password, NULL, NAME_SIZE);
	}
	PacketType GetType() { return E_C_REQ_SIGNIN; }

	void Encoding() {
		stream << (int)GetType();
		stream << name;
		stream << password;
	}

	void Decoding() {

		stream >> name;
		stream >> password;
	}
};

class PK_S_ANS_SIGNIN : public Packet {
public:
	bool result;
public:
	PK_S_ANS_SIGNIN() {
	}
	PacketType GetType() { return E_S_ANS_SIGNIN; }

	void Encoding() {
		stream << (int)GetType();
		stream << result;
	}

	void Decoding() {
		stream >> result;
	}
};
//E_S_NOTIFY_SERVER = 104,
class PK_S_NOTIFY_SERVER : public Packet_Ex {
public:

	//int accountId;
	int roomNumber;
	char	ip[NAME_SIZE];
	int		port;


public:
	PK_S_NOTIFY_SERVER() {
		memset(ip, NULL, NAME_SIZE);
	}
	PacketType GetType() { return E_S_NOTIFY_SERVER; }

	void Encoding() {
		stream << (int)GetType();
		stream << accountId;
		stream << roomNumber;
		stream << ip;
		stream << port;
	}

	void Decoding() {
		stream >> accountId;
		stream >> roomNumber;
		stream >> ip;
		stream >> port;
	}
};


//E_C_REQ_CONNECT = 105,

class PK_C_REQ_CONNECT : public Packet {
public:

	int accountId;
	char name[NAME_SIZE];

public:

	PK_C_REQ_CONNECT() {
		memset(name, NULL, NAME_SIZE);
	}
	PacketType GetType() { return E_C_REQ_CONNECT; }

	void Encoding() {
		stream << (int)GetType();
		stream << accountId;
		stream << name;
	}

	void Decoding() {
		stream >> accountId;
		stream >> name;
	}
};


//E_S_ANS_CONNECT = 106,
class PK_S_ANS_CONNECT : public Packet {
public:

public:
	PacketType GetType() { return E_S_ANS_CONNECT; }

	void Encoding() {
		stream << (int)GetType();
	}

	void Decoding() {
	}
};
//E_C_REQ_CONNECT_ROOM = 107,
class PK_C_REQ_CONNECT_ROOM : public Packet {
public:
	int accountId;
	int roomNumber;
public:
	PacketType GetType() { return E_C_REQ_CONNECT_ROOM; }

	void Encoding() {
		stream << (int)GetType();
		stream << accountId;
		stream << roomNumber;
	}

	void Decoding() {
		stream >> accountId;
		stream >> roomNumber;
	}
};

//E_S_ANS_CONNECT_ROOM = 108,

class PK_S_ANS_CONNECT_ROOM : public Packet {
public:
	int roomNumber;
	int userNumber;
public:
	PacketType GetType() { return E_S_ANS_CONNECT_ROOM; }

	void Encoding() {
		stream << (int)GetType();
		stream << roomNumber;
		stream << userNumber;
	}

	void Decoding() {
		stream >> roomNumber;
		stream >> userNumber;
		
	}
};

//E_S_NOTIFY_OTHER_CLIENT = 109,

class PK_S_NOTIFY_OTHER_CLIENT : public Packet {
public:
	int userNumber;
	float pos_X;
	float pos_Y;
	bool isDummy;
public:
	PK_S_NOTIFY_OTHER_CLIENT() {
		isDummy = true;
	}
	PacketType GetType() { return E_S_NOTIFY_OTHER_CLIENT; }

	void Encoding() {
		stream << (int)GetType();
		stream << userNumber;
		stream << pos_X;
		stream << pos_Y;
		stream << isDummy;
	}

	void Decoding() {
		stream >> userNumber;
		stream >> pos_X;
		stream >> pos_Y;
		stream >> isDummy;
	}
};
//E_C_REQ_MOVE = 112,

class PK_C_REQ_MOVE : public Packet {
public:
	int roomNumber;
	int userNumber;
	int direction;
	float pos_X;
	float pos_Y;
public:
	PacketType GetType() { return E_C_REQ_MOVE; }

	void Encoding() {
		stream << (int)GetType();
		stream << roomNumber;
		stream << userNumber;
		stream << direction;
		stream << pos_X;
		stream << pos_Y;
	}

	void Decoding() {
		stream >> roomNumber;
		stream >> userNumber;
		stream >> direction;
		stream >> pos_X;
		stream >> pos_Y;
	}
};
//E_S_ANS_MOVE = 113,


class PK_S_ANS_MOVE : public Packet {
public:
	int userNumber;
	int direction;
	float pos_X;
	float pos_Y;
public:
	PacketType GetType() { return E_S_ANS_MOVE; }

	void Encoding() {
		stream << (int)GetType();
		stream << userNumber;
		stream << direction;
		stream << pos_X;
		stream << pos_Y;
	}

	void Decoding() {
		stream >> userNumber;
		stream >> direction;
		stream >> pos_X;
		stream >> pos_Y;
	}
};

class PK_C_NOTIFY_CONNECTED_ROOM : public Packet {
public:
	int roomNumber;
	int userNumber;
public:
	PacketType GetType() { return E_C_NOTIFY_CONNECTED_ROOM; }


	void Encoding() {
		stream << (int)GetType();
		stream << roomNumber;
		stream << userNumber;
	}

	void Decoding() {
		stream >> roomNumber;
		stream >> userNumber;
	}

};

class PK_S_NOTIFY_USER_DATA : public Packet {
public:
	int userNumber;
	float pos_X;
	float pos_Y;
public:

	PacketType GetType() { return E_S_NOTIFY_USER_DATA; }


	void Encoding() {
		stream << (int)GetType();
		stream << userNumber;
		stream << pos_X;
		stream << pos_Y;
	}

	void Decoding() {
		stream >> userNumber;
		stream >> pos_X;
		stream >> pos_Y;
	}

};

class PK_C_REQ_SHOT : public Packet {
public:
	int roomNumber;
	int userNumber;
	int direction;
	int targetNumber;
public:

	PacketType GetType() { return E_C_REQ_SHOT; }


	void Encoding() {
		stream << (int)GetType();
		stream << roomNumber;
		stream << userNumber;
		stream << direction;
		stream << targetNumber;
	}

	void Decoding() {

		stream >> roomNumber;
		stream >> userNumber;
		stream >> direction;
		stream >> targetNumber;
	}

};

class PK_S_ANS_SHOT : public Packet {
public:
	int userNumber;
	int direction;
	int targetNumber;
public:

	PacketType GetType() { return E_S_ANS_SHOT; }


	void Encoding() {
		stream << (int)GetType();
		stream << userNumber;
		stream << direction;
		stream << targetNumber;
	}

	void Decoding() {
		stream >> userNumber;
		stream >> direction;
		stream >> targetNumber;
	}

};

class PK_C_REQ_EXIT_ROOM : public Packet {
public:
	int roomNumber;
	int userNumber;
public:
	PacketType GetType() { return E_C_REQ_EXIT_ROOM; }


	void Encoding() {
		stream << (int)GetType();
		stream << roomNumber;
		stream << userNumber;
	}

	void Decoding() {
		stream >> roomNumber;
		stream >> userNumber;
	}

};

class PK_S_ANS_EXIT_ROOM : public Packet {

public:
	int userNumber;
public:
	PacketType GetType() { return E_S_ANS_EXIT_ROOM; }

	void Encoding() {
		stream << (int)GetType();
		stream << userNumber;
	}

	void Decoding() {
		stream >> userNumber;
	}

};

class PK_S_ACCEPT_EXIT_ROOM : public Packet {

public:
public:
	PacketType GetType() { return E_S_ACCEPT_EXIT_ROOM; }

	void Encoding() {
		stream << (int)GetType();
	}

	void Decoding() {
	
	}

};

class PK_C_REQ_CROSS_OVER_SERVER : public Packet {
public:
	int roomNumber;
	int userNumber;
	int direction;
public:
	PacketType GetType() { return E_C_REQ_CROSS_OVER_SERVER; }



	void Encoding() {
		stream << (int)GetType();
		stream << roomNumber;
		stream << userNumber;
		stream << direction;
	}

	void Decoding() {
		stream >> roomNumber;
		stream >> userNumber;
		stream >> direction;
	}
};
class PK_T_ASK_HOLD_ROOM : public Packet {
public:
	int roomNumber;
public:
	PacketType GetType() { return E_T_ASK_HOLD_ROOM; }
	void Encoding() {
		stream << (int)GetType();
		stream << roomNumber;
	}

	void Decoding() {
		stream >> roomNumber;
	}
}; 

class PK_T_ANS_HOLD_ROOM : public Packet {
public:
	int roomNumber;
public:
	PacketType GetType() { return E_T_ANS_HOLD_ROOM; }



	void Encoding() {
		stream << (int)GetType();
		stream << roomNumber;
	}

	void Decoding() {
		stream >> roomNumber;
	}
};

class PK_T_REQ_CONNECT : public Packet {
public:
	char serverName[NAME_SIZE];
	int terminalNumber;
	char ip[NAME_SIZE];
	int port;

public:
	PK_T_REQ_CONNECT() {

		memset(serverName, NULL, NAME_SIZE);
		memset(ip, NULL, NAME_SIZE);
	}
	PacketType GetType() { return E_T_REQ_CONNECT; }

	void Encoding() {
		stream << GetType();
		stream << serverName;
		stream << terminalNumber;
		stream << ip;
		stream << port;
	}

	void Decoding() {
		stream >> serverName;
		stream >> terminalNumber;
		stream >> ip;
		stream >> port;
	}
};

class PK_T_ANS_CONNECT : public Packet {
public:
	char ip[NAME_SIZE];
	int port;

public:
	PK_T_ANS_CONNECT() {
		memset(ip, NULL, NAME_SIZE);
	}
	PacketType GetType() { return E_T_ANS_CONNECT; }

	void Encoding() {
		stream << GetType();
		stream << ip;
		stream << port;
	}

	void Decoding() {
		stream >> ip;
		stream >> port;
	}
};


class PK_T_ASK_LOAD : public Packet {
public:
	int roomNumber;
	int packetQuantity;
public:
	PK_T_ASK_LOAD() {

	}
	PacketType GetType() { return E_T_ASK_LOAD; }

	void Encoding() {
		stream << GetType();
		stream << roomNumber;
		stream << packetQuantity;
	}

	void Decoding() {
		stream >> roomNumber;
		stream >> packetQuantity;
	}
};

class PK_T_ANS_LOAD : public Packet {
public:
	
	int load;
	int state;
public:
	PK_T_ANS_LOAD() {
	}
	PacketType GetType() { return E_T_ANS_LOAD; }

	void Encoding() {
		stream << GetType();
		stream << load;
		stream << state;
	}

	void Decoding() {
		stream >> load;
		stream >> state;
	}
};

class PK_T_NOTIFY_ROOM : public Packet {
public:
	int roomNumber;
	int roomSize;
	int roomCount;
public:
	PK_T_NOTIFY_ROOM() {

	}

	PacketType GetType() { return E_T_NOTIFY_ROOM; }

	void Encoding() {
		stream << GetType();
		stream << roomNumber;
		stream << roomSize;
		stream << roomCount;
	}

	void Decoding() {

		stream >> roomNumber;
		stream >> roomSize;
		stream >> roomCount;
	}

};


class PK_T_NOTIFY_ROOM_GENERATE_COMPLETE : public Packet {
public:
	int roomNumber;
public:
	PK_T_NOTIFY_ROOM_GENERATE_COMPLETE() {

	}

	PacketType GetType() { return E_T_NOTIFY_ROOM_GENERATE_COMPLETE; }

	void Encoding() {
		stream << GetType();
		stream << roomNumber;
	}

	void Decoding() {
		stream >> roomNumber;
	}



};

class PK_T_NOTIFY_USER : public Packet {
public:
	int accountId;
	char name[NAME_SIZE];
	float pos_X, pos_Y;

	int roomNumber;
	int userNumber;
	int direction;
	bool isReady;
public:

	PK_T_NOTIFY_USER() {
		memset(name, NULL, NAME_SIZE);
	}

	PacketType GetType() { return E_T_NOTIFY_USER; }

	void Encoding() {
		stream << GetType();
		stream << accountId;
		stream << name;
		stream << roomNumber;
		stream << userNumber;
		stream << direction;
		stream << pos_X;
		stream << pos_Y;
		stream << isReady;
	}

	void Decoding() {
		stream >> accountId;
		stream >> name;
		stream >> roomNumber;
		stream >> userNumber;
		stream >> direction;
		stream >> pos_X;
		stream >> pos_Y;
		stream >> isReady;
	}

};

class PK_T_ANS_USER : public Packet {
public:
	int accountId;
	int roomNumber;
	int userNumber;
public:

	PK_T_ANS_USER() {

	}

	PacketType GetType() { return E_T_ANS_USER; }

	void Encoding() {
		stream << GetType();
		stream << accountId;
		stream << roomNumber;
		stream << userNumber;
	}

	void Decoding() {
		stream >> accountId;
		stream >> roomNumber;
		stream >> userNumber;
	}

};

