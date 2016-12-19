#ifndef _ots_SlowControlsOtsInterface_h
#define _ots_SlowControlsOtsInterface_h

#include "otsdaq-core/NetworkUtilities/ReceiverSocket.h"  // Make sure this is always first because <sys/types.h> (defined in Socket.h) must be first
#include "otsdaq-utilities/SlowControlsInterfaceCore/SlowControlsVInterface.h"
#include <string>
#include <array>
namespace ots
{
  
  class SlowControlsOtsInterface : public SlowControlsVInterface//, public Configurable, public ReceiverSocket
  {
    
  public:
    SlowControlsOtsInterface		();
    ~SlowControlsOtsInterface  		();				

    
    void initialize        	(                                        );
    void destroy           	(                                        );
    
    std::string getList 	(std::string format                      );
    void subscribe      	(std::string Name                        );
    void subscribeJSON  	(std::string List                        );
    void unsubscribe    	(std::string Name                        );
    std::array<std::string, 4>  getCurrentValue (std::string Name	 );
    std::array<std::string, 9>  getSettings     (std::string Name	 );   
  };
  
}

#endif
