#ifndef PAGE_HANDLER_H__
#define PAGE_HANDLER_H__

#include <string>

class PageHandler
{
public:
	virtual ~PageHandler() {}
	virtual void HandlePage(std::string& html) = 0;
	virtual void AfficherResultat() = 0;
};

#endif