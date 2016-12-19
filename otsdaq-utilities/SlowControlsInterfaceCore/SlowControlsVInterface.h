#ifndef _ots_SlowControlsVInterface_h
#define _ots_SlowControlsVInterface_h

#include <string>
#include <array>
namespace ots
{
  
  class SlowControlsVInterface
  {
    
  public:
    SlowControlsVInterface		 (void){} //Nothing happens but it is defined!
    //virtual ~SlowControlsVInterface    (void){} //^	

    
    virtual void initialize        	(                       );
    virtual void destroy           	(                       );
    
    virtual void 	subscribe      	(std::string Name       );
    virtual void 	subscribeJSON  	(std::string List       );
    virtual void 	unsubscribe    	(std::string Name       );
    virtual std::string getList 	(std::string format     );
    virtual std::array<std::string, 4>  getCurrentValue (std::string Name		);
    virtual std::array<std::string, 9>  getSettings     (std::string Name	    );   
  };
  
}
#define DEFINE_OTS_MONITOR(klass) \
  extern "C" \
  SlowControlsVInterface*			\
  make() \
  {\
    return new klass(); \
  }
#endif
