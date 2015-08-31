#include "OtsConfigurationWizardSupervisor.h"

#include "otsdaq-core/OTSMacros.h"

#include <xdaq/NamespaceURI.h>

#include <iostream>

using namespace ots;

XDAQ_INSTANTIATOR_IMPL(OtsConfigurationWizardSupervisor)

//========================================================================================================================
OtsConfigurationWizardSupervisor::OtsConfigurationWizardSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception):
        xdaq::Application(s   ),
        SOAPMessenger  (this)
{

    xgi::bind (this, &OtsConfigurationWizardSupervisor::Default,                "Default" );
    init();
}

//========================================================================================================================
OtsConfigurationWizardSupervisor::~OtsConfigurationWizardSupervisor(void)
{
	destroy();
}
//========================================================================================================================
void OtsConfigurationWizardSupervisor::init(void)
{
 	//called by constructor
}

//========================================================================================================================
void OtsConfigurationWizardSupervisor::destroy(void)
{
 	//called by destructor

}

//========================================================================================================================
void OtsConfigurationWizardSupervisor::Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
    std::cout << __COUT_HDR__ << std::endl;
	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/Supervisor.html?urn=" <<
		getenv("OTS_CONFIGURATION_WIZARD_SUPERVISOR_ID") <<"'></frameset></html>";
}

