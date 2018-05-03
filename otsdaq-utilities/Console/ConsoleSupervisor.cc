#include "otsdaq-utilities/Console/ConsoleSupervisor.h"
#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/Macros/CoutMacros.h"
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"
#include "otsdaq-core/NetworkUtilities/ReceiverSocket.h"
#include <xdaq/NamespaceURI.h>

#include <iostream>
#include <fstream>
#include <string>
#include <dirent.h> 	//for DIR
#include <sys/stat.h> 	//for mkdir
#include <thread>       //for std::thread

using namespace ots;


XDAQ_INSTANTIATOR_IMPL(ConsoleSupervisor)


#define USER_CONSOLE_PREF_PATH	std::string(getenv("SERVICE_DATA_PATH")) + "/ConsolePreferences/"
#define USERS_PREFERENCES_FILETYPE 		"pref"
#define QUIET_CFG_FILE		std::string(getenv("USER_DATA")) + "/MessageFacilityConfigurations/QuietForwarderGen.cfg"

#undef 	__MF_SUBJECT__
#define __MF_SUBJECT__ "Console"


//========================================================================================================================
ConsoleSupervisor::ConsoleSupervisor(xdaq::ApplicationStub* stub)
throw (xdaq::exception::Exception)
: CoreSupervisorBase	(stub)
, writePointer_     	(0)
, messageCount_     	(0)
{
	INIT_MF("ConsoleSupervisor");

	//attempt to make directory structure (just in case)
	mkdir(((std::string)USER_CONSOLE_PREF_PATH).c_str(), 0755);

	init();
}

//========================================================================================================================
ConsoleSupervisor::~ConsoleSupervisor(void)
{
	destroy();
}
//========================================================================================================================
void ConsoleSupervisor::init(void)
{
	//start mf msg listener
	std::thread([](ConsoleSupervisor *cs){ ConsoleSupervisor::MFReceiverWorkLoop(cs); },this).detach();
}

//========================================================================================================================
void ConsoleSupervisor::destroy(void)
{
	//called by destructor
}


