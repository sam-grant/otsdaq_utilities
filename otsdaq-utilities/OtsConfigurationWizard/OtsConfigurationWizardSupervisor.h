#ifndef _ots_OtsConfiguraionWizardSupervisor_h
#define _ots_OtsConfiguraionWizardSupervisor_h

#include "otsdaq-core/SOAPUtilities/SOAPMessenger.h"
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

#include "otsdaq-core/SupervisorDescriptorInfo/SupervisorDescriptorInfo.h"

namespace ots
{


class OtsConfigurationWizardSupervisor: public xdaq::Application, public SOAPMessenger
{

public:

    XDAQ_INSTANTIATOR();

    OtsConfigurationWizardSupervisor         						(xdaq::ApplicationStub *) throw (xdaq::exception::Exception);
    virtual ~OtsConfigurationWizardSupervisor						(void);


    void 						init                  				(void);
    void 						destroy                     		(void);

    void 						generateURL            				(void);
    static void 				printURL							(OtsConfigurationWizardSupervisor *ptr, std::string securityCode);

    void 						Default                    			(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    void 						verification               		 	(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    void						requestIcons                		(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);

    void 						editSecurity                		(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    void 						tooltipRequest                		(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    void                                                toggleSecurityCodeGeneration            (xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    //External Supervisor XOAP handlers
    xoap::MessageReference 		supervisorSequenceCheck 			(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference 		supervisorLastConfigGroupRequest	(xoap::MessageReference msg) throw (xoap::exception::Exception);

private:
    std::string					securityCode_;
    bool                                        defaultSequence_;
};

}

#endif
