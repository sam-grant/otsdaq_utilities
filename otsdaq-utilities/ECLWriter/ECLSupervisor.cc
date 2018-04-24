#include "otsdaq-utilities/ECLWriter/ECLSupervisor.h"
#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/Macros/CoutHeaderMacros.h"
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"
#include "otsdaq-core/SOAPUtilities/SOAPUtilities.h"
#include "otsdaq-core/SOAPUtilities/SOAPParameters.h"
#include "otsdaq-core/SOAPUtilities/SOAPCommand.h"
#include "otsdaq-core/ConfigurationInterface/ConfigurationManager.h"
#include "otsdaq-core/ConfigurationPluginDataFormats/XDAQContextConfiguration.h"
#include "otsdaq-core/ConfigurationDataFormats/ConfigurationGroupKey.h"

#include "otsdaq-utilities/ECLWriter/ECLConnection.h"

#include <dirent.h> /*DIR and dirent*/
#include <sys/stat.h> /*mkdir*/

#include <xdaq/NamespaceURI.h>

#include <iostream>


using namespace ots;

#undef 	__MF_SUBJECT__
#define __MF_SUBJECT__ "ECL"

XDAQ_INSTANTIATOR_IMPL(ECLSupervisor)

//========================================================================================================================
ECLSupervisor::ECLSupervisor(xdaq::ApplicationStub * s) 
	: xdaq::Application(s)
	, SOAPMessenger(this)
	, RunControlStateMachine("ECLSupervisor")
	, theConfigurationManager_(new ConfigurationManager)//(Singleton<ConfigurationManager>::getInstance()) //I always load the full config but if I want to load a partial configuration (new ConfigurationManager)
	, supervisorContextUID_(theConfigurationManager_->__GET_CONFIG__(XDAQContextConfiguration)->getContextUID(getApplicationContext()->getContextDescriptor()->getURL()))
	, supervisorApplicationUID_(theConfigurationManager_->__GET_CONFIG__(XDAQContextConfiguration)->getApplicationUID
	(
		getApplicationContext()->getContextDescriptor()->getURL(),
		getApplicationDescriptor()->getLocalId()
	))
	, supervisorConfigurationPath_("/" + supervisorContextUID_ + "/LinkToApplicationConfiguration/" + supervisorApplicationUID_ + "/LinkToSupervisorConfiguration")
	, theRemoteWebUsers_(this)
{
	INIT_MF("ECLSupervisor");
	__COUT__ << __PRETTY_FUNCTION__ << std::endl;
	__COUT__ << __PRETTY_FUNCTION__ << std::endl;
	__COUT__ << __PRETTY_FUNCTION__ << std::endl;
	__COUT__ << __PRETTY_FUNCTION__ << std::endl;
	__COUT__ << __PRETTY_FUNCTION__ << std::endl;
	__COUT__ << __PRETTY_FUNCTION__ << std::endl;
	__COUT__ << __PRETTY_FUNCTION__ << std::endl;
	__COUT__ << __PRETTY_FUNCTION__ << std::endl;
	__COUT__ << __PRETTY_FUNCTION__ << std::endl;
	__COUT__ << __PRETTY_FUNCTION__ << std::endl;


	__COUT__ << __PRETTY_FUNCTION__ << "done data manager" << std::endl;
	xgi::bind(this, &ECLSupervisor::Default, "Default");
	xgi::bind(this, &ECLSupervisor::request, "request");

	xgi::bind(this, &ECLSupervisor::dataRequest, "dataRequest");


	xgi::bind(this, &ECLSupervisor::safari, "safari");

	init();
}

//========================================================================================================================
ECLSupervisor::~ECLSupervisor(void)
{
	destroy();
}
//========================================================================================================================
void ECLSupervisor::init(void)
{
	//called by constructor
	allSupervisorInfo_.init(getApplicationContext());
}

//========================================================================================================================
void ECLSupervisor::destroy(void)
{
	//called by destructor
	delete theConfigurationManager_;
}

//========================================================================================================================
void ECLSupervisor::Default(xgi::Input * in, xgi::Output * out)

{
	//__COUT__ << this->getApplicationContext()->getURL() << __E__;

	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/ECL.html?urn=" <<
		this->getApplicationDescriptor()->getLocalId() << "'></frameset></html>";

}

//========================================================================================================================
void ECLSupervisor::safari(xgi::Input * in, xgi::Output * out)

{

	*out << "<!DOCTYPE HTML><html lang='en'><iframe style='width:100%;height:100%;position:absolute;left:0;top:0;border:0;padding:0;margin:0;' src='/WebPath/html/ECL.html?urn=" <<
		this->getApplicationDescriptor()->getLocalId() << "'></iframe></html>";

}


