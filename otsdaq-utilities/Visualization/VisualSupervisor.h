#ifndef _ots_VisualSupervisor_h
#define _ots_VisualSupervisor_h

#include "otsdaq-core/SOAPUtilities/SOAPMessenger.h"
#include "otsdaq-core/SupervisorConfigurations/SupervisorConfiguration.h"
#include "otsdaq-core/FiniteStateMachine/RunControlStateMachine.h"
#include "otsdaq-core/WebUsersUtilities/RemoteWebUsers.h"
#include "otsdaq-utilities/Visualization/VisualDataManager.h"

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

class ConfigurationManager;
class ConfigurationKey;

class VisualSupervisor: public xdaq::Application, public SOAPMessenger, public RunControlStateMachine
{

public:

    XDAQ_INSTANTIATOR();

    VisualSupervisor            	(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);
    virtual ~VisualSupervisor   	(void);
    void init                  		(void);
    void destroy               		(void);

    void 						Default               		(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    void 						request                     (xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    void 						safari               		(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
                              
    void stateRunning          (toolbox::fsm::FiniteStateMachine& fsm) throw (toolbox::fsm::exception::Exception);

    void transitionConfiguring (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
    void transitionHalting     (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
    void transitionStarting    (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
    void transitionStopping    (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);

private:

    void				binaryBufferToHexString(char *buff, unsigned int len, std::string& dest);

    const std::string                    supervisorType_;
    const unsigned int                   supervisorInstance_;
    SupervisorConfiguration              theSupervisorsConfiguration_;
    RemoteWebUsers                       theRemoteWebUsers_;
    ConfigurationManager*                theConfigurationManager_;
    VisualDataManager*                   theDataManager_;
    std::shared_ptr<ConfigurationKey>    theConfigurationKey_;

    unsigned int 			             loadedRunNumber_;

};

}

#endif
