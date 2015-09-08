#include "OtsConfigurationWizardSupervisor.h"

#include "otsdaq-core/OTSMacros.h"

#include <xdaq/NamespaceURI.h>
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"

#include <iostream>
#include <fstream>
#include <string>
using namespace ots;

XDAQ_INSTANTIATOR_IMPL(OtsConfigurationWizardSupervisor)

//========================================================================================================================
OtsConfigurationWizardSupervisor::OtsConfigurationWizardSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception):
        xdaq::Application(s   ),
        SOAPMessenger  (this)
{

	generateURL();

    xgi::bind (this, &OtsConfigurationWizardSupervisor::Default,                "Default" );
    xgi::bind (this, &OtsConfigurationWizardSupervisor::Verification,        securityCode_);
    xgi::bind (this, &OtsConfigurationWizardSupervisor::RequestIcons,       "requestIcons");
    xgi::bind (this, &OtsConfigurationWizardSupervisor::IconEditor,           "iconEditor");
    xgi::bind (this, &OtsConfigurationWizardSupervisor::EditSecurity,       "editSecurity");
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
void OtsConfigurationWizardSupervisor::generateURL()
{
	int length = 32;
	securityCode_ = "";

	static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

	srand(time(0));

    for (int i = 0; i < length; ++i) {
    	securityCode_ += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

	std::cout << "****************************************************************"
			  << "\n" << getenv("OTS_CONFIGURATION_WIZARD_SUPERVISOR_SERVER") << ":" << getenv("PORT") << "/urn:xdaq-application:lid="
  			  << getenv("OTS_CONFIGURATION_WIZARD_SUPERVISOR_ID") << "/" << securityCode_
			  << "\n****************************************************************" << std::endl;

    return;
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
	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/Unauthorized.html?urn=" <<
		getenv("OTS_CONFIGURATION_WIZARD_SUPERVISOR_ID") <<"'></frameset></html>";
}
//========================================================================================================================
void OtsConfigurationWizardSupervisor::Verification(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
    std::cout << __COUT_HDR__ << std::endl;
	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/OtsConfigurationWizard.html?urn=" <<
		getenv("OTS_CONFIGURATION_WIZARD_SUPERVISOR_ID") <<"'></frameset></html>";

}
//========================================================================================================================
void OtsConfigurationWizardSupervisor::RequestIcons(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
    std::cout << __COUT_HDR__ << std::endl;

    cgicc::Cgicc cgi(in);
    std::string submittedSequence = CgiDataUtilities::postData(cgi, "sequence");
    if(securityCode_.compare(submittedSequence) != 0)
    {
    	std::cout << __COUT_HDR__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;
    	return;
    }else
    {
    	std::cout << __COUT_HDR__ << "***Successfully authenticated security sequence." << std::endl;
    }


    *out << "Icon Editor,ICON,1,1,icon-IconEditor.png,/WebPath/html/iconEditor.html,Edit Security,SEC,1,1,icon-EditSecurity.png,/WebPath/html/editSecurity.html";
    return;
}
//========================================================================================================================
void OtsConfigurationWizardSupervisor::IconEditor(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
    std::cout << __COUT_HDR__ << std::endl;
    //TODO Security Check;
    //if sequence doesn't match up -> return
    cgicc::Cgicc cgi(in);
    std::string submittedSequence = CgiDataUtilities::postData(cgi, "sequence");
    std::string submittedIconList = CgiDataUtilities::postData(cgi, "iconList");
    std::string iconFileName = std::string(getenv("OTSDAQDEMO_REPO")) + "/Data/ServiceData/OtsWizardData/iconList.txt";




    if(securityCode_.compare(submittedSequence) != 0)
    {
    	std::cout << __COUT_HDR__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;
    	return;
    }else
    {
    	std::cout << __COUT_HDR__ << "***Successfully authenticated security sequence." << std::endl;
    }


    if(submittedIconList != "")
    {
    	std::cout << "Icon List exists!" << std::endl;
    	std::cout << submittedIconList << std::endl;

    	std::ofstream writeIconFile;

    	writeIconFile.open(iconFileName.c_str());
        if(writeIconFile.is_open())
        	writeIconFile << submittedIconList;
        else
        	std::cout << __COUT_HDR__ << "Error writing file!" << std::endl;


        writeIconFile.close();
    }


    //Always return the file
    std::ifstream iconFile;
    std::string line;
	std::string iconList = "";
	int lineNumber = 0;

	iconFile.open(iconFileName.c_str());

    if(!iconFile)
    {
		std::cout<<"Error opening file: "<< iconFileName << std::endl;
		system("pause");
		return;
    }
    if(iconFile.is_open())
    {
    	std::cout << "Opened File: " << iconFileName << std::endl;
        while(std::getline(iconFile, line))
    	{
    		iconList += line;
    		lineNumber++;
    	}
    	//std::cout << __COUT_HDR__ << std::to_string(lineNumber) << ":" << iconList << std::endl;

    	//Close file
        iconFile.close();
    }

    *out << iconList;

    return;
}

//========================================================================================================================
void OtsConfigurationWizardSupervisor::EditSecurity(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
	  std::cout << __COUT_HDR__ << std::endl;
	    //if sequence doesn't match up -> return
	    cgicc::Cgicc cgi(in);
	    std::string submittedSequence = CgiDataUtilities::postData(cgi, "sequence");
	    std::string submittedSecurity = CgiDataUtilities::postData(cgi, "selection");
	    std::string securityFileName = std::string(getenv("OTSDAQDEMO_REPO")) + "/Data/ServiceData/OtsWizardData/security.dat";




	    if(securityCode_.compare(submittedSequence) != 0)
	    {
	    	std::cout << __COUT_HDR__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;
	    	return;
	    }else
	    {
	    	std::cout << __COUT_HDR__ << "***Successfully authenticated security sequence." << std::endl;
	    }


	    if(submittedSecurity != "")
	    {
	    	std::cout << "Selection exists!" << std::endl;
	    	std::cout << submittedSecurity << std::endl;

	    	std::ofstream writeSecurityFile;

	    	writeSecurityFile.open(securityFileName.c_str());
	        if(writeSecurityFile.is_open())
	        	writeSecurityFile << submittedSecurity;
	        else
	        	std::cout << __COUT_HDR__ << "Error writing file!" << std::endl;


	        writeSecurityFile.close();
	    }


	    //Always return the file
	    std::ifstream securityFile;
	    std::string line;
		std::string security = "";
		int lineNumber = 0;

		securityFile.open(securityFileName.c_str());

	    if(!securityFile)
	    {
			std::cout<<"Error opening file: "<< securityFileName << std::endl;
			system("pause");
			return;
	    }
	    if(securityFile.is_open())
	    {
	    	std::cout << "Opened File: " << securityFileName << std::endl;
	        while(std::getline(securityFile, line))
	    	{
	        	security += line;
	    		lineNumber++;
	    	}
	    	//std::cout << __COUT_HDR__ << std::to_string(lineNumber) << ":" << iconList << std::endl;

	    	//Close file
	        securityFile.close();
	    }

	    *out << security;

	    return;



}
