#pragma once

#include "stdafx.h"



class SessionManager : public Singleton<SessionManager>{

private:
	friend Singleton;

	std::unordered_set<Session*> SessionMap;

	bool shutdown;
	CRITICAL_SECTION lock;
	SessionManager() {
		shutdown = false;
		InitializeCriticalSection(&lock);
	}

	~SessionManager() {
		LeaveCriticalSection(&lock);
	}




public:
	
	void FindAbnormalSessionAndDelete() {

		while (shutdown) {

		}

	}

	void AddSession(Session* session) {
		EnterCriticalSection(&lock);
		SessionMap.insert(session);
		LeaveCriticalSection(&lock);
	}
	void DeleteSession(Session* session) {
		EnterCriticalSection(&lock);
		delete session;
		SessionMap.erase(session);
		LeaveCriticalSection(&lock);
	}

};