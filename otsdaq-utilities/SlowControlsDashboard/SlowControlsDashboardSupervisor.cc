#include "otsdaq-utilities/SlowControlsDashboard/SlowControlsDashboardSupervisor.h"

#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/Macros/CoutHeaderMacros.h"

#include <xdaq/NamespaceURI.h>
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"
#include "otsdaq-core/WebUsersUtilities/WebUsers.h"

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
        SOAPMessenger  (this)
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

	std::cout << __COUT_HDR_FL__ << getenv("SLOW_CONTROLS_DASHBOARD_SUPERVISOR_ID") << std::endl;
	Default(in, out);
}
//========================================================================================================================
void SlowControlsDashboardSupervisor::init(void)
//called by constructor
{
  if(true)
	  EpicsInterface *interface_ = new EpicsInterface();
}
//========================================================================================================================
void SlowControlsDashboardSupervisor::destroy(void)
{
 	//called by destructor

}

//========================================================================================================================
void SlowControlsDashboardSupervisor::Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{

	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/SlowControlsDashboard.html?urn=" <<
		getenv("SLOW_CONTROLS_DASHBOARD_SUPERVISOR_ID") <<"'></frameset></html>";
}
//========================================================================================================================
void SlowControlsDashboardSupervisor::GetList(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{


}
//========================================================================================================================
void SlowControlsDashboardSupervisor::Subscribe(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{


}
//========================================================================================================================
void SlowControlsDashboardSupervisor::Unsubscribe(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
    

}
