#include "otsdaq-utilities/ECLWriter/ECLSupervisor.h"
#include "otsdaq/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq/ConfigurationInterface/ConfigurationManager.h"
#include "otsdaq/Macros/CoutMacros.h"
#include "otsdaq/MessageFacility/MessageFacility.h"
#include "otsdaq/SOAPUtilities/SOAPCommand.h"
#include "otsdaq/SOAPUtilities/SOAPParameters.h"
#include "otsdaq/SOAPUtilities/SOAPUtilities.h"
#include "otsdaq/TablePlugins/XDAQContextTable.h"
#include "otsdaq/XmlUtilities/HttpXmlDocument.h"

#include "otsdaq-utilities/ECLWriter/ECLConnection.h"

#include <dirent.h>   /*DIR and dirent*/
#include <sys/stat.h> /*mkdir*/

#include <xdaq/NamespaceURI.h>

#include <iomanip>
#include <iostream>
#include "otsdaq/TableCore/TableGroupKey.h"

using namespace ots;

#undef __MF_SUBJECT__
#define __MF_SUBJECT__ "ECL"

XDAQ_INSTANTIATOR_IMPL(ECLSupervisor)

//==============================================================================
ECLSupervisor::ECLSupervisor(xdaq::ApplicationStub* s)
    : CoreSupervisorBase(s)
    , theConfigurationManager_(
          new ConfigurationManager)  //(Singleton<ConfigurationManager>::getInstance())
                                     ////I always load the full config but if I want to
                                     // load a partial configuration (new
                                     // ConfigurationManager)
    , supervisorContextUID_(
          theConfigurationManager_->__GET_CONFIG__(XDAQContextTable)
              ->getContextUID(getApplicationContext()->getContextDescriptor()->getURL()))
    , supervisorApplicationUID_(
          theConfigurationManager_->__GET_CONFIG__(XDAQContextTable)
              ->getApplicationUID(
                  getApplicationContext()->getContextDescriptor()->getURL(),
                  getApplicationDescriptor()->getLocalId()))
    , supervisorConfigurationPath_("/" + supervisorContextUID_ +
                                   "/LinkToApplicationTable/" +
                                   supervisorApplicationUID_ + "/LinkToSupervisorTable")
{
	INIT_MF("." /*directory used is USER_DATA/LOG/.*/);
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

	init();
}

//==============================================================================
ECLSupervisor::~ECLSupervisor(void) { destroy(); }
//==============================================================================
void ECLSupervisor::init(void)
{
	// called by constructor
	// allSupervisorInfo_.init(getApplicationContext());
}

//==============================================================================
void ECLSupervisor::destroy(void)
{
	// called by destructor
	delete theConfigurationManager_;
}

//==============================================================================
void ECLSupervisor::defaultPage(xgi::Input* in, xgi::Output* out)

{
	//__COUT__ << this->getApplicationContext()->getURL() << __E__;

	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame "
	        "src='/WebPath/html/ECL.html?urn="
	     << this->getApplicationDescriptor()->getLocalId() << "'></frameset></html>";
}

//==============================================================================
void ECLSupervisor::transitionConfiguring(toolbox::Event::Reference e)

{
	// try
	{
		// theConfigurationTableGroupKey_ =
		// theConfigurationManager_->makeTheTableGroupKey(atoi(SOAPUtilities::translate(theStateMachine_.getCurrentMessage()).getParameters().getValue("TableGroupKey").c_str()));
		// theConfigurationManager_->activateTableGroupKey(theConfigurationTableGroupKey_,0);

		std::pair<std::string /*group name*/, TableGroupKey> theGroup(
		    SOAPUtilities::translate(theStateMachine_.getCurrentMessage())
		        .getParameters()
		        .getValue("ConfigurationTableGroupName"),
		    TableGroupKey(SOAPUtilities::translate(theStateMachine_.getCurrentMessage())
		                      .getParameters()
		                      .getValue("ConfigurationTableGroupKey")));

		__COUT__ << "Configuration table group name: " << theGroup.first
		         << " key: " << theGroup.second << std::endl;

		theConfigurationManager_->loadTableGroup(theGroup.first, theGroup.second, true);

		ConfigurationTree configLinkNode =
		    theConfigurationManager_->getSupervisorTableNode(supervisorContextUID_,
		                                                     supervisorApplicationUID_);

		ECLUser        = configLinkNode.getNode("ECLUserName").getValue<std::string>();
		ECLHost        = configLinkNode.getNode("ECLInstanceURL").getValue<std::string>();
		ECLPwd         = configLinkNode.getNode("ECLPassword").getValue<std::string>();
		ExperimentName = configLinkNode.getNode("ExperimentName").getValue<std::string>();
	}
	// catch(...)
	//{
	//	{__SS__;__THROW__(ss.str()+"Error configuring the visual supervisor most likely a
	// plugin name is wrong or your configuration table is outdated and doesn't match the
	// new plugin definition!");}
	//}
}

