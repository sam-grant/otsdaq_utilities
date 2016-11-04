#include "OtsConfigurationWizardSupervisor.h"

#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/Macros/CoutHeaderMacros.h"

#include <xdaq/NamespaceURI.h>
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"
#include "otsdaq-core/WebUsersUtilities/WebUsers.h"
#include "otsdaq-core/SOAPUtilities/SOAPUtilities.h"
#include "otsdaq-core/SOAPUtilities/SOAPCommand.h"

#include <iostream>
#include <fstream>
#include <string>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds


using namespace ots;


#define SECURITY_FILE_NAME 		std::string(getenv("SERVICE_DATA_PATH")) + "/OtsWizardData/security.dat"
#define ICON_FILE_NAME 			std::string(getenv("SERVICE_DATA_PATH")) + "/OtsWizardData/iconList.dat";

XDAQ_INSTANTIATOR_IMPL(OtsConfigurationWizardSupervisor)



#undef 	__MF_SUBJECT__
#define __MF_SUBJECT__ "Wizard"


//========================================================================================================================
OtsConfigurationWizardSupervisor::OtsConfigurationWizardSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception):
xdaq::Application(s   ),
SOAPMessenger  (this)
{
	INIT_MF("OtsConfigurationWizard");

	generateURL();
	xgi::bind (this, &OtsConfigurationWizardSupervisor::Default,            	"Default" 			);

	xgi::bind (this, &OtsConfigurationWizardSupervisor::Verification,        	"Verify" 	  		);	//securityCode_);

	xgi::bind (this, &OtsConfigurationWizardSupervisor::RequestIcons,       	"requestIcons"		);
	xgi::bind (this, &OtsConfigurationWizardSupervisor::IconEditor,           	"iconEditor"		);
	xgi::bind (this, &OtsConfigurationWizardSupervisor::EditSecurity,       	"editSecurity"		);

	xoap::bind(this, &OtsConfigurationWizardSupervisor::supervisorSequenceCheck,        "SupervisorSequenceCheck",        XDAQ_NS_URI);

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
	getApplicationContext();
}

