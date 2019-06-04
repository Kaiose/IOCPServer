#pragma once


#include "stdafx.h"
#include <Windows.h>
#include <Psapi.h>
#include <queue>
#include <Pdh.h>

/*
	System Info 를 가져와서 출력하는 부분과 

	command를 실행하는 부분이 자꾸 충돌하는데

	문법적오류가 아닌 시스템적 오류로 보임

*/
class Work :  public Singleton<Work> {

public:
	std::thread runThread;
	MEMORYSTATUSEX memInfo;
	DWORDLONG totalVirtualMem;
	DWORDLONG totalUsingVirtualMem;
	PROCESS_MEMORY_COUNTERS_EX processMemory;
	SIZE_T UsingVirtualMemoryByProcess;
	DWORDLONG totalPhysicalMemory;
	DWORDLONG totalUsingPhysicalMemory;
	SIZE_T	UsingPhysicalMemoryByProcess;
	HANDLE MsgHandle;

	PDH_HQUERY cpuQuery;
	PDH_HCOUNTER cpuTotal;

	ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
	int numProcessors;
	HANDLE self;

	SYSTEM_INFO sysInfo;
	
	double CpuUsageByProcess;
	double CpuUsage;
	long double currentRamUsage;



	FILETIME ftime, fsys, fuser;
	ULARGE_INTEGER now, sys, user;
	double percent;


	HANDLE hIOCP;
private:
	friend Singleton;

	Work() {
	
		GetSystemInfo(&sysInfo);
		numProcessors = sysInfo.dwNumberOfProcessors;
		GetSystemTimeAsFileTime(&ftime);

		memcpy(&lastCPU, &ftime, sizeof(FILETIME));

		self = GetCurrentProcess();
		GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
		memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
		memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));

		
		
		//Run();
	}
	~Work() {
		runThread.join();
	}

public:
	
	void RegistIOCPHandle(HANDLE handle) {
		hIOCP = handle;
	}

	double getCurrentCpuUsageByProcess() {

		GetSystemTimeAsFileTime(&ftime);
		memcpy(&now, &ftime, sizeof(FILETIME));


		GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
		memcpy(&sys, &fsys, sizeof(FILETIME));
		memcpy(&user, &fuser, sizeof(FILETIME));
		percent = double((sys.QuadPart - lastSysCPU.QuadPart) + (user.QuadPart - lastUserCPU.QuadPart));
		percent /= (now.QuadPart - lastCPU.QuadPart);
		percent /= numProcessors;
		lastCPU = now;
		lastUserCPU = user;
		lastSysCPU = sys;

		return percent * 100;
	}


	

	void getSystemInfo() {

		memInfo.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&memInfo);
	//	totalVirtualMem = memInfo.ullTotalVirtual;
	//	totalUsingVirtualMem = memInfo.ullTotalVirtual - memInfo.ullAvailVirtual;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&processMemory, sizeof(processMemory));
	//	UsingVirtualMemoryByProcess = processMemory.PrivateUsage;

		totalPhysicalMemory = memInfo.ullTotalPhys;
		totalUsingPhysicalMemory = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
		UsingPhysicalMemoryByProcess = processMemory.WorkingSetSize;
		
		/*	printf("Total VitualMemory : %llu		Total Using VirtualMemory : %llu		Using VirtualMemory By Process = %llu\n",
			totalVirtualMem, totalUsingVirtualMem, UsingVirtualMemoryByProcess);

			printf("Total PhysicalMemory : %llu		Total Using PhysicalMemory : %llu		Using PhysicalMemory By Process : %llu\n",
			totalPhysicalMemory, totalUsingPhysicalMemory, UsingPhysicalMemoryByProcess*100);
		*/

		currentRamUsage = ((long double)UsingPhysicalMemoryByProcess / (long double)totalPhysicalMemory);
		//long double currentVirtualMemoryUsage = (long double)(UsingVirtualMemoryByProcess / totalVirtualMem);

		CpuUsageByProcess = getCurrentCpuUsageByProcess();

		printf("Process CPU Usage : %lf %% \n", CpuUsageByProcess);
		printf("Process Physical Memory Usage : %Lf %% \n", currentRamUsage*100);
		
		if (CpuUsageByProcess > 80.0f || currentRamUsage > 70.0f) {
			PostQueuedCompletionStatus(hIOCP, 0, NULL, NULL);
		}
		
	}

	void Run() {
		int ret = 0;
		runThread = std::thread([&]() {
			while (true) {

				ret = MsgWaitForMultipleObjectsEx(0, NULL, 5000, QS_POSTMESSAGE, MWMO_INPUTAVAILABLE);

				if (ret == WAIT_FAILED) {
					printf("Message Wait Failed !! \n");
					return;
				}



				if (ret == WAIT_TIMEOUT) {
					getSystemInfo();
				}

				if (ret == 1) {

				}

			}
		});

	}


};