//==============================================================================
void ECLSupervisor::transitionStarting(toolbox::Event::Reference e)

{
	try
	{
		__COUT_INFO__ << "ECLSupervisor sending Start Run log message to ECL"
		              << std::endl;
		run = SOAPUtilities::translate(theStateMachine_.getCurrentMessage())
		          .getParameters()
		          .getValue("RunNumber");
		run_start   = std::chrono::steady_clock::now();
		duration_ms = 0;
		Write(WriteState::kStart);
	}
	catch(...)
	{
		__COUT_INFO__ << "ERROR! Couldn't Start the ECLSupervisor" << std::endl;
	}
}

//==============================================================================
void ECLSupervisor::transitionStopping(toolbox::Event::Reference e)

{
	try
	{
		__COUT_INFO__ << "ECLSupervisor sending Stop Run log message to ECL" << std::endl;
		Write(WriteState::kStop);
	}
	catch(...)
	{
		__COUT_INFO__ << "ERROR! Couldn't Stop the ECLSupervisor" << std::endl;
	}
}

//==============================================================================
void ECLSupervisor::transitionPausing(toolbox::Event::Reference e)

{
	try
	{
		__COUT_INFO__ << "ECLSupervisor sending Pause Run log message to ECL"
		              << std::endl;
		Write(WriteState::kPause);
		duration_ms += std::chrono::duration_cast<std::chrono::milliseconds>(
		                   std::chrono::steady_clock::now() - run_start)
		                   .count();
	}
	catch(...)
	{
		__COUT_INFO__ << "ERROR! Couldn't Pause the ECLSupervisor" << std::endl;
	}
}

//==============================================================================
void ECLSupervisor::transitionResuming(toolbox::Event::Reference e)

{
	try
	{
		__COUT_INFO__ << "ECLSupervisor sending Resume Run log message to ECL"
		              << std::endl;
		run_start = std::chrono::steady_clock::now();
		Write(WriteState::kResume);
	}
	catch(...)
	{
		__COUT_INFO__ << "ERROR! Couldn't Resume the ECLSupervisor" << std::endl;
	}
}

void ECLSupervisor::enteringError(toolbox::Event::Reference e)
{
	try
	{
		__COUT_INFO__ << "ECLSupervisor sending Error log message to ECL" << std::endl;
		Write(WriteState::kError);
	}
	catch(...)
	{
		__COUT_INFO__ << "ERROR! Couldn't Error the ECLSupervisor" << std::endl;
	}
}

//==============================================================================
// xoap::MakeSystemLogbookEntry
//	make a system logbook entry into active experiment's logbook from Supervisor only
//	TODO: (how to enforce?)
xoap::MessageReference ECLSupervisor::MakeSystemLogbookEntry(xoap::MessageReference msg)