//========================================================================================================================
//MFReceiverWorkLoop ~~
//	Thread for printing Message Facility messages without decorations
//	Note: Uses std::mutex to avoid conflict with reading thread.
void ConsoleSupervisor::MFReceiverWorkLoop(ConsoleSupervisor *cs)
{
	__COUT__ << std::endl;

	std::string configFile = QUIET_CFG_FILE;
	FILE *fp = fopen(configFile.c_str(),"r");
	if(!fp)
	{
		__SS__ << "File with port info could not be loaded: " <<
				QUIET_CFG_FILE << std::endl;
		__COUT__ << "\n" << ss.str();
		throw std::runtime_error(ss.str());
	}
	char tmp[100];
	fgets(tmp,100,fp); //receive port (ignore)
	fgets(tmp,100,fp); //destination port *** used here ***
	int myport;
	sscanf(tmp,"%*s %d",&myport);

	fgets(tmp,100,fp); //destination ip *** used here ***
	char myip[100];
	sscanf(tmp,"%*s %s",myip);
	fclose(fp);

	ReceiverSocket rsock(myip,myport); //Take Port from Configuration
	try
	{
		rsock.initialize();
	}
	catch(...)
	{
		//lockout the messages array for the remainder of the scope
		//this guarantees the reading thread can safely access the messages
		std::lock_guard<std::mutex> lock(cs->messageMutex_);

		//generate special message to indicate failed socket
		__SS__ << "FATAL Console error. Could not initialize socket on port " <<
				myport << ". Perhaps it is already in use? Exiting Console receive loop." << std::endl;
		__COUT__ << ss.str();


		cs->messages_[cs->writePointer_].set("||0|||Error|Console|||0||ConsoleSupervisor|" +
				ss.str(),
				cs->messageCount_++);


		if(++cs->writePointer_ == cs->messages_.size()) //handle wrap-around
			cs->writePointer_ = 0;

		return;
	}

	std::string buffer;
	int i = 0;
	int heartbeatCount = 0;
	int selfGeneratedMessageCount = 0;

	std::map<unsigned int, unsigned int> sourceLastSequenceID; //map from sourceID to lastSequenceID to identify missed messages
	unsigned int newSourceId;
	unsigned int newSequenceId;

	while(1)
	{
		//if receive succeeds display message

		if(rsock.receive(buffer,1 /*timeoutSeconds*/,0/*timeoutUSeconds*/,
				false /*verbose*/) != -1)
		{
			if(i != 200)
			{
				__COUT__ << "Console has first message." << std::endl;
				i = 200; //mark so things are good for all time. (this indicates things are configured to be sent here)

				mf::LogDebug (__MF_SUBJECT__) << __COUT_HDR_FL__ << "DEBUG messages look like this." << std::endl;
				mf::LogInfo (__MF_SUBJECT__) << __COUT_HDR_FL__ << "INFO messages look like this." << std::endl;
				mf::LogWarning (__MF_SUBJECT__) << __COUT_HDR_FL__ << "WARNING messages look like this." << std::endl;
				mf::LogError (__MF_SUBJECT__) << __COUT_HDR_FL__ << "ERROR messages look like this." << std::endl;
			}

			if(selfGeneratedMessageCount)
				--selfGeneratedMessageCount; //decrement internal message count
			else
				heartbeatCount = 0; //reset heartbeat if external messages are coming through

			//__COUT__ << buffer << std::endl;

			//lockout the messages array for the remainder of the scope
			//this guarantees the reading thread can safely access the messages
			std::lock_guard<std::mutex> lock(cs->messageMutex_);

			cs->messages_[cs->writePointer_].set(buffer,cs->messageCount_++);


			//check if sequence ID is out of order
			newSourceId = cs->messages_[cs->writePointer_].getSourceIDAsNumber();
			newSequenceId = cs->messages_[cs->writePointer_].getSequenceIDAsNumber();

			//__COUT__ << "newSourceId: " << newSourceId << std::endl;
			//__COUT__ << "newSequenceId: " << newSequenceId << std::endl;

			if(sourceLastSequenceID.find(newSourceId) !=
					sourceLastSequenceID.end() && //ensure not first packet received
					((newSequenceId == 0 &&
							sourceLastSequenceID[newSourceId] != (unsigned int)-1) ||  //wrap around case
						newSequenceId != sourceLastSequenceID[newSourceId] + 1)) //normal sequence case
			{
				//missed some messages!
				__SS__ << "Missed packets from " <<
						cs->messages_[cs->writePointer_].getSource() << "! Sequence IDs " <<
						sourceLastSequenceID[newSourceId] <<
						" to " << newSequenceId << "." << std::endl;
				std::cout << ss.str();

				if(++cs->writePointer_ == cs->messages_.size()) //handle wrap-around
					cs->writePointer_ = 0;

				//generate special message to indicate missed packets
				cs->messages_[cs->writePointer_].set("||0|||Warning|Console|||0||ConsoleSupervisor|" +
						ss.str(),
						cs->messageCount_++);
			}

			//save the new last sequence ID
			sourceLastSequenceID[newSourceId] = newSequenceId;

			if(++cs->writePointer_ == cs->messages_.size()) //handle wrap-around
				cs->writePointer_ = 0;

		}
		else
		{
			if(i < 120) //if nothing received for 120 seconds, then something is wrong with Console configuration
				++i;

			sleep(1); //sleep one second, if timeout

			//every 60 heartbeatCount (2 seconds each = 1 sleep and 1 timeout) print a heartbeat message
			if(i != 200 ||  //show first message, if not already a message
					(heartbeatCount < 60*5 && heartbeatCount%60 == 59)) //every ~2 min for first 5 messages
			{
				++selfGeneratedMessageCount; //increment internal message count
				mf::LogDebug (__MF_SUBJECT__) << "Console is alive and waiting... (if no messages, next heartbeat is in approximately two minutes)" << std::endl;
			}
			else if(heartbeatCount%(60*30) == 59) //approx every hour
			{
				++selfGeneratedMessageCount; //increment internal message count
				mf::LogDebug (__MF_SUBJECT__) << "Console is alive and waiting a long time... (if no messages, next heartbeat is in approximately one hour)" << std::endl;
			}

			++heartbeatCount;
		}

		//if nothing received for 2 minutes seconds, then something is wrong with Console configuration
		//	after 5 seconds there is a self-send. Which will at least confirm configuration.
		//	OR if 5 generated messages and never cleared.. then the forwarding is not working.
		if(i==120 || selfGeneratedMessageCount == 5)
		{
			__COUT__ << "No messages received at Console Supervisor. Exiting Console MFReceiverWorkLoop" << std::endl;
			break; //assume something wrong, and break loop
		}
	}

}

