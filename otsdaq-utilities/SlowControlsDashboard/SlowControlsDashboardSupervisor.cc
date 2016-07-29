#include "otsdaq-utilities/SlowControlsDashboard/SlowControlsDashboardSupervisor.h"

#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/Macros/CoutHeaderMacros.h"

#include <xdaq/NamespaceURI.h>
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"
#include "otsdaq-core/WebUsersUtilities/WebUsers.h"
#include "otsdaq-core/SOAPUtilities/SOAPParameters.h"

#include <iostream>
#include <fstream>
#include <string>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

#include "otsdaq-utilities/SlowControlsDashboard/EpicsInterface.h"
#include "otsdaq-utilities/SlowControlsDashboard/SlowControlsInterface.h"

using namespace ots;




XDAQ_INSTANTIATOR_IMPL(SlowControlsDashboardSupervisor)

//========================================================================================================================
SlowControlsDashboardSupervisor::SlowControlsDashboardSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception):
        xdaq::Application(s   ),
        SOAPMessenger  (this),
        theRemoteWebUsers_(this)
{
     INIT_MF("SlowControlsDashboardSupervisor");
 	std::cout << __COUT_HDR_FL__ << getenv("SLOW_CONTROLS_DASHBOARD_SUPERVISOR_ID") << std::endl;

    xgi::bind (this, &SlowControlsDashboardSupervisor::requestHandler, "Default");
    init();

}

//========================================================================================================================
SlowControlsDashboardSupervisor::~SlowControlsDashboardSupervisor(void)
{
	destroy();
}
//========================================================================================================================
void SlowControlsDashboardSupervisor::requestHandler(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
    cgicc::Cgicc cgi(in);
    std::string Command = CgiDataUtilities::getData(cgi,"RequestType");
    std::cout << __COUT_HDR_FL__ << Command << std::endl;
	std::cout << __COUT_HDR_FL__ << getenv("SLOW_CONTROLS_DASHBOARD_SUPERVISOR_ID") << " " << Command << " : end"<< std::endl;
	if(Command == "")
	{
		Default(in, out);		
	}
	else if(Command == "getList")
	{
		GetList(in, out);
	}
	std::cout << "********************************************************" << std::endl;
	//**** start LOGIN GATEWAY CODE ***//
	//If TRUE, cookie code is good, and refreshed code is in cookieCode, also pointers optionally for UInt8 userPermissions
	//Else, error message is returned in cookieCode
	/*uint8_t userPermissions;
	//std::string cookieCode = Command == "PreviewEntry"? cgi("CookieCode"):	CgiDataUtilities::postData(cgi,"CookieCode");
    std::string cookieCode = CgiDataUtilities::postData(cgi,"CookieCode");
	if(!theRemoteWebUsers_.cookieCodeIsActiveForRequest(theSupervisorsConfiguration_.getSupervisorDescriptor(),
			cookieCode, &userPermissions, "", true)) //only refresh cookie if not automatic refresh
	{
		*out << cookieCode;
		std::cout << __COUT_HDR_FL__ << "Invalid Cookie Code: " << cookieCode << std::endl;
		return;
	}

	theRemoteWebUsers_.getUserInfoForCookie(theSupervisorsConfiguration_.getSupervisorDescriptor(),cookieCode, &username, 0,0);
	SOAPParameters retParameters;
	retParameters.addParameter("Username", username);*/

	//**** end LOGIN GATEWAY CODE ***//

}
//========================================================================================================================
void SlowControlsDashboardSupervisor::init(void)
//called by constructor
{
  //if(true)
  interface_ = new EpicsInterface();
  interface_->initialize();
}
//========================================================================================================================
void SlowControlsDashboardSupervisor::destroy(void)
{
 	//called by destructor

}

//========================================================================================================================
void SlowControlsDashboardSupervisor::Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
	std::cout << __COUT_HDR_FL__ << getenv("SLOW_CONTROLS_DASHBOARD_SUPERVISOR_ID") << std::endl;

	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/SlowControlsDashboard.html?urn=" <<
		getenv("SLOW_CONTROLS_DASHBOARD_SUPERVISOR_ID") <<"' /></frameset></html>";
}
//========================================================================================================================
void SlowControlsDashboardSupervisor::GetList(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{

	std::cout << __COUT_HDR_FL__ << getenv("SLOW_CONTROLS_DASHBOARD_SUPERVISOR_ID") << " "	<< interface_->getList("JSON") << std::endl;

	*out << interface_->getList("JSON");
}
//========================================================================================================================
void SlowControlsDashboardSupervisor::Subscribe(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{


}
//========================================================================================================================
void SlowControlsDashboardSupervisor::Unsubscribe(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
    

}
