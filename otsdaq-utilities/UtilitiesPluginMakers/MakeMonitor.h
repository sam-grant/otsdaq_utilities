#ifndef _ots_MakeMonitor_h_
#define _ots_MakeMonitor_h_
// Using LibraryManager, find the correct library and return an instance of the specified Slow Controls Interface.

#include <string>

namespace ots
{
class SlowControlsVInterface;

 SlowControlsVInterface* makeMonitor(std::string const& monitorPluginName);

}

#endif /* _ots_MakeMonitor_h_ */
