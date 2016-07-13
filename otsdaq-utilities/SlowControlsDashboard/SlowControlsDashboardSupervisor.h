#ifndef _ots_SlowControlsDashboardSupervisor_h
#define _ots_SlowControlsDashboardSupervisor_h

#include "otsdaq-core/SOAPUtilities/SOAPMessenger.h"
#include "otsdaq-core/SupervisorConfigurations/SupervisorConfiguration.h"
#include "otsdaq-core/WebUsersUtilities/RemoteWebUsers.h"

#include "xdaq/Application.h"
#include "xgi/Method.h"

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPBody.h"
#include "xoap/domutils.h"
#include "xoap/Method.h"


#include "cgicc/HTMLClasses.h"
#include <cgicc/HTTPCookie.h>
#include "cgicc/HTMLDoctype.h"
#include <cgicc/HTTPHeader.h>

#include <string>
#include <map>



namespace ots
{

	class SlowControlsInterface;

class SlowControlsDashboardSupervisor: public xdaq::Application, public SOAPMessenger
{

public:

    XDAQ_INSTANTIATOR();

    SlowControlsDashboardSupervisor              (xdaq::ApplicationStub *        ) throw (xdaq::exception::Exception);
    virtual ~SlowControlsDashboardSupervisor     (void                                                              );
    void init                  		  	 			(void                                                              );
void destroy                      		 			(void                                                              );
	void requestHandler	            		 		(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);	
	void Default                      		 		(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    void GetList                                 	(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception); 
    void Subscribe                               	(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    void Unsubscribe                             	(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
 

private:
	SlowControlsInterface * interface_;

};

}

#endif
