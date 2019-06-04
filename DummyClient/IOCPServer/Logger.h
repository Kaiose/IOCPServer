#pragma once
#include "stdafx.h"

#define Log(arg, ...) Logger::getInstance().PrintLog(arg,__VA_ARGS__)

class Logger : public Singleton<Logger>{

	FILE* LogFile;
	
	bool IsFileExisting;
	HANDLE hFile;
	friend Singleton;

	Logger() {
		IsFileExisting = false;
		//fopen_s(&LogFile,"Log.txt", "w");
		//OPEN_EXISTING <-- 없으면 Open Fail // Error Code : ERROR_FILE_NOT_FOUND
		//GENERIC_WRITE에  이미 SYNCHRONIZE 를 지원함
		// 한 파일에 쓰는 작업에 대해서 비동기는 별로 중요하지 않다고 생각 -> FILE_FLAG_OVERLAPPED 설정안함
		// 로깅을 하게 되면 느려질 수 밖에 없음 필요하지 않다면 만들어 놓지 않음
		hFile = CreateFile(_T("Log.txt"), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			if (GetLastError() != ERROR_FILE_NOT_FOUND) {
				printf("Log File Open Error!");
				//초기화 되지 않은 Singleton객체에 대해 생성자에서 또 다시 Singleton에 접근한다면 무한루프에 빠지게 된다.
				exit(1);
			}

			IsFileExisting = false;
			
		}
		else {
			if (!SetEndOfFile(hFile)) {
				printf("File Move To EndPoint Failed\n");
			}
			IsFileExisting = true;
		}
	}

	~Logger() {
		//fclose(LogFile);

		CloseHandle(hFile);
	}
	
public:

	std::queue<char*> LoggerQueue;
	
public:
#define TEXTSIZE 1024
	void PrintLog(const char* fmt , ...) {

		
		va_list args;
		va_start(args, fmt);

		tm struct_tm = clock.GetTime();
		int month = struct_tm.tm_mon;
		int day = struct_tm.tm_mday;
		int hour = struct_tm.tm_hour;
		int min = struct_tm.tm_min;

		char logStr[TEXTSIZE] = { 0, };

		sprintf_s(logStr,"%d 월 %d 일 %d 시 %d 분 ", month, day, hour, min);

		int logLength = strlen(logStr);
		vsprintf_s(&logStr[logLength], TEXTSIZE - logLength, fmt, args);
		
		logLength = strlen(logStr);
		logStr[logLength] = '\n';
		logStr[logLength + 1] = '\0';
		printf("%s",logStr);

		if (IsFileExisting) {
//			fprintf(LogFile, "%s", logStr);
			WriteFile(hFile, logStr, logLength + 2, NULL, NULL);
		}
		va_end(args);

	}


};