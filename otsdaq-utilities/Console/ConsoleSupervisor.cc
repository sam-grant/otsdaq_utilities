#include "otsdaq-utilities/Console/ConsoleSupervisor.h"
#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/Macros/CoutHeaderMacros.h"
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"
//#include "otsdaq-core/SOAPUtilities/SOAPUtilities.h"
//#include "otsdaq-core/SOAPUtilities/SOAPParameters.h"
#include "otsdaq-core/NetworkUtilities/ReceiverSocket.h"

#include <xdaq/NamespaceURI.h>

#include <iostream>
#include <fstream>
#include <string>
#include <dirent.h> 	//for DIR
#include <sys/stat.h> 	//for mkdir
#include <thread>       // std::thread

using namespace ots;


XDAQ_INSTANTIATOR_IMPL(ConsoleSupervisor)


#define MF_POS_OF_MSG	11


#define __MF_SUBJECT__ "Console"
#define __MF_HDR__		__COUT_HDR_FL__
#define __MOUT_ERR__  	mf::LogError	(__MF_SUBJECT__) << __MF_HDR__
#define __MOUT_WARN__  	mf::LogWarning	(__MF_SUBJECT__) << __MF_HDR__
#define __MOUT_INFO__  	mf::LogInfo		(__MF_SUBJECT__) << __COUT_HDR__
#define __MOUT__  		mf::LogDebug	(__MF_SUBJECT__) << __MF_HDR__



//========================================================================================================================
ConsoleSupervisor::ConsoleSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception):
xdaq::Application(s   ),
SOAPMessenger  (this),
theRemoteWebUsers_(this)
{
	INIT_MF("ConsoleSupervisor");
	xgi::bind (this, &ConsoleSupervisor::Default,                	"Default" );
	xgi::bind (this, &ConsoleSupervisor::Request,              		"Request" );
	init();
}

//========================================================================================================================
ConsoleSupervisor::~ConsoleSupervisor(void)
{
	destroy();
}
//========================================================================================================================
void ConsoleSupervisor::init(void)
{
	//called by constructor
	theSupervisorsConfiguration_.init(getApplicationContext());

	//start mf msg listener
	std::thread([](){ ConsoleSupervisor::MFReceiverWorkLoop(); }).detach();
}

//========================================================================================================================
void ConsoleSupervisor::destroy(void)
{
	//called by destructor
}


//========================================================================================================================
//MFReceiverWorkLoop ~~
//	Thread for printing Message Facility messages without decorations
void ConsoleSupervisor::MFReceiverWorkLoop()
{
	std::cout << __COUT_HDR_FL__ << std::endl;
	ReceiverSocket rsock("127.0.0.1",30001);
	rsock.initialize();

	//	int receive(std::string& buffer, unsigned int timeoutSeconds=1, unsigned int timeoutUSeconds=0);
	std::string buffer;
	int i,p;
	while(1)
	{
		//if receive succeeds display message
		if(rsock.receive(buffer,1,0,false) != -1) //set to rcv quiet mode
		{
			//find position of message and save to p
				//by jumping to the correct '|' marker
			for(p=0,i=0;i<MF_POS_OF_MSG;++i)
				p = buffer.find('|',p)+1;
			std::cout << "+" << buffer << std::endl;///&buffer[p] << std::endl;
			//messages[0].set(buffer);
		}
	}

}

//========================================================================================================================
void ConsoleSupervisor::Default(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	__MOUT__ << std::endl;
	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/Console.html?urn=" <<
			getenv("LOGBOOK_SUPERVISOR_ID") << "'></frameset></html>";
}

//========================================================================================================================
//	Request
//		Handles Web Interface requests to Console supervisor.
//		Does not refresh cookie for automatic update checks.
void ConsoleSupervisor::Request(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	cgicc::Cgicc cgi(in);
	std::string Command;
	if((Command = CgiDataUtilities::postData(cgi,"RequestType")) == "")
		Command = cgi("RequestType"); //get command from form, if PreviewEntry

	__MOUT__ << "Command " << Command << " files: " << cgi.getFiles().size() << std::endl;

	//Commands:
		//GetConsoleMsgs

	//**** start LOGIN GATEWAY CODE ***//
	//If TRUE, cookie code is good, and refreshed code is in cookieCode, also pointers optionally for UInt8 userPermissions
	//Else, error message is returned in cookieCode
	uint8_t userPermissions;
	std::string cookieCode = Command == "PreviewEntry"? cgi("CookieCode"):
			CgiDataUtilities::postData(cgi,"CookieCode");
	if(!theRemoteWebUsers_.cookieCodeIsActiveForRequest(theSupervisorsConfiguration_.getSupervisorDescriptor(),
			cookieCode, &userPermissions, "0", Command != "RefreshLogbook")) //only refresh cookie if not automatic refresh
	{
		*out << cookieCode;
		__MOUT_ERR__ << "Invalid Cookie Code" << std::endl;
		return;
	}
	//**** end LOGIN GATEWAY CODE ***//


	HttpXmlDocument xmldoc(cookieCode);
	if(userPermissions < ADMIN_PERMISSIONS_THRESHOLD)
	{
		xmldoc.addTextElementToData("Error","Error - Insufficient permissions.");
		goto CLEANUP;
	}

	//to report to logbook admin status use xmldoc.addTextElementToData(XML_ADMIN_STATUS,tempStr);

	if(Command == "GetConsoleMsgs")
	{

		__MOUT__ << "" << std::endl;
	}



	CLEANUP:

	//return xml doc holding server response
	xmldoc.outputXmlDocument((std::ostringstream*)out);
}









