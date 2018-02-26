#ifndef _ots_MacroMakerSupervisor_h_
#define _ots_MacroMakerSupervisor_h_

#include "otsdaq-core/SOAPUtilities/SOAPMessenger.h"
#include "otsdaq-core/WebUsersUtilities/RemoteWebUsers.h"
#include "otsdaq-core/CoreSupervisors/FESupervisor.h"
#include "otsdaq-core/SupervisorDescriptorInfo/SupervisorDescriptorInfo.h"

#include <xdaq/Application.h>
#include <xgi/Method.h>

#include <xoap/MessageReference.h>
#include <xoap/MessageFactory.h>
#include <xoap/SOAPEnvelope.h>
#include <xoap/SOAPBody.h>
#include <xoap/domutils.h>
#include <xoap/Method.h>


#include <cgicc/HTMLClasses.h>
#include <cgicc/HTTPCookie.h>
#include <cgicc/HTMLDoctype.h>
#include <cgicc/HTTPHeader.h>

#include <string>
#include <map>
#include "otsdaq-core/SupervisorInfo/AllSupervisorInfo.h"

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

    //	void printStatus();

	void handleRequest				(const std::string Command, HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username, const uint8_t userPermissions);
	void getFElist					(HttpXmlDocument& xmldoc);
	void getFEMacroList				(HttpXmlDocument& xmldoc, const std::string &username);

	void writeData					(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username);
	void readData					(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username);
	void createMacro				(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username);
	void loadMacros					(HttpXmlDocument& xmldoc, const std::string &username);
	void appendCommandToHistory		(std::string command, std::string Format, std::string time, std::string interfaces, const std::string &username);
	void loadHistory				(HttpXmlDocument& xmldoc, const std::string &username);
	void deleteMacro				(HttpXmlDocument& xmldoc,cgicc::Cgicc& cgi, const std::string &username);
	void editMacro					(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username);
	void clearHistory				(const std::string &username);
	void exportMacro				(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username);
	void runFEMacro					(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi);


	std::string generateHexArray	(const std::string& sourceHexString,int &numOfBytes);

    AllSupervisorInfo 						allSupervisorInfo_;
    RemoteWebUsers							theRemoteWebUsers_;
    //FESupervisor*			 				theFESupervisor_;
	//ConfigurationManager*   				theConfigurationManager_;
	SupervisorInfoMap						allFESupervisorInfo_;

};

}

#endif
