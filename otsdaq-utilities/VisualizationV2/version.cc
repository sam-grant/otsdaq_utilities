#include "config/version.h"
#include <xcept/version.h>
#include <xdaq/version.h>
#include "otsdaq-utilities/VisualizationV2/version.h"

GETPACKAGEINFO(VisualizationV2)

//==============================================================================
void VisualizationV2::checkPackageDependencies()
{
	CHECKDEPENDENCY(config);
	CHECKDEPENDENCY(xcept);
	CHECKDEPENDENCY(xdaq);
}

//==============================================================================
std::set<std::string, std::less<std::string> > VisualizationV2::getPackageDependencies()
{
	std::set<std::string, std::less<std::string> > dependencies;

	ADDDEPENDENCY(dependencies, config);
	ADDDEPENDENCY(dependencies, xcept);
	ADDDEPENDENCY(dependencies, xdaq);

	return dependencies;
}
