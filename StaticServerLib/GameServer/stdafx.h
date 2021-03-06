// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//


#pragma once
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.

#pragma comment(lib,"StaticServerLib.lib")

#pragma comment(lib,"ws2_32.LIB")

#include "targetver.h"
#include <iostream>
#include <stdio.h>
#include <tchar.h>

#include <wchar.h>
#include <string.h>
//#include <Windows.h>
//#include <mstcpip.h>
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <time.h>
#include <queue>
#include <stack>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <MSWSock.h>
#include <thread>
#include <utility>
#include <set>
#include <string.h>

#include <mutex>

#include "Util/tinyXml\tinystr.h"


#include "Util/tinyXml\tinyxml.h"

//using namespace std;


//#include "Profile.h"

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.

#include "Util/Util.h"
#include "Util/Singleton.h"
#include "Database/Database.h"
#include "Util/Clock.h"
#include "Util/Logger.h"
#include "Util/Work.h"
#include "Util/Profile.h"

#include "Util/User.h"
#include "Util/UserManager.h"
#include "Util/Room.h"
#include "Util/LOCK.h"
#include "Util/RoomManager.h"


#include "Packet/PacketClass.h"
#include "Session/Session.h"
#include "Session/SessionManager.h"

#include "Terminal/Terminal.h"
#include "Terminal/TerminalManager.h"

#include "Contents/ContentsProcess.h"
#include "Server/Server.h"
#include "Util/Monitor.h"
#include "GamePacketProcess.h"






// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.


// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
