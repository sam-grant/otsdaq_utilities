#include "config/version.h"
#include <xcept/version.h>
#include <xdaq/version.h>
#include "otsdaq-utilities/Console/version.h"


GETPACKAGEINFO(Console)


//========================================================================================================================
void Console::checkPackageDependencies() 
{
	CHECKDEPENDENCY(config);
	CHECKDEPENDENCY(xcept );
	CHECKDEPENDENCY(xdaq  );
}

//========================================================================================================================
std::set<std::string, std::less<std::string> > Console::getPackageDependencies()
{
	std::set<std::string, std::less<std::string> > dependencies;

	ADDDEPENDENCY(dependencies,config);
	ADDDEPENDENCY(dependencies,xcept );
	ADDDEPENDENCY(dependencies,xdaq  );

	return dependencies;
}