//========================================================================================================================
void ECLSupervisor::dataRequest(xgi::Input * in, xgi::Output * out)

{
	cgicc::Cgicc cgi(in);
	std::string Command;
	if ((Command = CgiDataUtilities::postData(cgi, "RequestType")) == "")
		Command = cgi("RequestType"); //from GET or POST

	__COUT__ << "Command " << Command << std::endl;

	HttpXmlDocument xmldoc;
	uint8_t userPermissions;
	std::string userName;

	//**** start LOGIN GATEWAY CODE ***//
	//check cookieCode, sequence, userWithLock, and permissions access all in one shot!
	{
		bool automaticCommand = false; //automatic commands should not refresh cookie code.. only user initiated commands should!

		bool checkLock = false;
		bool needUserName = false;//Command == "setUserPreferences" || Command == "getUserPreferences";
		bool requireLock = false;

		if (!theRemoteWebUsers_.xmlLoginGateway(
			cgi,
			out,
			&xmldoc,
			allSupervisorInfo_,
			&userPermissions,  			//acquire user's access level (optionally null pointer)
			!automaticCommand,			//true/false refresh cookie code
			1, 							//set access level requirement to pass gateway
			checkLock,					//true/false enable check that system is unlocked or this user has the lock
			requireLock,				//true/false requires this user has the lock to proceed
			0,//&userWithLock,			//acquire username with lock (optionally null pointer)
			(needUserName ? &userName : 0)	//acquire username of this user (optionally null pointer)
			, 0//&displayName			//acquire user's Display Name
			, 0//&activeSessionIndex		//acquire user's session index associated with the cookieCode
		))
		{	//failure
			//std::cout << out->str() << std::endl; //could print out return string on failure
			return;
		}
	}
	//done checking cookieCode, sequence, userWithLock, and permissions access all in one shot!
	//**** end LOGIN GATEWAY CODE ***//

	//
	if (Command == "getRawData")
	{
	}
	//return xml doc holding server response
	xmldoc.outputXmlDocument((std::ostringstream*) out, false);
}


//========================================================================================================================
void ECLSupervisor::request(xgi::Input * in, xgi::Output * out)

{


	cgicc::Cgicc cgi(in);
	std::string Command;
	if ((Command = CgiDataUtilities::postData(cgi, "RequestType")) == "")
		Command = cgi("RequestType"); //from GET or POST

	//__COUT__ << "Command " << Command << " files: " << cgi.getFiles().size() << std::endl;

	//Commands
	//	getGeometry
	//	getEvents
	//	getRoot
	//	getDirectoryContents
	//	setUserPreferences
	//	getUserPreferences

	HttpXmlDocument xmldoc;
	uint8_t userPermissions;
	std::string userName;

	//**** start LOGIN GATEWAY CODE ***//
	//check cookieCode, sequence, userWithLock, and permissions access all in one shot!
	{
		bool automaticCommand = false;//Command == "getRoot" || Command == "getEvents"; //automatic commands should not refresh cookie code.. only user initiated commands should!

		bool checkLock = false;
		bool needUserName = Command == "setUserPreferences" || Command == "getUserPreferences";
		bool requireLock = false;

		if (!theRemoteWebUsers_.xmlLoginGateway(
			cgi,
			out,
			&xmldoc,
			allSupervisorInfo_,
			&userPermissions,  			//acquire user's access level (optionally null pointer)
			!automaticCommand,			//true/false refresh cookie code
			1, 							//set access level requirement to pass gateway
			checkLock,					//true/false enable check that system is unlocked or this user has the lock
			requireLock,				//true/false requires this user has the lock to proceed
			0,//&userWithLock,			//acquire username with lock (optionally null pointer)
			(needUserName ? &userName : 0)	//acquire username of this user (optionally null pointer)
			, 0//&displayName			//acquire user's Display Name
			, 0//&activeSessionIndex		//acquire user's session index associated with the cookieCode
		))
		{	//failure
			//std::cout << out->str() << std::endl; //could print out return string on failure
			return;
		}
	}
	//done checking cookieCode, sequence, userWithLock, and permissions access all in one shot!
	//**** end LOGIN GATEWAY CODE ***//

	if (Command == "setUserPreferences")
	{
		std::string radioSelect = CgiDataUtilities::getData(cgi, "radioSelect");
		std::string autoRefresh = CgiDataUtilities::getData(cgi, "autoRefresh");
		std::string autoHide = CgiDataUtilities::getData(cgi, "autoHide");
		std::string hardRefresh = CgiDataUtilities::getData(cgi, "hardRefresh");
		std::string autoRefreshPeriod = CgiDataUtilities::getData(cgi, "autoRefreshPeriod");

		__COUT__ << "radioSelect: " << radioSelect << std::endl;
		__COUT__ << "autoRefresh: " << autoRefresh << std::endl;
		__COUT__ << "autoHide: " << autoHide << std::endl;
		__COUT__ << "hardRefresh: " << hardRefresh << std::endl;
		__COUT__ << "autoRefreshPeriod: " << autoRefreshPeriod << std::endl;


	}
	else
		__COUT__ << "Command request, " << Command << ", not recognized." << std::endl;
	//return xml doc holding server response
	xmldoc.outputXmlDocument((std::ostringstream*) out, false);
}


