#include "OtsConfigurationWizardSupervisor.h"

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


using namespace ots;


#define SECURITY_FILE_NAME 		std::string(getenv("SERVICE_DATA")) + "/OtsWizardData/security.dat"
#define ICON_FILE_NAME 			std::string(getenv("SERVICE_DATA")) + "/OtsWizardData/iconList.dat";

XDAQ_INSTANTIATOR_IMPL(OtsConfigurationWizardSupervisor)



#define __MF_SUBJECT__ "Wizard"
#define __MF_HDR__		__COUT_HDR_FL__
#define __MOUT_ERR__  	mf::LogError	(__MF_SUBJECT__) << __MF_HDR__
#define __MOUT_WARN__  	mf::LogWarning	(__MF_SUBJECT__) << __MF_HDR__
#define __MOUT_INFO__  	mf::LogInfo		(__MF_SUBJECT__) << __MF_HDR__
#define __MOUT__  		mf::LogDebug	(__MF_SUBJECT__) << __MF_HDR__
#define __SS__			std::stringstream ss; ss << __COUT_HDR_FL__


//========================================================================================================================
OtsConfigurationWizardSupervisor::OtsConfigurationWizardSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception):
xdaq::Application(s   ),
SOAPMessenger  (this)
{
	INIT_MF("OtsConfigurationWizard");
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

	srand(0);//time(0)); FIXME for testing!

	for (int i = 0; i < length; ++i) {
		securityCode_ += alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	std::thread([&](){printURL();}).detach();

	return;
}

void OtsConfigurationWizardSupervisor::printURL()
{
	INIT_MF("ConfigurationWizard");
	// child process
	int i = 0;
	for (; i < 5; ++i)
	{
		std::this_thread::sleep_for (std::chrono::seconds(2));
		mf::LogError(__FILE__) << __COUT_HDR_P__ << getenv("OTS_CONFIGURATION_WIZARD_SUPERVISOR_SERVER") << ":" << getenv("PORT") << "/urn:xdaq-application:lid="
				<< getenv("OTS_CONFIGURATION_WIZARD_SUPERVISOR_ID") << "/" << securityCode_ << std::endl;
	}
}

//========================================================================================================================
void OtsConfigurationWizardSupervisor::destroy(void)
{
	//called by destructor

}

//========================================================================================================================
void OtsConfigurationWizardSupervisor::Default(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{


	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/Unauthorized.html?urn=" <<
			getenv("OTS_CONFIGURATION_WIZARD_SUPERVISOR_ID") <<"'></frameset></html>";
}
//========================================================================================================================
void OtsConfigurationWizardSupervisor::Verification(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{

	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/OtsConfigurationWizard.html?urn=" <<
			getenv("OTS_CONFIGURATION_WIZARD_SUPERVISOR_ID") <<"'></frameset></html>";

}
//========================================================================================================================
void OtsConfigurationWizardSupervisor::RequestIcons(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{


	cgicc::Cgicc cgi(in);
	std::string submittedSequence = CgiDataUtilities::postData(cgi, "sequence");
	if(securityCode_.compare(submittedSequence) != 0)
	{
		std::cout << __COUT_HDR_FL__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;
		return;
	}
	else
	{
		std::cout << __COUT_HDR_FL__ << "***Successfully authenticated security sequence." << std::endl;
	}

	//an icon is 6 fields.. give comma-separated
	//0 - alt = text below icon
	//1 - subtext = text for icon if no image
	//2 - uniqueWin = if true, only one window is allowed, else multiple instances of window
	//3 - permissions = security level needed to see icon
	//4 - picfn = icon image filename, 0 for no image
	//5 - linkurl = url of the window to open

	*out << "Icon Editor,ICON,1,1,icon-IconEditor.png,/WebPath/html/iconEditor.html" <<
			",Edit Security,SEC,1,1,icon-EditSecurity.png,/WebPath/html/editSecurity.html" <<
			",Configure,CFG,1,1,icon-Configure.png,/urn:xdaq-application:lid=280/" <<
			",Console,C,1,1,icon-Console.png,/urn:xdaq-application:lid=261/" <<
			",DB Utilities,DB,1,1,0,http://127.0.0.1:8080/db/client.html";
	return;
}
//========================================================================================================================
void OtsConfigurationWizardSupervisor::IconEditor(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{

	//if sequence doesn't match up -> return
	cgicc::Cgicc cgi(in);
	std::string submittedSequence = CgiDataUtilities::postData(cgi, "sequence");
	std::string submittedIconList = CgiDataUtilities::postData(cgi, "iconList");
	std::string iconFileName = ICON_FILE_NAME;



	//Security Check ================
	if(securityCode_ != submittedSequence)
	{
		std::cout << __COUT_HDR_FL__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;
		return;
	}
	//Security Check complete ================

	std::cout << __COUT_HDR_FL__ << "***Successfully authenticated security sequence." << std::endl;



	if(submittedIconList != "")
	{
		std::cout << __COUT_HDR_FL__ << "Icon List exists!" << std::endl;
		std::cout << __COUT_HDR_FL__ << submittedIconList << std::endl;

		std::ofstream writeIconFile;

		writeIconFile.open(iconFileName.c_str());
		if(writeIconFile.is_open())
			writeIconFile << submittedIconList;
		else
			std::cout << __COUT_HDR_FL__ << "Error writing file!" << std::endl;


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
		std::cout << __COUT_HDR_FL__<<"Error opening file: "<< iconFileName << std::endl;
		system("pause");
		return;
	}
	if(iconFile.is_open())
	{
		std::cout << __COUT_HDR_FL__ << "Opened File: " << iconFileName << std::endl;
		while(std::getline(iconFile, line))
		{
			iconList += line;
			lineNumber++;
		}
		//std::cout << __COUT_HDR_FL__ << std::to_string(lineNumber) << ":" << iconList << std::endl;

		//Close file
		iconFile.close();
	}

	*out << iconList;

	return;
}

//========================================================================================================================
void OtsConfigurationWizardSupervisor::EditSecurity(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{

	//if sequence doesn't match up -> return
	cgicc::Cgicc cgi(in);
	std::string submittedSequence = CgiDataUtilities::postData(cgi, "sequence");
	std::string submittedSecurity = CgiDataUtilities::postData(cgi, "selection");
	std::string securityFileName = SECURITY_FILE_NAME;


	//SECURITY CHECK START ****
	if(securityCode_.compare(submittedSequence) != 0)
	{
		std::cout << __COUT_HDR_FL__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;
		return;
	}
	else
	{
		std::cout << __COUT_HDR_FL__ << "***Successfully authenticated security sequence." << std::endl;
	}
	//SECURITY CHECK END ****



	if(submittedSecurity != "")
	{
		std::cout << __COUT_HDR_FL__ << "Selection exists!" << std::endl;
		std::cout << __COUT_HDR_FL__ <<  submittedSecurity << std::endl;

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
				std::cout << __COUT_HDR_FL__ << "Error writing file!" << std::endl;

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
		std::cout << __COUT_HDR_FL__<<"Error opening file: "<< securityFileName << std::endl;
		system("pause");
		return;
	}
	if(securityFile.is_open())
	{
		std::cout << __COUT_HDR_FL__ << "Opened File: " << securityFileName << std::endl;
		while(std::getline(securityFile, line))
		{
			security += line;
			lineNumber++;
		}
		//std::cout << __COUT_HDR_FL__ << std::to_string(lineNumber) << ":" << iconList << std::endl;

		//Close file
		securityFile.close();
	}

	*out << security;
}
