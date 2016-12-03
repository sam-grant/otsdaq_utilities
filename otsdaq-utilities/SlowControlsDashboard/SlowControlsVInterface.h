#ifndef _ots_SlowControlsVInterface_h
#define _ots_SlowControlsVInterface_h

#include <string>
namespace ots
{
  
  class SlowControlsVInterface
  {
    
  public:
    SlowControlsVInterface			(void)				{;}
	virtual ~SlowControlsVInterface  (void) 				{;}

    
    virtual void initialize        	(                                        );
    virtual void destroy           	(                                        );
    
    virtual std::string getList 	(std::string format                      );
    virtual void subscribe      	(std::string Name                        );
    virtual void subscribeJSON  	(std::string List                        );
    virtual void unsubscribe    	(std::string Name                        );
    
  };
  
}

#endif
