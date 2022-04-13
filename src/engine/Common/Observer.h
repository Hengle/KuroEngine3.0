#pragma once
#include<string>
class Observer
{
public:
	virtual ~Observer() {}
	virtual void OnNotify(const std::string& EventKey) = 0;
};