//========================================================================================================================
void ConsoleSupervisor::defaultPage(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	__COUT__ << "ApplicationDescriptor LID=" << getApplicationDescriptor()->getLocalId() << std::endl;
	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/Console.html?urn=" <<
			getApplicationDescriptor()->getLocalId() << "'></frameset></html>";
}

//========================================================================================================================
//forceSupervisorPropertyValues
//		override to force supervisor property values (and ignore user settings)
void ConsoleSupervisor::forceSupervisorPropertyValues()
{
	CorePropertySupervisorBase::setSupervisorProperty(CorePropertySupervisorBase::SUPERVISOR_PROPERTIES.AutomatedRequestTypes,
			"GetConsoleMsgs");
//	CorePropertySupervisorBase::setSupervisorProperty(CorePropertySupervisorBase::SUPERVISOR_PROPERTIES.NeedUsernameRequestTypes,
//			"SaveUserPreferences | LoadUserPreferences");
}

//========================================================================================================================
//	Request
//		Handles Web Interface requests to Console supervisor.
//		Does not refresh cookie for automatic update checks.
void ConsoleSupervisor::request(const std::string& requestType, cgicc::Cgicc& cgiIn,
		HttpXmlDocument& xmlOut, const WebUsers::RequestUserInfo& userInfo)
