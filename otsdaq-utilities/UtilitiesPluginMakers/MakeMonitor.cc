#include "otsdaq-utilities/UtilitiesPluginMakers/MakeMonitor.h"
#include "otsdaq-utilities/SlowControlsInterfaceCore/SlowControlsVInterface.h"

#include <cetlib/BasicPluginFactory.h>

namespace ots
{
 SlowControlsVInterface* makeMonitor(std::string const& monitorPluginName)
{
	static cet::BasicPluginFactory basicPluginInterfaceFactory("monitor", "make");

	return basicPluginInterfaceFactory.makePlugin<SlowControlsVInterface*> (monitorPluginName);
}
}


