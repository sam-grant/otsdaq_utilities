#include "OtsConfigurationWizardSupervisor.h"

#include "otsdaq-core/OTSMacros.h"

#include <xdaq/NamespaceURI.h>
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"
#include "otsdaq-core/WebUsersUtilities/WebUsers.h"

#include <iostream>
#include <fstream>
#include <string>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds


using namespace ots;


#define SECURITY_FILE_NAME 		std::string(getenv("SERVICE_DATA")) + "/OtsWizardData/security.dat"
#define ICON_FILE_NAME 			std::string(getenv("SERVICE_DATA")) + "/OtsWizardData/iconList.dat";

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


	//////////////////////////////////////////////////////////////////////
	//display otsdaq main url for user convenience (e.g. for 10 seconds)
	pid_t pid = fork();
	if (pid == 0)
	{
		// child process
		int i = 0;
		for (; i < 5; ++i)
		{
			std::this_thread::sleep_for (std::chrono::seconds(2));
			std::cout << __COUT_HDR_P__ << "******************************************************************** " << std::endl;
			std::cout << __COUT_HDR_P__ << "******************************************************************** " << std::endl;
			std::cout << __COUT_HDR_P__ << getenv("OTS_CONFIGURATION_WIZARD_SUPERVISOR_SERVER") << ":" << getenv("PORT") << "/urn:xdaq-application:lid="
		  			  << getenv("OTS_CONFIGURATION_WIZARD_SUPERVISOR_ID") << "/" << securityCode_ << std::endl;
			std::cout << __COUT_HDR_P__ << "******************************************************************** " << std::endl;
			std::cout << __COUT_HDR_P__ << "******************************************************************** " << std::endl;
		}
		exit(0); //done
	}
	else if (pid < 0)
	{
		// fork failed
		std::cout << __COUT_HDR__ << "fork() failed!" << std::endl;
		exit(0);
	}
	//parent process continues
	//////////////////////////////////////////////////////////////////////

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
    }
    else
    {
    	std::cout << __COUT_HDR__ << "***Successfully authenticated security sequence." << std::endl;
    }

    //an icon is 6 fields.. give comma-separated
    	//0 - alt = text for mouse over
    	//1 - subtext = text below icon
    	//2 - uniqueWin = if true, only one window is allowed, else multiple instances of window
    	//3 - permissions = security level needed to see icon
    	//4 - picfn = icon image filename
    	//5 - linkurl = url of the window to open

    *out << "Icon Editor,ICON,1,1,icon-IconEditor.png,/WebPath/html/iconEditor.html" <<
    		",Edit Security,SEC,1,1,icon-EditSecurity.png,/WebPath/html/editSecurity.html";
    return;
}
//========================================================================================================================
void OtsConfigurationWizardSupervisor::IconEditor(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
    std::cout << __COUT_HDR__ << std::endl;


    //if sequence doesn't match up -> return
    cgicc::Cgicc cgi(in);
    std::string submittedSequence = CgiDataUtilities::postData(cgi, "sequence");
    std::string submittedIconList = CgiDataUtilities::postData(cgi, "iconList");
    std::string iconFileName = ICON_FILE_NAME;



    //Security Check ================
    if(securityCode_ != submittedSequence)
    {
    	std::cout << __COUT_HDR__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;
    	return;
    }
    //Security Check complete ================

    std::cout << __COUT_HDR__ << "***Successfully authenticated security sequence." << std::endl;



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
	std::string securityFileName = SECURITY_FILE_NAME;


	//SECURITY CHECK START ****
	if(securityCode_.compare(submittedSequence) != 0)
	{
		std::cout << __COUT_HDR__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;
		return;
	}
	else
	{
		std::cout << __COUT_HDR__ << "***Successfully authenticated security sequence." << std::endl;
	}
	//SECURITY CHECK END ****



	if(submittedSecurity != "")
	{
		std::cout << __COUT_HDR__ << "Selection exists!" << std::endl;
		std::cout << __COUT_HDR__ <<  submittedSecurity << std::endl;

		if (strcmp(submittedSecurity.c_str(), "ResetSecurityUserData") == 0)
		{
			WebUsers::deleteUserData();
		}
		else
		{
			std::ofstream writeSecurityFile;

			writeSecurityFile.open(securityFileName.c_str());
			if(writeSecurityFile.is_open())
				writeSecurityFile << submittedSecurity;
			else
				std::cout << __COUT_HDR__ << "Error writing file!" << std::endl;

			writeSecurityFile.close();
		}
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
		std::cout << __COUT_HDR__ << "Opened File: " << securityFileName << std::endl;
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
}
