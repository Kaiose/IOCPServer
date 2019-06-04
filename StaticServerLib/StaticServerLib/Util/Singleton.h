#pragma once

template<class T>
class Singleton
{
private:

protected:
	Singleton(const Singleton&) = delete;
	Singleton(const Singleton&&) = delete;
	Singleton& operator = (const Singleton&) = delete;
	Singleton& operator = (const Singleton&&) = delete;

	Singleton() = default;
	~Singleton() = default;

public:
	
	static T& getInstance()
	{	
		static T instance;
		return instance;
	}

};