// IOCPServer.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"

int main()
{
	//status = START;
	PacketProcess* packetProcess = new PacketProcess();
	// <-- 여기 문제가있음.
	Server* server = new Server(packetProcess);
	//Work::getInstance().RegistIOCPHandle(server->hIOCP);
	//Monitor::getInstance().Initialize("127.0.0.1", 8000);
	//Server* server = new Server(NULL);
	TiXmlDocument config;
	config.LoadFile("config.xml");

	TiXmlElement* plotlyServer = config.FirstChildElement("Server")->FirstChildElement("PlotlyServer");
	TiXmlElement* plotlyServerIp = plotlyServer->FirstChildElement("IP");
	Monitor::getInstance().Initialize(plotlyServerIp->GetText(), 8000);

	
	Database db;

	//db.DBConnect();
	//db.AttributeBind();
	//db.Show();
	//db.Insert(L"testUser1", L"1234");
	//db.Show();
	////



	Log("Server Start");

	char command[100];
	while (true) {
		memset(command, NULL, sizeof(char) * 100);

		std::cin >> command;

		if (strcmp(command, "Exit") == 0) {
			status = STOP;
			break;
		}

		if (strcmp(command, "Show") == 0) {
			server->ShowPool();
		}

		if (strcmp(command, "Release") == 0) {
			server->Release_Resources();
		}

		if (strcmp(command, "Increase") == 0) {
			server->Create_Resources();
		}
	}
	return 0;
}
