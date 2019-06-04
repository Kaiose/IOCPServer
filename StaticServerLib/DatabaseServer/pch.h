// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.

#ifndef PCH_H
#define PCH_H

// TODO: 여기에 미리 컴파일하려는 헤더 추가

#endif //PCH_H
#pragma once
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.

#pragma comment(lib,"StaticServerLib.lib")

#pragma comment(lib,"ws2_32.LIB")

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




// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.

#include "Util/Util.h"
#include "Util/Singleton.h"
#include "Database/Database.h"
#include "Util/Clock.h"
#include "Util/Logger.h"


#include "Util/User.h"
#include "Util/UserManager.h"
#include "Util/Room.h"
#include "Util/RoomManager.h"


#include "Packet/PacketClass.h"
#include "Session/Session.h"
#include "Session/SessionManager.h"

#include "Terminal/Terminal.h"
#include "Terminal/TerminalManager.h"

#include "Contents/ContentsProcess.h"
#include "Server/Server.h"

#include "DatabasePacketProcess.h"


#include "Util/Work.h"