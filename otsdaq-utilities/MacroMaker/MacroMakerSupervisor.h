#ifndef _ots_MacroMakerSupervisor_h_
#define _ots_MacroMakerSupervisor_h_

#include "otsdaq-core/SOAPUtilities/SOAPMessenger.h"
#include "otsdaq-core/WebUsersUtilities/RemoteWebUsers.h"
#include "otsdaq-core/CoreSupervisors/FESupervisor.h"
#include "otsdaq-core/SupervisorDescriptorInfo/SupervisorDescriptorInfo.h"


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

    //xoap GetMacroList (username) //give macro list for user's and public
    //xoap RunMacro(macropath)		//get back unique id?
    //xoap getProgressOfRunningMacro(uid) //100%

private:
    SupervisorDescriptorInfo              	theSupervisorDescriptorInfo_;
    RemoteWebUsers							theRemoteWebUsers_;
    FESupervisor*			 				theFESupervisor_;
//	void printStatus();
	void handleRequest(const std::string Command, HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi);
	void getFElist(HttpXmlDocument& xmldoc);

	void writeData(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi);
	void readData(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi);
	void createMacro(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi);
	void loadMacros(HttpXmlDocument& xmldoc);
	void appendCommandToHistory(std::string command, std::string Format, std::string time, std::string interfaces);
	void loadHistory(HttpXmlDocument& xmldoc);
	void deleteMacro(HttpXmlDocument& xmldoc,cgicc::Cgicc& cgi);
	void editMacro(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi);
	void clearHistory();
	void exportMacro(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi);
	std::vector<std::string> interfaceList;
	std::string username;
	uint8_t userPermissions;

	std::string generateHexArray(const std::string& sourceHexString,int &numOfBytes);


	//generateInterfaceCodeFromMacro
	// 	input
		// macro name
		// user
	//  from
		// out c++ to the ..../MacroCodeGen/<user>/<macro>.cc
				//  //comment generated from macro user/macro on this date
				// universalread(0xff,0xfa);
//
//
//	SupervisorConfiguration* superConfiguration_;
//	SupervisorsInfo* 		 superInfo_;
//	FEWInterfacesManager*    theFEWInterfacesManager_;
	ConfigurationManager*    theConfigurationManager_;

//
//


};

}

#endif