{
	SOAPParameters parameters("EntryText");
	//	SOAPParametersV parameters(1);
	//	parameters[0].setName("EntryText");
	SOAPUtilities::receive(msg, parameters);
	std::string EntryText = parameters.getValue("EntryText");

	__COUT__ << "Received External Supervisor System Entry " << EntryText << std::endl;

	std::string retStr = "Success";

	ECLEntry_t eclEntry;
	eclEntry.author(ECLUser);
	eclEntry.category("Facility/DAQ");
	Form_t                 form;
	Field_t                field;
	Form_t::field_sequence fields;
	std::string            users =
	    theRemoteWebUsers_.getActiveUserList(allSupervisorInfo_.getGatewayDescriptor());

	form.name("OTSDAQ System Logbook Entry");

	field = Field_t(EscapeECLString(ExperimentName), "Experiment");
	fields.push_back(field);

	field = Field_t(EscapeECLString(run), "RunNumber");
	fields.push_back(field);

	field = Field_t(EscapeECLString(users), "ActiveUsers");
	fields.push_back(field);

	field = Field_t(EscapeECLString(EntryText), "Entry");
	fields.push_back(field);

	ECLConnection eclConn(ECLUser, ECLPwd, ECLHost);
	if(!eclConn.Post(eclEntry))
	{
		retStr = "Failure";
	}

	// fill return parameters
	SOAPParameters retParameters("Status", retStr);

	return SOAPUtilities::makeSOAPMessageReference("LogbookEntryStatusResponse",
	                                               retParameters);
}

int ECLSupervisor::Write(WriteState state)
{
	ECLEntry_t eclEntry;
	eclEntry.author(ECLUser);
	eclEntry.category("Facility/DAQ");
	Form_t                 form;
	Field_t                field;
	Form_t::field_sequence fields;
	std::string            users =
	    theRemoteWebUsers_.getActiveUserList(allSupervisorInfo_.getGatewayDescriptor());

	switch(state)
	{
	case WriteState::kStart:
		form.name("OTSDAQ Start Run");
		break;
	case WriteState::kStop:
		form.name("OTSDAQ Stop Run");
		break;
	case WriteState::kResume:
		form.name("OTSDAQ Resume Run");
		break;
	case WriteState::kPause:
		form.name("OTSDAQ Pause Run");
		break;
	case WriteState::kError:
		form.name("OTSDAQ Run Error");
		break;
	}

	field = Field_t(EscapeECLString(ExperimentName), "Experiment");
	fields.push_back(field);

	field = Field_t(EscapeECLString(run), "RunNumber");
	fields.push_back(field);

	field = Field_t(EscapeECLString(users), "ActiveUsers");
	fields.push_back(field);

	if(state != WriteState::kStart && state != WriteState::kResume)
	{
		int dur = std::chrono::duration_cast<std::chrono::milliseconds>(
		              std::chrono::steady_clock::now() - run_start)
		              .count() +
		          duration_ms;
		int dur_s = dur / 1000;
		dur       = dur % 1000;
		int dur_m = dur_s / 60;
		dur_s     = dur_s % 60;
		int dur_h = dur_m / 60;
		dur_m     = dur_m % 60;

		std::ostringstream dur_ss;
		dur_ss << std::setw(2) << std::setfill('0') << dur_h << ":" << std::setw(2)
		       << std::setfill('0') << dur_m << ":" << std::setw(2) << std::setfill('0')
		       << dur_s << "." << dur;

		field = Field_t(EscapeECLString(dur_ss.str()), "Duration");
		fields.push_back(field);
	}

	form.field(fields);

	eclEntry.form(form);

	ECLConnection eclConn(ECLUser, ECLPwd, ECLHost);
	if(!eclConn.Post(eclEntry))
	{
		return -1;
	}

	return 0;
}

//************************************************************

std::string ECLSupervisor::EscapeECLString(std::string input)
{
	std::string output = input;
	size_t      pos    = output.find('&');
	while(pos != std::string::npos)
	{
		output = output.replace(pos, 1, "&amp;");
		pos    = output.find('&', pos + 2);
	}

	pos = output.find('"');
	while(pos != std::string::npos)
	{
		output = output.replace(pos, 1, "&quot;");
		pos    = output.find('"', pos + 1);
	}

	pos = output.find('\'');
	while(pos != std::string::npos)
	{
		output = output.replace(pos, 1, "&apos;");
		pos    = output.find('\'', pos + 1);
	}

	pos = output.find('<');
	while(pos != std::string::npos)
	{
		output = output.replace(pos, 1, "&lt;");
		pos    = output.find('<', pos + 1);
	}

	pos = output.find('>');
	while(pos != std::string::npos)
	{
		output = output.replace(pos, 1, "&gt;");
		pos    = output.find('>', pos + 1);
	}

	return output;
}
