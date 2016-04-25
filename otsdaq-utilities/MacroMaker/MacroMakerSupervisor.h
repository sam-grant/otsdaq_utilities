#ifndef _ots_MacroMakerSupervisor_h
#define _ots_MacroMakerSupervisor_h

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

class HttpXmlDocument;


class MacroMakerSupervisor: public xdaq::Application, public SOAPMessenger
{

public:

    XDAQ_INSTANTIATOR();

    MacroMakerSupervisor            (xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);
    virtual ~MacroMakerSupervisor   (void);

    void init                  		(void);
    void destroy                    (void);
    void Default               		(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    void MacroMakerRequest          (xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);

private:
    SupervisorConfiguration              	theSupervisorsConfiguration_;
    RemoteWebUsers							theRemoteWebUsers_;

//	void printStatus();
	void handleRequest(const std::string Command, HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi);
	void writeData(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi);
//	void readData(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi);
//
//
//private:
//	SupervisorConfiguration* superConfiguration_;
//	SupervisorsInfo* 		 superInfo_;
//	FEWInterfacesManager*    theFEWInterfacesManager_;
//	ConfigurationManager*    theConfigurationManager_;
//
//
    void getFElist(HttpXmlDocument& xmldoc);

};

}

#endif