throw (xgi::exception::Exception)
{
	//__COUT__ << "requestType " << requestType << std::endl;

	//Commands:
		//GetConsoleMsgs
		//SaveUserPreferences
		//LoadUserPreferences


	//Note: to report to logbook admin status use xmlOut.addTextElementToData(XML_ADMIN_STATUS,refreshTempStr_);

	if(requestType == "GetConsoleMsgs")
	{
		//lindex of -1 means first time and user just gets update lcount and lindex
        std::string lastUpdateCountStr = CgiDataUtilities::postData(cgiIn,"lcount");
        std::string lastUpdateIndexStr = CgiDataUtilities::postData(cgiIn,"lindex");

        if(lastUpdateCountStr == "" || lastUpdateIndexStr == "")
        {
    		__COUT_ERR__ << "Invalid Parameters! lastUpdateCount=" << lastUpdateCountStr <<
    				", lastUpdateIndex=" << lastUpdateIndexStr << std::endl;
    		xmlOut.addTextElementToData("Error","Error - Invalid parameters for GetConsoleMsgs.");
    		return;
        }

        clock_t lastUpdateCount;
        sscanf(lastUpdateCountStr.c_str(),"%ld",&lastUpdateCount);

        unsigned int lastUpdateIndex;
        sscanf(lastUpdateIndexStr.c_str(),"%u",&lastUpdateIndex);
//		__COUT__ << "lastUpdateCount=" << lastUpdateCount <<
//				", lastUpdateIndex=" << lastUpdateIndex << std::endl;

		insertMessageRefresh(&xmlOut,lastUpdateCount,lastUpdateIndex);
	}
	else if(requestType == "SaveUserPreferences")
	{
        int colorIndex = CgiDataUtilities::postDataAsInt(cgiIn,"colorIndex");
        int showSideBar = CgiDataUtilities::postDataAsInt(cgiIn,"showSideBar");
        int noWrap = CgiDataUtilities::postDataAsInt(cgiIn,"noWrap");
        int messageOnly = CgiDataUtilities::postDataAsInt(cgiIn,"messageOnly");
        int hideLineNumers = CgiDataUtilities::postDataAsInt(cgiIn,"hideLineNumers");

		__COUT__ << "requestType " << requestType << std::endl;
		__COUT__ << "colorIndex: " << colorIndex << std::endl;
		__COUT__ << "showSideBar: " << showSideBar << std::endl;
		__COUT__ << "noWrap: " << noWrap << std::endl;
		__COUT__ << "messageOnly: " << messageOnly << std::endl;
		__COUT__ << "hideLineNumers: " << hideLineNumers << std::endl;

		if(userInfo.username_ == "") //should never happen?
		{
			__COUT_ERR__ << "Invalid user found! user=" << userInfo.username_ << std::endl;
			xmlOut.addTextElementToData("Error","Error - InvauserInfo.username_user found.");
			return;
		}

		std::string fn = (std::string)USER_CONSOLE_PREF_PATH + userInfo.username_ + "." + (std::string)USERS_PREFERENCES_FILETYPE;

		__COUT__ << "Save preferences: " << fn << std::endl;
		FILE *fp = fopen(fn.c_str(),"w");
		if(!fp)
			{__SS__;throw std::runtime_error(ss.str()+"Could not open file: " + fn);}
		fprintf(fp,"colorIndex %d\n",colorIndex);
		fprintf(fp,"showSideBar %d\n",showSideBar);
		fprintf(fp,"noWrap %d\n",noWrap);
		fprintf(fp,"messageOnly %d\n",messageOnly);
		fprintf(fp,"hideLineNumers %d\n",hideLineNumers);
		fclose(fp);
	}
	else if(requestType == "LoadUserPreferences")
	{
		__COUT__ << "requestType " << requestType << std::endl;

		unsigned int colorIndex,showSideBar,noWrap,messageOnly,hideLineNumers;

		if(userInfo.username_ == "") //should never happen?
		{
			__COUT_ERR__ << "Invalid user found! user=" << userInfo.username_ << std::endl;
			xmlOut.addTextElementToData("Error","Error - Invalid user found.");
			return;
		}

		std::string fn = (std::string)USER_CONSOLE_PREF_PATH + userInfo.username_ + "." + (std::string)USERS_PREFERENCES_FILETYPE;

		__COUT__ << "Load preferences: " << fn << std::endl;

		FILE *fp = fopen(fn.c_str(),"r");
		if(!fp)
		{
			//return defaults
			__COUT__ << "Returning defaults." << std::endl;
			xmlOut.addTextElementToData("colorIndex","0");
			xmlOut.addTextElementToData("showSideBar","0");
			xmlOut.addTextElementToData("noWrap","1");
			xmlOut.addTextElementToData("messageOnly","0");
			xmlOut.addTextElementToData("hideLineNumers","1");
			return;
		}
		fscanf(fp,"%*s %u",&colorIndex);
		fscanf(fp,"%*s %u",&showSideBar);
		fscanf(fp,"%*s %u",&noWrap);
		fscanf(fp,"%*s %u",&messageOnly);
		fscanf(fp,"%*s %u",&hideLineNumers);
		fclose(fp);
		__COUT__ << "colorIndex: " << colorIndex << std::endl;
		__COUT__ << "showSideBar: " << showSideBar << std::endl;
		__COUT__ << "noWrap: " << noWrap << std::endl;
		__COUT__ << "messageOnly: " << messageOnly << std::endl;
		__COUT__ << "hideLineNumers: " << hideLineNumers << std::endl;

		char tmpStr[20];
		sprintf(tmpStr,"%u",colorIndex);
		xmlOut.addTextElementToData("colorIndex",tmpStr);
		sprintf(tmpStr,"%u",showSideBar);
		xmlOut.addTextElementToData("showSideBar",tmpStr);
		sprintf(tmpStr,"%u",noWrap);
		xmlOut.addTextElementToData("noWrap",tmpStr);
		sprintf(tmpStr,"%u",messageOnly);
		xmlOut.addTextElementToData("messageOnly",tmpStr);
		sprintf(tmpStr,"%u",hideLineNumers);
		xmlOut.addTextElementToData("hideLineNumers",tmpStr);
	}

}


//========================================================================================================================
//ConsoleSupervisor::insertMessageRefresh()
//	if lastUpdateClock is current, return nothing
//	else return new messages
//	(note: lastUpdateIndex==(unsigned int)-1 first time and returns as much as possible// nothing but lastUpdateClock)
//
//	format of xml:
//
//	<last_update_count/>
//	<last_update_index/>
//	<messages>
//		<message_FIELDNAME*/>
			//"Level"
			//"Label"
			//"Source"
			//"Msg"
			//"Time"
			//"Count"
