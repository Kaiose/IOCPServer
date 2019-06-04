#include "stdafx.h"
#include "Clock.h"



Clock::Clock()
{
	className = "Clock";


}


Clock::~Clock()
{
}


tm Clock::GetTime() {
	time_t now = time(NULL);
	tm tm_struct;

	localtime_s(&tm_struct,&now);

	return tm_struct;
}
