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
#include <sys/stat.h> 	  // mkdir


using namespace ots;


#define SECURITY_FILE_NAME 		std::string(getenv("SERVICE_DATA_PATH")) + "/OtsWizardData/security.dat"
#define SEQUENCE_FILE_NAME 		std::string(getenv("SERVICE_DATA_PATH")) + "/OtsWizardData/sequence.dat"

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
	FILE *fp = fopen((SEQUENCE_FILE_NAME).c_str(),"r");
	if(fp)
	{
		__MOUT_INFO__ <<  "Sequence length file found: " << SEQUENCE_FILE_NAME << std::endl;
		char line[100];
		fgets(line,100,fp);
		sscanf(line,"%d",&length);
		fclose(fp);
		if(length < 4) length = 4; //don't allow shorter than 4
	}
	else
		__MOUT_INFO__ <<  "Sequence length file NOT found: " << SEQUENCE_FILE_NAME << std::endl;

	__MOUT__ << "Sequence length = " << length << std::endl;

	securityCode_ = "";

	static const char alphanum[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";

	srand(0);//time(0));

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
void OtsConfigurationWizardSupervisor::tooltipRequest(xgi::Input * in, xgi::Output * out)
throw (xgi::exception::Exception)
{
	cgicc::Cgicc cgi(in);

	std::string Command = CgiDataUtilities::getData(cgi, "RequestType");
	__MOUT__ << Command <<  std::endl;

	std::string submittedSequence = CgiDataUtilities::postData(cgi, "sequence");

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
				CgiDataUtilities::getData(cgi, "doNeverShow") == "1"?true:false);

	}
	else
		__MOUT__ << "Command Request, " << Command << ", not recognized." << std::endl;

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
	__MOUT__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;
	*out << "Unauthorized Request.";
}

//========================================================================================================================
void OtsConfigurationWizardSupervisor::verification(xgi::Input * in, xgi::Output * out )
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
		__MOUT__ << "***Successfully authenticated security sequence. " <<
				time(0) << std::endl;
	}

	*out << "<!DOCTYPE HTML><html lang='en'><head><title>ots wiz</title></head>" <<
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
		__MOUT__ << "Unauthorized Request made, security sequence doesn't match!" << std::endl;
		return;
	}
	else
	{
		__MOUT__ << "***Successfully authenticated security sequence." << std::endl;
	}
	//SECURITY CHECK END ****


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
		__SS__ << "Error opening file: "<< securityFileName << std::endl;
		__MOUT_ERR__ << "\n" << ss.str();
		//throw std::runtime_error(ss.str());
		//return;
		security = "DigestAccessAuthentication"; //default security when no file exists
	}
	if(securityFile.is_open())
	{
		//__MOUT__ << "Opened File: " << securityFileName << std::endl;
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
