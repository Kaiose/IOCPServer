#pragma once
#include "stdafx.h"

#define Log(arg, ...) Logger::getInstance().PrintLog(arg,__VA_ARGS__)

class Logger : public Singleton<Logger>{

	//FILE* LogFile;
	
	bool IsFileExisting;
	HANDLE hFile;
	friend Singleton;

	Logger() {
		IsFileExisting = false;
		//fopen_s(&LogFile,"Log.txt", "w");
		//OPEN_EXISTING <-- ������ Open Fail // Error Code : ERROR_FILE_NOT_FOUND
		//GENERIC_WRITE��  �̹� SYNCHRONIZE �� ������
		// �� ���Ͽ� ���� �۾��� ���ؼ� �񵿱�� ���� �߿����� �ʴٰ� ���� -> FILE_FLAG_OVERLAPPED ��������
		// �α��� �ϰ� �Ǹ� ������ �� �ۿ� ���� �ʿ����� �ʴٸ� ����� ���� ����
		hFile = CreateFile(_T("Log.txt"), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			if (GetLastError() != ERROR_FILE_NOT_FOUND) {
				printf("Log File Open Error!");
				//�ʱ�ȭ ���� ���� Singleton��ü�� ���� �����ڿ��� �� �ٽ� Singleton�� �����Ѵٸ� ���ѷ����� ������ �ȴ�.
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

	//std::queue<char*> LoggerQueue;
	
public:
#define TEXTSIZE 1024
	void PrintLog(const char* fmt , ...) {

		
		va_list args;
		va_start(args, fmt);

		tm struct_tm = clock.GetTime();
		int month = struct_tm.tm_mon + 1;
		int day = struct_tm.tm_mday;
		int hour = struct_tm.tm_hour;
		int min = struct_tm.tm_min;


		char logStr[TEXTSIZE] = { 0, };

	
	
		sprintf_s(logStr,"%d �� %d �� %d �� %d �� ", month, day, hour, min);
	
		int logLength = int(strlen(logStr));
		vsprintf_s(&logStr[logLength-1], TEXTSIZE - logLength + 1, fmt, args);
		
		logLength = int(strlen(logStr));
		logStr[logLength] = '\r';
		logStr[logLength + 1] = '\n';
		logStr[logLength + 2] = '\0';

		printf("%s",logStr);


		
		logLength = (int)strlen(logStr);
		if (IsFileExisting) {
			WriteFile(hFile, logStr, logLength , NULL, NULL);
		//	WriteFile(hFile, "\r\n", , NULL, NULL);
		}
		va_end(args);

	}


};