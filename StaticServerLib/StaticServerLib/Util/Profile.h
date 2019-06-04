#pragma once

#include "stdafx.h"
#include <Windows.h>
#include <string>
#include <chrono>

#define MAX_PROFILE_SAMPLES 5000
using namespace std::chrono;

typedef struct _PROFILE_SAMPLE_HISTORY {
	bool      valid;               //
	wchar_t      name[_MAX_PATH];
	double      average;
	double      minimum;
	double      maximum;
}PROFILE_SAMPLE_HISTORY;


typedef struct _PROFILE_SAMPLE {
	bool      valid;
	UINT      profileInstance;      // Profile 의 깊이를 확인
	INT32      openProfiles;         // Profile 의 깊이를 확인
	wchar_t      name[_MAX_PATH];
	system_clock::time_point      startTime;
	std::chrono::milliseconds duringTime;
	UINT      numberOfParents;      // 상위 Parentrk 몇 개 존재하는지를 저장.

}PROFILE_SAMPLE;




class Profile : public Singleton<Profile> {

	friend Singleton;
private:

	PROFILE_SAMPLE_HISTORY profileSampleHistories[MAX_PROFILE_SAMPLES];
	PROFILE_SAMPLE profileSamples[MAX_PROFILE_SAMPLES];

	bool      valid;
	int         totalCount;
	int         currentCount;

	
	system_clock::time_point beginProfile;
	system_clock::time_point endProfile;
public:

	Profile() {
		Initialize();
	}

	void Initialize() {
	
	}

	system_clock::time_point getExactTime() {
		
		return system_clock::now();
	}

	void Begin(const wchar_t* name) {
		INT i = 0;

		while (i < MAX_PROFILE_SAMPLES && profileSamples[i].valid == true) {
			if (wcscmp(profileSamples[i].name, name) == 0 && profileSamples[i].valid == true) {
				profileSamples[i].openProfiles++;
				profileSamples[i].profileInstance++;

				return;
			}

			i++;
		}

		if (i >= MAX_PROFILE_SAMPLES) {
			return;
		}

		wcscpy_s(profileSamples[i].name, name);

		profileSamples[i].valid = true;
		profileSamples[i].openProfiles = 1;
		profileSamples[i].profileInstance = 1;
		//profileSamples[i].duringTime = 0;
		profileSamples[i].startTime = getExactTime();
		//profileSamples[i].childrenSampleTime = 0.0f;
	}

	void End(const wchar_t* name) {
		/**
		int i = 0, numberOfParents = 0;
		while (i < MAX_PROFILE_SAMPLES && profileSamples[i].valid == true) {
			if (wcscmp(profileSamples[i].name, name) == 0) {
				int inner = 0;
				int parent = -1;

				system_clock::time_point endTime = getExactTime();
				profileSamples[i].openProfiles--;


				while (profileSamples[inner].valid == true) {

					if (profileSamples[inner].openProfiles > 0) {
						numberOfParents++;
						if (parent < 0) {
							parent = inner;
						}
						else if (profileSamples[i].startTime >= profileSamples[parent].startTime) {
							parent = inner;
						}
						inner++;
					}

				}

				profileSamples[i].numberOfParents = numberOfParents;


				profileSamples[i].duringTime += (endTime - profileSamples[i].startTime);

				return;
			}
			i++;
		}
		*/
	}

	void DumpProfileOutput() {
		int i = 0;
		endProfile = getExactTime();
		std::chrono::milliseconds mill = std::chrono::duration_cast<std::chrono::milliseconds>(endProfile - profileSamples[i].startTime);

		wprintf(L"      Time        :    Profile Name\n");
		wprintf(L"------------------------------------------------------------\n");

		while (i < MAX_PROFILE_SAMPLES && profileSamples[i].valid == TRUE) {
	
			std::cout << mill.count() << "  milliseconds       :     ";
			wprintf(L"%s\n", profileSamples[i].name);


			Work::getInstance().getSystemInfo();


			profileSamples[i].startTime = getExactTime();
			profileSamples[i].valid = TRUE;
			
			i++;

		}


		for (i = 0; i < MAX_PROFILE_SAMPLES; i++)
			profileSamples[i].valid = FALSE;

	}

	void DumpProfileOutput(int sessionCount, int& byte) {
		int i = 0;
		endProfile = getExactTime();
		std::chrono::milliseconds mill = std::chrono::duration_cast<std::chrono::milliseconds>(endProfile - profileSamples[i].startTime);
		if (mill.count() < 10000) {
			return;
		}


		TCHAR   indentedName[256] = { 0, };
		
		wprintf(L"  Session Count     :      Byte     :       Time                    :      Profile Name\n");
		wprintf(L"-------------------------------------------------------------------------------------------\n");

		while (i < MAX_PROFILE_SAMPLES && profileSamples[i].valid == TRUE) {
			
			std::cout << "        " << sessionCount << "       :       " << byte << "       :      " << mill.count() << "  milliseconds       :     ";
			wprintf(L"%s\n", profileSamples[i].name);
			

			Work::getInstance().getSystemInfo();


			profileSamples[i].startTime = getExactTime();
			profileSamples[i].valid = TRUE;
			byte = 0;

			i++;

		}


		/*for (i = 0; i < MAX_PROFILE_SAMPLES; i++)
			profileSamples[i].valid = FALSE;
*/

		//	beginProfile = getExactTime();


		//	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BACKGROUND_RED | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);
	}

	void storeProfileInHistory(const wchar_t* lpwName, double percent) {
		int i = 0;
		double oldRatio = 0.0;
		double newRatio = 0.0;

		if (newRatio > 1.0)
			newRatio = 1.0;

		oldRatio = 1.0 - newRatio;
		while (i < MAX_PROFILE_SAMPLES && profileSampleHistories[i].valid == true) {
			if (wcscmp(profileSampleHistories[i].name, lpwName) == 0) {
				profileSampleHistories[i].average = (profileSampleHistories[i].average * oldRatio) + (percent*newRatio);

				if (percent < profileSampleHistories[i].minimum)
					profileSampleHistories[i].minimum = percent;
				else {
					profileSampleHistories[i].minimum = (profileSampleHistories[i].minimum * oldRatio) + (percent * newRatio);
				}
				if (profileSampleHistories[i].minimum < 0.0)
					profileSampleHistories[i].maximum = 0.0;

				if (percent > profileSampleHistories[i].maximum) {
					profileSampleHistories[i].maximum = percent;
				}
				else
					profileSampleHistories[i].maximum = (profileSampleHistories[i].maximum * oldRatio) + (percent* newRatio);
				return;
			}
			i++;
		}

		if (i < MAX_PROFILE_SAMPLES) {
			wcsncpy_s(profileSampleHistories[i].name, lpwName, MAX_PATH);

			profileSampleHistories[i].valid = TRUE;
			profileSampleHistories[i].average = profileSampleHistories[i].minimum = profileSampleHistories[i].maximum = percent;
		}



	}

	void getProfileFromHistory(const wchar_t* lpwName, double& average, double& minimum, double& maximum) {
		int i = 0;
		while (i < MAX_PROFILE_SAMPLES && profileSampleHistories[i].valid == true) {
			if (wcscmp(profileSampleHistories[i].name, lpwName) == 0) {
				average = profileSampleHistories[i].average;
				minimum = profileSampleHistories[i].minimum;
				maximum = profileSampleHistories[i].maximum;
				return;
			}
			i++;
		}
		average = minimum = maximum = 0.0;
	}






};