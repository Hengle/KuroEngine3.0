#include"Subject.h"
#include"Observer.h"

void Subject::Notify(const std::string& EventKey)
{
	for (auto ptr : observers)
	{
		ptr->OnNotify(EventKey);
	}
}