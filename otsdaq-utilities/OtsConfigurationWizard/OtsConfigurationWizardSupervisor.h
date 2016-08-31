#ifndef _ots_OtsConfiguraionWizardSupervisor_h
#define _ots_OtsConfiguraionWizardSupervisor_h

#include "otsdaq-core/SOAPUtilities/SOAPMessenger.h"
#include "otsdaq-core/SupervisorConfigurations/SupervisorConfiguration.h"
//#include "otsdaq-core/WebUsersUtilities/RemoteWebUsers.h"

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


class OtsConfigurationWizardSupervisor: public xdaq::Application, public SOAPMessenger
{

public:

    XDAQ_INSTANTIATOR();

    OtsConfigurationWizardSupervisor         (xdaq::ApplicationStub *        ) throw (xdaq::exception::Exception);
    virtual ~OtsConfigurationWizardSupervisor(void                                                              );
    void init                  		  		 (void                                                              );
    void destroy                      		 (void                                                              );

    void Default                      		 (xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    void Verification                 		 (xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    void generateURL                         (                                                                  );
    void printURL();
    void RequestIcons                        (xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    void IconEditor                          (xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    void EditSecurity                        (xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);

    //External Supervisor XOAP handlers
    xoap::MessageReference 		supervisorSequenceCheck 		 (xoap::MessageReference msg) 			throw (xoap::exception::Exception);


private:
    std::string				securityCode_;

};

}

#endif
