#ifndef _ots_SlowControlsInterface_h
#define _ots_SlowControlsInterface_h

#include <string>
namespace ots
{
  
  class SlowControlsInterface
  {
    
  public:
    SlowControlsInterface			(void)				{;}
	virtual ~SlowControlsInterface  (void) 				{;}

    
    virtual void initialize        	(                                        );
    virtual void destroy           	(                                        );
    
    virtual std::string getList 	(std::string format                      );
    virtual void subscribe      	(std::string Name                        );
    virtual void subscribeJSON  	(std::string List                        );
    virtual void unsubscribe    	(std::string Name                        );
    
  };
  
}

#endif