//	</messages>
//
//	NOTE: Uses std::mutex to avoid conflict with writing thread. (this is the reading thread)
void ConsoleSupervisor::insertMessageRefresh(HttpXmlDocument *xmlOut,
		const time_t lastUpdateCount, const unsigned int lastUpdateIndex)
{
	//__COUT__ << std::endl;

	//validate lastUpdateIndex
	if(lastUpdateIndex > messages_.size() &&
			lastUpdateIndex != (unsigned int)-1)
	{
		__SS__ << "Invalid lastUpdateIndex: " << lastUpdateIndex <<
				" messagesArray size = " << messages_.size() << std::endl;
		throw std::runtime_error(ss.str());
	}

	//lockout the messages array for the remainder of the scope
	//this guarantees the reading thread can safely access the messages
	std::lock_guard<std::mutex> lock(messageMutex_);

	//newest available read pointer is defined as always one behind writePointer_
	refreshReadPointer_ = (writePointer_ + messages_.size() - 1) % messages_.size();

	sprintf(refreshTempStr_,"%lu",messages_[refreshReadPointer_].getCount());
	xmlOut->addTextElementToData("last_update_count",refreshTempStr_);
	sprintf(refreshTempStr_,"%u",refreshReadPointer_);
	xmlOut->addTextElementToData("last_update_index",refreshTempStr_);

	if(!messages_[refreshReadPointer_].getTime()) //if no data, then no data
		return;

	//else, send all messages since last_update_count, from last_update_index(refreshReadPointer_) on
	if(lastUpdateIndex != (unsigned int)-1 &&  //if not first time, and index-count are valid then start at next message
			messages_[lastUpdateIndex].getCount() == lastUpdateCount)
		refreshReadPointer_ = (lastUpdateIndex+1) % messages_.size();
	else if(messages_[writePointer_].getTime()) //check that writePointer_ message has been initialized, therefore has wrapped around at least once already
	{
		//This means we have had many messages and that some were missed since last update (give as many messages as we can!)
		xmlOut->addTextElementToData("message_overflow","1");
		__COUT__ << "Overflow was detected!" << std::endl;
		refreshReadPointer_ = (writePointer_+1) % messages_.size();
	}
	else  //user does not have valid index, and writePointer_ has not wrapped around, so give all new messages
		refreshReadPointer_ = 0;

//	__COUT__ << "refreshReadPointer_: " << refreshReadPointer_ << std::endl;
//	__COUT__ << "lastUpdateCount: " << lastUpdateCount << std::endl;
//	__COUT__ << "writePointer_: " << writePointer_ << std::endl;

	//return anything from refreshReadPointer_ to writePointer_
	//all should have a clock greater than lastUpdateClock

	refreshParent_ = xmlOut->addTextElementToData("messages","");

	bool requestOutOfSync = false;
	std::string requestOutOfSyncMsg;
	//output oldest to new (from refreshReadPointer_ to writePointer_-1, inclusive)
	for(/*refreshReadPointer_=<first index to read>*/;
			refreshReadPointer_ != writePointer_;
			refreshReadPointer_ = (refreshReadPointer_+1) % messages_.size())
	{
		if(messages_[refreshReadPointer_].getCount() < lastUpdateCount)
		{
			if(!requestOutOfSync) //record out of sync message once only
			{
				requestOutOfSync = true;
				__SS__ << "Request is out of sync! Message count should be more recent than update clock! " <<
						messages_[refreshReadPointer_].getCount() << " < " <<
						lastUpdateCount << std::endl;
				requestOutOfSyncMsg = ss.str();
			}
			//assume these messages are new (due to a system restart)
			//continue;
		}

		//for all fields, give value
		for(refreshIndex_=0; refreshIndex_ < messages_[refreshReadPointer_].fields.size();++refreshIndex_)
			xmlOut->addTextElementToParent("message_" +
					messages_[refreshReadPointer_].fields[refreshIndex_].fieldName,
					messages_[refreshReadPointer_].getField(refreshIndex_), refreshParent_);

		//give timestamp also
		sprintf(refreshTempStr_,"%lu",messages_[refreshReadPointer_].getTime());
		xmlOut->addTextElementToParent("message_Time",
				refreshTempStr_, refreshParent_);
		//give clock also
		sprintf(refreshTempStr_,"%lu",messages_[refreshReadPointer_].getCount());
		xmlOut->addTextElementToParent("message_Count",
				refreshTempStr_, refreshParent_);
	}

	if(requestOutOfSync) //if request was out of sync, show message
		__COUT__ << requestOutOfSyncMsg;
}









