#include "otsdaq-utilities/OtsConfigurationWizard/OtsConfigurationWizard.h"
#include "otsdaq-core/OTSMacros.h"

#include <xdaq/NamespaceURI.h>

#include <iostream>

using namespace ots;

XDAQ_INSTANTIATOR_IMPL(OtsConfigurationWizard)

//========================================================================================================================
OtsConfigurationWizard::OtsConfigurationWizard(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception):
        xdaq::Application(s   ),
        SOAPMessenger  (this)
{

    xgi::bind (this, &OtsConfigurationWizard::Default,                "Default" );
    init();
}

//========================================================================================================================
OtsConfigurationWizard::~OtsConfigurationWizard(void)
{
	destroy();
}
//========================================================================================================================
void OtsConfigurationWizard::init(void)
{
 	//called by constructor
}

//========================================================================================================================
void OtsConfigurationWizard::destroy(void)
{
 	//called by destructor

}

//========================================================================================================================
void OtsConfigurationWizard::Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
    std::cout << __COUT_HDR__ << std::endl;
	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/Chat.html?urn=" << 
		getenv("CHAT_SUPERVISOR_ID") <<"'></frameset></html>";
}

