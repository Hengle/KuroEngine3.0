#pragma once
#include<vector>
#include<string>
class Observer;
class Subject
{
	std::vector<Observer*>observers;
protected:
	void Notify(const std::string& EventKey);
public:
	void AddObserver(Observer* Arg) { observers.emplace_back(Arg); }
	void RemoveObserver(Observer* Arg) 
	{
		for (auto itr = observers.begin(); itr != observers.end(); ++itr)
		{
			if (*itr == Arg)
			{
				observers.erase(itr);
				break;
			}
		}
	}
};