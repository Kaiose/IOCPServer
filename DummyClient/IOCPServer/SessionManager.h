#pragma once

#include "stdafx.h"



class SessionManager : public Singleton<SessionManager>{

private:
	friend Singleton;
	int	Session_Id;

	std::string		className;

	std::unordered_map<int,Session*> SessionMap;
	session_map_it it_map;

	CRITICAL_SECTION lock;
	SessionManager() {
		className = "SessionManager";
		Session_Id = 0;
		InitializeCriticalSection(&lock);
	}

	~SessionManager() {
		LeaveCriticalSection(&lock);
	}
public:
	
	void AddSession(Session* session) {
		EnterCriticalSection(&lock);
		SessionMap.insert(std::make_pair(Session_Id, session));
		LeaveCriticalSection(&lock);
	}
	void DeleteSession(Session* session) {
		EnterCriticalSection(&lock);
		for (it_map = SessionMap.begin(); it_map != SessionMap.end(); ++it_map) {
			if (it_map->second == session) {
				SessionMap.erase(it_map->first);
				break;
			}
		}
		LeaveCriticalSection(&lock);
	}
	void DeleteSession(int sessionId) {
		EnterCriticalSection(&lock);
		//없는걸 삭제할시 에러남
		SessionMap.erase(it_map->first);
		LeaveCriticalSection(&lock);

	}

	void CloseSession(Session* session) {
		session->Close();
	}

};