//========================================================================================================================
void OtsConfigurationWizardSupervisor::generateURL()
{
	int length = 4;
	securityCode_ = "";

	static const char alphanum[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";

	srand(time(0));

	for (int i = 0; i < length; ++i) {
		securityCode_ += alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	std::cout << __COUT_HDR_FL__ <<
			getenv("OTS_CONFIGURATION_WIZARD_SUPERVISOR_SERVER") << ":" << getenv("PORT") <<
			"/urn:xdaq-application:lid="
			<< this->getApplicationDescriptor()->getLocalId() << "/Verify?code=" << securityCode_ << std::endl;

	std::thread([&](OtsConfigurationWizardSupervisor *ptr, std::string securityCode)
			{printURL(ptr,securityCode);},this,securityCode_).detach();

	return;
}

void OtsConfigurationWizardSupervisor::printURL(OtsConfigurationWizardSupervisor *ptr,
		std::string securityCode)
{
	INIT_MF("ConfigurationWizard");
	// child process
	int i = 0;
	for (; i < 5; ++i)
	{
		std::this_thread::sleep_for (std::chrono::seconds(2));
		std::cout << __COUT_HDR_FL__ <<
				getenv("OTS_CONFIGURATION_WIZARD_SUPERVISOR_SERVER") << ":" << getenv("PORT") <<
				"/urn:xdaq-application:lid="
				<< ptr->getApplicationDescriptor()->getLocalId() << "/Verify?code=" << securityCode << std::endl;
	}
}

//========================================================================================================================
void OtsConfigurationWizardSupervisor::destroy(void)
{
	//called by destructor

}
//========================================================================================================================
//xoap::supervisorSequenceCheck
//	verify cookie
xoap::MessageReference OtsConfigurationWizardSupervisor::supervisorSequenceCheck(xoap::MessageReference message)
throw (xoap::exception::Exception)
{
	//receive request parameters
	SOAPParameters parameters;
	parameters.addParameter("sequence");
	receive(message, parameters);

	std::string submittedSequence = parameters.getValue("sequence");

	//If submittedSequence matches securityCode_ then return full permissions (255)
	//	else, return permissions 0
	uint8_t userPermissions = 0;
	std::string userWithLock = "";

	if(securityCode_ == submittedSequence)
		userPermissions = 255;
	else
		__MOUT__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;

	//fill return parameters
	SOAPParameters retParameters;
	char tmp[5];
	sprintf(tmp, "%d", userPermissions);
	retParameters.addParameter("Permissions", tmp);


	return SOAPUtilities::makeSOAPMessageReference("SequenceResponse",
			retParameters);
}

//========================================================================================================================
void OtsConfigurationWizardSupervisor::Default(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	*out << "<!DOCTYPE HTML><html lang='en'><head><title>ots wiz</title></head>" <<
			"<frameset col='100%' row='100%'><frame src='/WebPath/html/Unauthorized.html?urn=" <<
			this->getApplicationDescriptor()->getLocalId() <<"'></frameset></html>";
}
//========================================================================================================================
void OtsConfigurationWizardSupervisor::Verification(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	cgicc::Cgicc cgi(in);
	std::string submittedSequence = CgiDataUtilities::getData(cgi, "code");
	__MOUT__ << "submittedSequence=" << submittedSequence << std::endl;

	if(securityCode_.compare(submittedSequence) != 0)
	{
		__MOUT__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;
		*out << "Invalid code.";
		return;
	}
	else
	{
		__MOUT__ << "***Successfully authenticated security sequence." << std::endl;
	}

	*out << "<!DOCTYPE HTML><html lang='en'><head><title>ots wiz</title></head>" <<
			"<frameset col='100%' row='100%'><frame src='/WebPath/html/OtsConfigurationWizard.html?urn=" <<
			this->getApplicationDescriptor()->getLocalId() <<"'></frameset></html>";

}
//========================================================================================================================
void OtsConfigurationWizardSupervisor::RequestIcons(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	cgicc::Cgicc cgi(in);
	std::string submittedSequence = CgiDataUtilities::postData(cgi, "sequence");
	if(securityCode_.compare(submittedSequence) != 0)
	{
		__MOUT__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;
		return;
	}
	else
	{
		__MOUT__ << "***Successfully authenticated security sequence." << std::endl;
	}

	//an icon is 6 fields.. give comma-separated
	//0 - alt = text below icon
	//1 - subtext = text for icon if no image
	//2 - uniqueWin = if true, only one window is allowed, else multiple instances of window
	//3 - permissions = security level needed to see icon
	//4 - picfn = icon image filename, 0 for no image
	//5 - linkurl = url of the window to open

	*out << "Edit Security,SEC,1,1,icon-EditSecurity.png,/WebPath/html/EditSecurity.html" <<
			//",Icon Editor,ICON,1,1,icon-IconEditor.png,/WebPath/html/IconEditor.html" <<
			",Configure,CFG,0,1,icon-Configure.png,/urn:xdaq-application:lid=280/" <<
			",Table Editor,TBL,0,1,icon-IconEditor.png,/urn:xdaq-application:lid=280/?tableEditor=1" <<
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
		__MOUT__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;
		return;
	}
	//Security Check complete ================

	__MOUT__ << "***Successfully authenticated security sequence." << std::endl;



	if(submittedIconList != "")
	{
		__MOUT__ << "Icon List exists!" << std::endl;
		__MOUT__ << submittedIconList << std::endl;

		std::ofstream writeIconFile;

		writeIconFile.open(iconFileName.c_str());
		if(writeIconFile.is_open())
			writeIconFile << submittedIconList;
		else
			__MOUT__ << "Error writing file!" << std::endl;


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
		__MOUT__<<"Error opening file: "<< iconFileName << std::endl;
		system("pause");
		return;
	}
	if(iconFile.is_open())
	{
		__MOUT__ << "Opened File: " << iconFileName << std::endl;
		while(std::getline(iconFile, line))
		{
			iconList += line;
			lineNumber++;
		}
		//__MOUT__ << std::to_string(lineNumber) << ":" << iconList << std::endl;

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
		__MOUT__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;
		return;
	}
	else
	{
		__MOUT__ << "***Successfully authenticated security sequence." << std::endl;
	}
	//SECURITY CHECK END ****



	if(submittedSecurity != "")
	{
		__MOUT__ << "Selection exists!" << std::endl;
		__MOUT__ <<  submittedSecurity << std::endl;

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
				__MOUT__ << "Error writing file!" << std::endl;

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
		__MOUT__<<"Error opening file: "<< securityFileName << std::endl;
		system("pause");
		return;
	}
	if(securityFile.is_open())
	{
		__MOUT__ << "Opened File: " << securityFileName << std::endl;
		while(std::getline(securityFile, line))
		{
			security += line;
			lineNumber++;
		}
		//__MOUT__ << std::to_string(lineNumber) << ":" << iconList << std::endl;

		//Close file
		securityFile.close();
	}

	*out << security;
}
