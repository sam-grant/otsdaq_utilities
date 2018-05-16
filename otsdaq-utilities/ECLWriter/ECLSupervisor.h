#ifndef _ots_ECLSupervisor_h
#define _ots_ECLSupervisor_h

#include "otsdaq-core/SOAPUtilities/SOAPMessenger.h"
#include "otsdaq-core/FiniteStateMachine/RunControlStateMachine.h"
#include "otsdaq-core/WebUsersUtilities/RemoteWebUsers.h"

#include <xdaq/Application.h>
#include "otsdaq-core/Macros/XDAQApplicationMacros.h"
#include <xgi/Method.h>

#include <cgicc/HTMLClasses.h>
#include <cgicc/HTTPCookie.h>
#include <cgicc/HTMLDoctype.h>
#include <cgicc/HTTPHeader.h>

#include <string>
#include <map>
#include <chrono>
#include "otsdaq-core/CoreSupervisors/CoreSupervisorBase.h"



namespace ots
{

class ConfigurationManager;
class ConfigurationGroupKey;

class ECLSupervisor: public CoreSupervisorBase
{

public:

    XDAQ_INSTANTIATOR();

    ECLSupervisor            	(xdaq::ApplicationStub * s) ;
    virtual ~ECLSupervisor   	(void);
    void init                  		(void);
    void destroy               		(void);

    void 						defaultPage                 (xgi::Input* in, xgi::Output* out) ;

    void 						transitionConfiguring 		(toolbox::Event::Reference e) ;
    void 						transitionStarting    		(toolbox::Event::Reference e) ;
    void 						transitionStopping    		(toolbox::Event::Reference e) ;
    void 						transitionPausing	  		(toolbox::Event::Reference e) ;
    void 						transitionResuming	  		(toolbox::Event::Reference e) ;

	xoap::MessageReference 		MakeSystemLogbookEntry(xoap::MessageReference msg) 			;

private:

    ConfigurationManager*                	theConfigurationManager_;
 	const std::string                    	supervisorContextUID_;
	const std::string                    	supervisorApplicationUID_;
	const std::string                    	supervisorConfigurationPath_;


	std::string ECLUser;
	std::string ECLHost;
	std::string ECLPwd;
	std::string ExperimentName;
	std::string run;
	std::chrono::steady_clock::time_point run_start;

	std::string EscapeECLString(std::string input = "");

	int Write(bool atEnd, bool pause);
};

}

#endif
