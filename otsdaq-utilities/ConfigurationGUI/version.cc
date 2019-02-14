#include "config/version.h"
#include <xcept/version.h>
#include <xdaq/version.h>
#include "otsdaq-utilities/ConfigurationGUI/version.h"


GETPACKAGEINFO(ConfigurationGUI)


//========================================================================================================================
void ConfigurationGUI::checkPackageDependencies() 
{
	CHECKDEPENDENCY(config);
	CHECKDEPENDENCY(xcept );
	CHECKDEPENDENCY(xdaq  );
}

//========================================================================================================================
std::set<std::string, std::less<std::string> > ConfigurationGUI::getPackageDependencies()
{
	std::set<std::string, std::less<std::string> > dependencies;

	ADDDEPENDENCY(dependencies,config);
	ADDDEPENDENCY(dependencies,xcept );
	ADDDEPENDENCY(dependencies,xdaq  );

	return dependencies;
}
