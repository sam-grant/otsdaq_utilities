#include "otsdaq-utilities/SlowControlsInterfacePlugins/SlowControlsOtsInterface.h"


using namespace ots;

SlowControlsOtsInterface::SlowControlsOtsInterface()// : SlowControlsVInterface()
{

}

SlowControlsOtsInterface::~SlowControlsOtsInterface()
{
  destroy();
}

void SlowControlsOtsInterface::initialize()
{
}

void SlowControlsOtsInterface::destroy()
{
}

std::string SlowControlsOtsInterface::getList(std::string format)
{
  return (std::string) "list";
}
void SlowControlsOtsInterface::subscribe(std::string Name)
{
}

void SlowControlsOtsInterface::subscribeJSON(std::string List)
{
}

void SlowControlsOtsInterface::unsubscribe(std::string Name)
{
}

std::array<std::string, 4>  SlowControlsOtsInterface::getCurrentValue (std::string Name)
{
  return {"a","b","c","d"};
}

std::array<std::string, 9>  SlowControlsOtsInterface::getSettings (std::string Name)
{
  return {"a","b","c","d","e","f","g","h","i"};
}   
 
DEFINE_OTS_MONITOR(SlowControlsOtsInterface)