//========================================================================================================================
void ECLSupervisor::transitionConfiguring(toolbox::Event::Reference e)

{

	//try
	{
		//theConfigurationGroupKey_ = theConfigurationManager_->makeTheConfigurationGroupKey(atoi(SOAPUtilities::translate(theStateMachine_.getCurrentMessage()).getParameters().getValue("ConfigurationGroupKey").c_str()));
		//theConfigurationManager_->activateConfigurationGroupKey(theConfigurationGroupKey_,0);

		std::pair<std::string /*group name*/, ConfigurationGroupKey> theGroup(
			SOAPUtilities::translate(theStateMachine_.getCurrentMessage()).
			getParameters().getValue("ConfigurationGroupName"),
			ConfigurationGroupKey(SOAPUtilities::translate(theStateMachine_.getCurrentMessage()).
				getParameters().getValue("ConfigurationGroupKey")));

		__COUT__ << "Configuration group name: " << theGroup.first << " key: " <<
			theGroup.second << std::endl;

		theConfigurationManager_->loadConfigurationGroup(
			theGroup.first,
			theGroup.second, true);


		ConfigurationTree configLinkNode = theConfigurationManager_->getSupervisorConfigurationNode(
			supervisorContextUID_, supervisorApplicationUID_);

		ECLUser = configLinkNode.getNode("ECLUserName").getValue<std::string>();
		ECLHost = configLinkNode.getNode("ECLInstanceURL").getValue<std::string>();
		ECLPwd = configLinkNode.getNode("ECLPassword").getValue<std::string>();
		ExperimentName = configLinkNode.getNode("ExperimentName").getValue<std::string>();

	}
	//catch(...)
	//{
	//	{__SS__;throw std::runtime_error(ss.str()+"Error configuring the visual supervisor most likely a plugin name is wrong or your configuration table is outdated and doesn't match the new plugin definition!");}
	//}
}

//========================================================================================================================
void ECLSupervisor::transitionStarting(toolbox::Event::Reference e)

{
	try
	{
		__COUT_INFO__ << "ECLSupervisor sending Start Run log message to ECL" << std::endl;
		run = SOAPUtilities::translate(theStateMachine_.getCurrentMessage()).getParameters().getValue("RunNumber");
		run_start = std::chrono::steady_clock::now();
		Write(false, false);
	}
	catch (...)
	{
		__COUT_INFO__ << "ERROR! Couldn't Start the ECLSupervisor" << std::endl;
	}
}

//========================================================================================================================
void ECLSupervisor::transitionStopping(toolbox::Event::Reference e)

{
	try
	{
		__COUT_INFO__ << "ECLSupervisor sending Stop Run log message to ECL" << std::endl;
		Write(true, false);
	}
	catch (...)
	{
		__COUT_INFO__ << "ERROR! Couldn't Stop the ECLSupervisor" << std::endl;
	}
}


//========================================================================================================================
void ECLSupervisor::transitionPausing(toolbox::Event::Reference e)

{
	try
	{
		__COUT_INFO__ << "ECLSupervisor sending Pause Run log message to ECL" << std::endl;
		Write(true, true);
	}
	catch (...)
	{
		__COUT_INFO__ << "ERROR! Couldn't Pause the ECLSupervisor" << std::endl;
	}
}

//========================================================================================================================
void ECLSupervisor::transitionResuming(toolbox::Event::Reference e)

{
	try
	{
		__COUT_INFO__ << "ECLSupervisor sending Resume Run log message to ECL" << std::endl;
		run_start = std::chrono::steady_clock::now();
		Write(false, true);
	}
	catch (...)
	{
		__COUT_INFO__ << "ERROR! Couldn't Resume the ECLSupervisor" << std::endl;
	}
}

