#ifndef _ots_SlowControlsOtsInterface_h
#define _ots_SlowControlsOtsInterface_h

#include "otsdaq-core/NetworkUtilities/ReceiverSocket.h"  // Make sure this is always first because <sys/types.h> (defined in Socket.h) must be first
#include "otsdaq-utilities/SlowControlsDashboard/SlowControlsVInterface.h"
#include "otsdaq-core/ConfigurationInterface/Configurable.h"

#include <string>
namespace ots
{
  
  class SlowControlsOtsInterface : public SlowControlsVInterface//, public Configurable, public ReceiverSocket
  {
    
  public:
    SlowControlsOtsInterface			(void)
//  :Configurable                	(theXDAQContextConfigTree, configurationPath)
  	  {;}
	~SlowControlsOtsInterface  			(void) 				{;}

    
    void initialize        	(                                        );
    void destroy           	(                                        );
    
    std::string getList 	(std::string format                      );
    void subscribe      	(std::string Name                        );
    void subscribeJSON  	(std::string List                        );
    void unsubscribe    	(std::string Name                        );
    
  };
  
}

#endif
