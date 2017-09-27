#include "OtsConfigurationWizardSupervisor.h"

#include "otsdaq-core/Supervisor/Supervisor.h"

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
#include <sys/stat.h> 	  // mkdir


using namespace ots;


#define SECURITY_FILE_NAME 		std::string(getenv("SERVICE_DATA_PATH")) + "/OtsWizardData/security.dat"
#define SEQUENCE_FILE_NAME 		std::string(getenv("SERVICE_DATA_PATH")) + "/OtsWizardData/sequence.dat"
#define SEQUENCE_OUT_FILE_NAME 	std::string(getenv("SERVICE_DATA_PATH")) + "/OtsWizardData/sequence.out"

XDAQ_INSTANTIATOR_IMPL(OtsConfigurationWizardSupervisor)



#undef 	__MF_SUBJECT__
#define __MF_SUBJECT__ "Wizard"


//========================================================================================================================
OtsConfigurationWizardSupervisor::OtsConfigurationWizardSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception):
xdaq::Application(s   ),
SOAPMessenger  (this)
{
	INIT_MF("OtsConfigurationWizard");


	//attempt to make directory structure (just in case)
	mkdir((std::string(getenv("SERVICE_DATA_PATH"))).c_str(), 0755);
	mkdir((std::string(getenv("SERVICE_DATA_PATH")) + "/OtsWizardData").c_str(), 0755);

	generateURL();
	xgi::bind (this, &OtsConfigurationWizardSupervisor::Default,            	"Default" 			);

	xgi::bind (this, &OtsConfigurationWizardSupervisor::verification,        	"Verify" 	  		);

	xgi::bind (this, &OtsConfigurationWizardSupervisor::requestIcons,       	"requestIcons"		);
	xgi::bind (this, &OtsConfigurationWizardSupervisor::editSecurity,       	"editSecurity"		);
	xgi::bind (this, &OtsConfigurationWizardSupervisor::tooltipRequest,         "TooltipRequest"	);

	xoap::bind(this, &OtsConfigurationWizardSupervisor::supervisorSequenceCheck,			"SupervisorSequenceCheck",        	XDAQ_NS_URI);
	xoap::bind(this, &OtsConfigurationWizardSupervisor::supervisorLastConfigGroupRequest,	"SupervisorLastConfigGroupRequest", XDAQ_NS_URI);
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
	FILE *fp = fopen((SEQUENCE_FILE_NAME).c_str(),"r");
	if(fp)
	{
		__COUT_INFO__ <<  "Sequence length file found: " << SEQUENCE_FILE_NAME << std::endl;
		char line[100];
		fgets(line,100,fp);
		sscanf(line,"%d",&length);
		fclose(fp);
		if(length < 4) length = 4; //don't allow shorter than 4
		srand(time(0)); //randomize differently each "time"
	}
	else
	{
		__COUT_INFO__ <<  "(Reverting to default wiz security) Sequence length file NOT found: " << SEQUENCE_FILE_NAME << std::endl;
		srand(0);	//use same seed for convenience if file not found
	}

	__COUT__ << "Sequence length = " << length << std::endl;

	securityCode_ = "";

	static const char alphanum[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";


	for (int i = 0; i < length; ++i) {
		securityCode_ += alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	std::cout << __COUT_HDR_FL__ <<
			getenv("OTS_CONFIGURATION_WIZARD_SUPERVISOR_SERVER") << ":" << getenv("PORT") <<
			"/urn:xdaq-application:lid="
			<< this->getApplicationDescriptor()->getLocalId() << "/Verify?code=" << securityCode_ << std::endl;

	//Note: print out handled by StartOTS.sh now
	//std::thread([&](OtsConfigurationWizardSupervisor *ptr, std::string securityCode)
	//		{printURL(ptr,securityCode);},this,securityCode_).detach();

	fp = fopen((SEQUENCE_OUT_FILE_NAME).c_str(),"w");
	if(fp)
	{
		fprintf(fp,"%s",securityCode_.c_str());
		fclose(fp);
	}
	else
		__COUT_ERR__ <<  "Sequence output file NOT found: " << SEQUENCE_OUT_FILE_NAME << std::endl;


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
void OtsConfigurationWizardSupervisor::tooltipRequest(xgi::Input * in, xgi::Output * out)
throw (xgi::exception::Exception)
{
	cgicc::Cgicc cgi(in);

	std::string Command = CgiDataUtilities::getData(cgi, "RequestType");
	__COUT__ << "Command = " << Command <<  std::endl;

	std::string submittedSequence = CgiDataUtilities::postData(cgi, "sequence");

	//SECURITY CHECK START ****
	if(securityCode_.compare(submittedSequence) != 0)
	{
		__COUT__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;
		return;
	}
	else
	{
		__COUT__ << "***Successfully authenticated security sequence." << std::endl;
	}
	//SECURITY CHECK END ****

	HttpXmlDocument xmldoc;

	if(Command == "check")
	{
		WebUsers::tooltipCheckForUsername(
				WebUsers::DEFAULT_ADMIN_USERNAME,
				&xmldoc,
				CgiDataUtilities::getData(cgi, "srcFile"),
				CgiDataUtilities::getData(cgi, "srcFunc"),
				CgiDataUtilities::getData(cgi, "srcId"));
	}
	else if(Command == "setNeverShow")
	{
		WebUsers::tooltipSetNeverShowForUsername(
				WebUsers::DEFAULT_ADMIN_USERNAME,
				&xmldoc,
				CgiDataUtilities::getData(cgi, "srcFile"),
				CgiDataUtilities::getData(cgi, "srcFunc"),
				CgiDataUtilities::getData(cgi, "srcId"),
				CgiDataUtilities::getData(cgi, "doNeverShow") == "1"?true:false,
				CgiDataUtilities::getData(cgi, "temporarySilence") == "1"?true:false);

	}
	else
		__COUT__ << "Command Request, " << Command << ", not recognized." << std::endl;

	xmldoc.outputXmlDocument((std::ostringstream*) out, false, true);
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
		__COUT__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;

	//fill return parameters
	SOAPParameters retParameters;
	char tmp[5];
	sprintf(tmp, "%d", userPermissions);
	retParameters.addParameter("Permissions", tmp);


	return SOAPUtilities::makeSOAPMessageReference("SequenceResponse",
			retParameters);
}

//===================================================================================================================
//xoap::supervisorLastConfigGroupRequest
//	return the group name and key for the last state machine activity
//
//	Note: same as Supervisor::supervisorLastConfigGroupRequest
xoap::MessageReference OtsConfigurationWizardSupervisor::supervisorLastConfigGroupRequest(
		xoap::MessageReference message)
throw (xoap::exception::Exception)
{
	SOAPParameters parameters;
	parameters.addParameter("ActionOfLastGroup");
	receive(message, parameters);

	return Supervisor::lastConfigGroupRequestHandler(parameters);
}

//========================================================================================================================
void OtsConfigurationWizardSupervisor::Default(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	__COUT__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;
	*out << "Unauthorized Request.";
}

//========================================================================================================================
void OtsConfigurationWizardSupervisor::verification(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	cgicc::Cgicc cgi(in);
	std::string submittedSequence = CgiDataUtilities::getData(cgi, "code");
	__COUT__ << "submittedSequence=" << submittedSequence <<
			" " << time(0) << std::endl;

	if(securityCode_.compare(submittedSequence) != 0)
	{
		__COUT__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;
		*out << "Invalid code.";
		return;
	}
	else
	{
		__COUT__ << "***Successfully authenticated security sequence. " <<
				time(0) << std::endl;
	}

	*out << "<!DOCTYPE HTML><html lang='en'><head><title>ots wiz</title>" <<
			//show ots icon
			//	from http://www.favicon-generator.org/
			"<link rel='apple-touch-icon' sizes='57x57' href='/WebPath/images/otsdaqIcons/apple-icon-57x57.png'>\
		<link rel='apple-touch-icon' sizes='60x60' href='/WebPath/images/otsdaqIcons/apple-icon-60x60.png'>\
		<link rel='apple-touch-icon' sizes='72x72' href='/WebPath/images/otsdaqIcons/apple-icon-72x72.png'>\
		<link rel='apple-touch-icon' sizes='76x76' href='/WebPath/images/otsdaqIcons/apple-icon-76x76.png'>\
		<link rel='apple-touch-icon' sizes='114x114' href='/WebPath/images/otsdaqIcons/apple-icon-114x114.png'>\
		<link rel='apple-touch-icon' sizes='120x120' href='/WebPath/images/otsdaqIcons/apple-icon-120x120.png'>\
		<link rel='apple-touch-icon' sizes='144x144' href='/WebPath/images/otsdaqIcons/apple-icon-144x144.png'>\
		<link rel='apple-touch-icon' sizes='152x152' href='/WebPath/images/otsdaqIcons/apple-icon-152x152.png'>\
		<link rel='apple-touch-icon' sizes='180x180' href='/WebPath/images/otsdaqIcons/apple-icon-180x180.png'>\
		<link rel='icon' type='image/png' sizes='192x192'  href='/WebPath/images/otsdaqIcons/android-icon-192x192.png'>\
		<link rel='icon' type='image/png' sizes='32x32' href='/WebPath/images/otsdaqIcons/favicon-32x32.png'>\
		<link rel='icon' type='image/png' sizes='96x96' href='/WebPath/images/otsdaqIcons/favicon-96x96.png'>\
		<link rel='icon' type='image/png' sizes='16x16' href='/WebPath/images/otsdaqIcons/favicon-16x16.png'>\
		<link rel='manifest' href='/WebPath/images/otsdaqIcons/manifest.json'>\
		<meta name='msapplication-TileColor' content='#ffffff'>\
		<meta name='msapplication-TileImage' content='/ms-icon-144x144.png'>\
		<meta name='theme-color' content='#ffffff'>" <<
					//end show ots icon
			"</head>" <<
			"<frameset col='100%' row='100%'><frame src='/WebPath/html/OtsConfigurationWizard.html?urn=" <<
			this->getApplicationDescriptor()->getLocalId() <<"'></frameset></html>";

}

//========================================================================================================================
void OtsConfigurationWizardSupervisor::requestIcons(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	cgicc::Cgicc cgi(in);

	std::string submittedSequence = CgiDataUtilities::postData(cgi, "sequence");

	//SECURITY CHECK START ****
	if(securityCode_.compare(submittedSequence) != 0)
	{
		__COUT__ << "Unauthorized Request made, security sequence doesn't match! " <<
				time(0) << std::endl;
		return;
	}
	else
	{
		__COUT__ << "***Successfully authenticated security sequence. " <<
				time(0) << std::endl;
	}
	//SECURITY CHECK END ****


	//an icon is 7 fields.. give comma-separated
	//0 - subtext = text below icon
	//1 - altText = text for icon if image set to 0
	//2 - uniqueWin = if true, only one window is allowed, else multiple instances of window
	//3 - permissions = security level needed to see icon
	//4 - picfn = icon image filename, 0 for no image
	//5 - linkurl = url of the window to open
	//6 - folderPath = folder and subfolder location

	*out << "Edit Security,SEC,1,1,icon-EditSecurity.png,/WebPath/html/EditSecurity.html,/" <<
			",Edit User Data,USER,1,1,icon-Chat.png,/WebPath/html/EditUserData.html,/" <<
			",Configure,CFG,0,1,icon-Configure.png,/urn:xdaq-application:lid=280/,/" <<
			",Table Editor,TBL,0,1,icon-IconEditor.png,/urn:xdaq-application:lid=280/?configWindowName=tableEditor,/" <<
			",Configure,CFG,0,1,icon-Configure.png,/urn:xdaq-application:lid=280/,/" <<
			",Iterate,IT,0,1,icon-Iterate.png,/urn:xdaq-application:lid=280/?configWindowName=iterate,/" <<
			//",Configure,CFG,0,1,icon-Configure.png,/urn:xdaq-application:lid=280/,myFolder" <<
			//",Configure,CFG,0,1,icon-Configure.png,/urn:xdaq-application:lid=280/,/myFolder/mySub.folder" <<
			//",Configure,CFG,0,1,icon-Configure.png,/urn:xdaq-application:lid=280/,myFolder/" <<
			//",Console,C,1,1,icon-Console.png,/urn:xdaq-application:lid=261/" <<
			//",DB Utilities,DB,1,1,0,http://127.0.0.1:8080/db/client.html" <<
			"";
	return;
}

//========================================================================================================================
void OtsConfigurationWizardSupervisor::editSecurity(xgi::Input * in, xgi::Output * out )
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
		__COUT__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;
		return;
	}
	else
	{
		__COUT__ << "***Successfully authenticated security sequence." << std::endl;
	}
	//SECURITY CHECK END ****



	if(submittedSecurity != "")
	{
		__COUT__ << "Selection exists!" << std::endl;
		__COUT__ <<  submittedSecurity << std::endl;

		if(submittedSecurity == "ResetSecurityUserData")
		{
			WebUsers::deleteUserData();
		}
		else if(submittedSecurity == "ResetAllUserTooltips")
		{
			WebUsers::resetAllUserTooltips();
			*out << submittedSecurity;
			return;
		}
		else if(submittedSecurity == "DigestAccessAuthentication" ||
				submittedSecurity == "NoSecurity")
		{
			std::ofstream writeSecurityFile;

			writeSecurityFile.open(securityFileName.c_str());
			if(writeSecurityFile.is_open())
				writeSecurityFile << submittedSecurity;
			else
				__COUT__ << "Error writing file!" << std::endl;

			writeSecurityFile.close();
		}
		else
		{
			__COUT_ERR__ << "Invalid submittedSecurity string: " <<
					submittedSecurity << std::endl;
			*out << "Error";
			return;
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
		__SS__ << "Error opening file: "<< securityFileName << std::endl;
		__COUT_ERR__ << "\n" << ss.str();
		//throw std::runtime_error(ss.str());
		//return;
		security = "DigestAccessAuthentication"; //default security when no file exists
	}
	if(securityFile.is_open())
	{
		//__COUT__ << "Opened File: " << securityFileName << std::endl;
		while(std::getline(securityFile, line))
		{
			security += line;
			lineNumber++;
		}
		//__COUT__ << std::to_string(lineNumber) << ":" << iconList << std::endl;

		//Close file
		securityFile.close();
	}

	*out << security;
}
