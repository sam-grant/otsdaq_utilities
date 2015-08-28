#ifndef _ots_OtsConfiguraionWizard_h
#define _ots_OtsConfiguraionWizard_h

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
//#include "otsdaq/otsdaq-core/SOAPUtilities/SOAPMessenger.h"
#include "otsdaq-core/SOAPUtilities/SOAPMessenger.h"
#include "otsdaq-core/WebUsersUtilities/WebUsers.h"
#include "otsdaq-core/SystemMessenger/SystemMessenger.h"
#include "otsdaq-core/WorkLoopManager/WorkLoopManager.h"
#include "otsdaq-core/FiniteStateMachine/RunControlStateMachine.h"
#include "otsdaq-core/SupervisorConfigurations/SupervisorConfiguration.h"
#include "otsdaq-core/Supervisor/SupervisorsInfo.h"


#include <xdaq/Application.h>
//#include <toolbox/fsm/FiniteStateMachine.h>
#include <toolbox/task/WorkLoop.h>
#include <xgi/Method.h>
#include <xdata/String.h>

#include <string>
#include <set>

namespace ots
{


class OtsConfigurationWizard: public xdaq::Application, public SOAPMessenger
{

public:

    XDAQ_INSTANTIATOR();

    OtsConfigurationWizard            (xdaq::ApplicationStub * s                                         ) throw (xdaq::exception::Exception);
    virtual ~OtsConfigurationWizard   (void                                                              );
    void init                  		  (void                                                              );
    void destroy                      (void                                                              );
    void Default                      (xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
                              

private:

};

}

#endif
