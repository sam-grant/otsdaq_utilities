#include "config/version.h"
#include <xcept/version.h>
#include <xdaq/version.h>
#include "otsdaq-utilities/Logbook/version.h"


GETPACKAGEINFO(Logbook)


//========================================================================================================================
void Logbook::checkPackageDependencies() 
{
	CHECKDEPENDENCY(config);
	CHECKDEPENDENCY(xcept );
	CHECKDEPENDENCY(xdaq  );
}

//========================================================================================================================
std::set<std::string, std::less<std::string> > Logbook::getPackageDependencies()
{
	std::set<std::string, std::less<std::string> > dependencies;

	ADDDEPENDENCY(dependencies,config);
	ADDDEPENDENCY(dependencies,xcept );
	ADDDEPENDENCY(dependencies,xdaq  );

	return dependencies;
}