//========================================================================================================================
//xoap::MakeSystemLogbookEntry
//	make a system logbook entry into active experiment's logbook from Supervisor only
//	TODO: (how to enforce?)
xoap::MessageReference ECLSupervisor::MakeSystemLogbookEntry(xoap::MessageReference msg)

{
	SOAPParameters parameters("EntryText");
	//	SOAPParametersV parameters(1);
	//	parameters[0].setName("EntryText");
	receive(msg, parameters);
	std::string EntryText = parameters.getValue("EntryText");

	__COUT__ << "Received External Supervisor System Entry " << EntryText << std::endl;

	std::string retStr = "Success";

	ECLEntry_t eclEntry;
	eclEntry.author(ECLUser);
	eclEntry.category("System Entry");
	Form_t form;
	Field_t field;
	Form_t::field_sequence fields;
	std::string users = theRemoteWebUsers_.getActiveUserList(allSupervisorInfo_.getGatewayDescriptor());

	form.name(ExperimentName + " System Logbook Entry");

	field = Field_t(EscapeECLString(run), "RunNumber");
	fields.push_back(field);

	field = Field_t(EscapeECLString(users), "ActiveUsers");
	fields.push_back(field);

	field = Field_t(EscapeECLString(EntryText), "Entry");
	fields.push_back(field);

	ECLConnection eclConn(ECLUser, ECLPwd, ECLHost);
	if (!eclConn.Post(eclEntry)) {
		retStr = "Failure";
	}

	//fill return parameters
	SOAPParameters retParameters("Status", retStr);

	return SOAPUtilities::makeSOAPMessageReference("LogbookEntryStatusResponse", retParameters);
}


int ECLSupervisor::Write(bool atEnd, bool pause)
{
	ECLEntry_t eclEntry;
	eclEntry.author(ECLUser);
	eclEntry.category("Run History");
	Form_t form;
	Field_t field;
	Form_t::field_sequence fields;
	std::string users = theRemoteWebUsers_.getActiveUserList(allSupervisorInfo_.getGatewayDescriptor());

	if (!atEnd) {

		if (pause) form.name(ExperimentName + " OTSDAQ Resume Run");
		else 			form.name(ExperimentName + " OTSDAQ Start Run");


		field = Field_t(EscapeECLString(run), "RunNumber");
		fields.push_back(field);

		field = Field_t(EscapeECLString(users), "ActiveUsers");
		fields.push_back(field);

		form.field(fields);

		eclEntry.form(form);

	}
	else {
		if (pause) 				form.name(ExperimentName + " OTSDAQ Pause Run");
		else 			form.name(ExperimentName + " OTSDAQ End Run");


		field = Field_t(EscapeECLString(run), "RunNumber");
		fields.push_back(field);

		field = Field_t(EscapeECLString(users), "ActiveUsers");
		fields.push_back(field);

		int dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - run_start).count();
		int dur_s = dur / 1000;
		dur = dur % 1000;
		int dur_m = dur_s / 60;
		dur_s = dur_s % 60;
		int dur_h = dur_m / 60;
		dur_m = dur_m % 60;

		std::ostringstream dur_ss;
		dur_ss << dur_h << ":" << dur_m << ":" << dur_s << "." << dur;

		field = Field_t(EscapeECLString(dur_ss.str()), "Duration");
		fields.push_back(field);

		form.field(fields);

		eclEntry.form(form);
	}

	ECLConnection eclConn(ECLUser, ECLPwd, ECLHost);
	if (!eclConn.Post(eclEntry)) {
		return -1;
	}

	return 0;
}

//************************************************************

std::string ECLSupervisor::EscapeECLString(std::string input)
{
	std::string output = input;
	size_t pos = output.find('&');
	while (pos != std::string::npos)
	{
		output = output.replace(pos, 1, "&amp;");
		pos = output.find('&', pos + 2);
	}

	pos = output.find('"');
	while (pos != std::string::npos)
	{
		output = output.replace(pos, 1, "&quot;");
		pos = output.find('"', pos + 1);
	}

	pos = output.find('\'');
	while (pos != std::string::npos)
	{
		output = output.replace(pos, 1, "&apos;");
		pos = output.find('\'', pos + 1);
	}

	pos = output.find('<');
	while (pos != std::string::npos)
	{
		output = output.replace(pos, 1, "&lt;");
		pos = output.find('<', pos + 1);
	}

	pos = output.find('>');
	while (pos != std::string::npos)
	{
		output = output.replace(pos, 1, "&gt;");
		pos = output.find('>', pos + 1);
	}

	return output;
}



