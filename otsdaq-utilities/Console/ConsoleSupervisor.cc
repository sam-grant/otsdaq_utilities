#include "otsdaq-utilities/Console/ConsoleSupervisor.h"
#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/Macros/CoutHeaderMacros.h"
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


#define USER_CONSOLE_COLOR_PREF_PATH	std::string(getenv("SERVICE_DATA")) + "/ConsolePreferences/"
#define USERS_PREFERENCES_FILETYPE 		"pref"

#undef 	__MF_SUBJECT__
#define __MF_SUBJECT__ "Console"


//========================================================================================================================
ConsoleSupervisor::ConsoleSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception):
xdaq::Application(s   ),
SOAPMessenger  (this),
theRemoteWebUsers_(this),
writePointer_(0),
messageCount_(0)
{
	INIT_MF("ConsoleSupervisor");

	//attempt to make directory structure (just in case)
	mkdir(((std::string)USER_CONSOLE_COLOR_PREF_PATH).c_str(), 0755);

	xgi::bind (this, &ConsoleSupervisor::Default,                	"Default" );
	xgi::bind (this, &ConsoleSupervisor::Console,              		"Console" );
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
	//called by constructor
	theSupervisorsConfiguration_.init(getApplicationContext());

	__MOUT__ << "ApplicationDescriptor LID=" << getApplicationDescriptor()->getLocalId() << std::endl;

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
	std::cout << __COUT_HDR_FL__ << std::endl;
	ReceiverSocket rsock("127.0.0.1",30001); //FIXME == Take IP/Port from Configuration
	rsock.initialize();

	std::string buffer;
	int i = 0;
	while(1)
	{
		//if receive succeeds display message

		//	int receive(std::string& buffer, unsigned int timeoutSeconds=1, unsigned int timeoutUSeconds=0);
		if(rsock.receive(buffer,1,0,false) != -1) //set to rcv quiet mode
		{
			i = 10; //so things are good for all time.

			//lockout the messages array for the remainder of the scope
			//this guarantees the reading thread can safely access the messages
			std::lock_guard<std::mutex> lock(cs->messageMutex_);

			cs->messages_[cs->writePointer_++].set(buffer,cs->messageCount_++);
			if(cs->writePointer_ == cs->messages_.size()) //handle wrap-around
				cs->writePointer_ = 0;
		}
		else
		{
			if(i < 5)
				++i;
			sleep(1); //sleep one second, if timeout
		}

		if(i==5) break; //assume something wrong, and break loop
	}

}

//========================================================================================================================
void ConsoleSupervisor::Default(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	__MOUT__ << "ApplicationDescriptor LID=" << getApplicationDescriptor()->getLocalId() << std::endl;
	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/Console.html?urn=" <<
			getApplicationDescriptor()->getLocalId() << "'></frameset></html>";
}

//========================================================================================================================
//	Request
//		Handles Web Interface requests to Console supervisor.
//		Does not refresh cookie for automatic update checks.
void ConsoleSupervisor::Console(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	cgicc::Cgicc cgi(in);
	std::string Command;
	if((Command = CgiDataUtilities::postData(cgi,"RequestType")) == "")
		Command = cgi("RequestType"); //get command from form, if PreviewEntry

	//__MOUT__ << "Command " << Command << std::endl;

	//Commands:
		//GetConsoleMsgs
		//SaveColorChoice
		//LoadColorChoice

	HttpXmlDocument xmldoc;
	uint64_t activeSessionIndex;
	std::string user;

	//**** start LOGIN GATEWAY CODE ***//
	{
		bool automaticCommand = Command == "GetConsoleMsgs"; //automatic commands should not refresh cookie code.. only user initiated commands should!
		bool checkLock = true;
		bool getUser = (Command == "SaveColorChoice") || (Command == "LoadColorChoice");
		bool requireLock = false;

		if(!theRemoteWebUsers_.xmlLoginGateway(
				cgi,out,&xmldoc,theSupervisorsConfiguration_,
				0,//&userPermissions,  		//acquire user's access level (optionally null pointer)
				!automaticCommand,			//true/false refresh cookie code
				ADMIN_PERMISSIONS_THRESHOLD, //set access level requirement to pass gateway
				checkLock,					//true/false enable check that system is unlocked or this user has the lock
				requireLock,				//true/false requires this user has the lock to proceed
				0,//&userWithLock,			//acquire username with lock (optionally null pointer)
				getUser?&user:0				//acquire username of this user (optionally null pointer)
				,0//,&displayName			//acquire user's Display Name
				,&activeSessionIndex		//acquire user's session index associated with the cookieCode
				))
		{	//failure
			__MOUT__ << "Failed Login Gateway: " <<
					out->str() << std::endl; //print out return string on failure
			return;
		}
	}
	//**** end LOGIN GATEWAY CODE ***//


	//to report to logbook admin status use xmldoc.addTextElementToData(XML_ADMIN_STATUS,refreshTempStr_);

	if(Command == "GetConsoleMsgs")
	{
		//lindex of -1 means first time and user just gets update lcount and lindex
        std::string lastUpdateCountStr = CgiDataUtilities::postData(cgi,"lcount");
        std::string lastUpdateIndexStr = CgiDataUtilities::postData(cgi,"lindex");

        if(lastUpdateCountStr == "" || lastUpdateIndexStr == "")
        {
    		__MOUT_ERR__ << "Invalid Parameters! lastUpdateCount=" << lastUpdateCountStr <<
    				", lastUpdateIndex=" << lastUpdateIndexStr << std::endl;
    		xmldoc.addTextElementToData("Error","Error - Invalid parameters for GetConsoleMsgs.");
    		goto CLEANUP;
        }

        clock_t lastUpdateCount;
        sscanf(lastUpdateCountStr.c_str(),"%ld",&lastUpdateCount);

        unsigned int lastUpdateIndex;
        sscanf(lastUpdateIndexStr.c_str(),"%u",&lastUpdateIndex);
//		__MOUT__ << "lastUpdateCount=" << lastUpdateCount <<
//				", lastUpdateIndex=" << lastUpdateIndex << std::endl;

		insertMessageRefresh(&xmldoc,lastUpdateCount,lastUpdateIndex);
	}
	else if(Command == "SaveColorChoice")
	{
        std::string colorIndex = CgiDataUtilities::postData(cgi,"cindex");
		__MOUT__ << "Command " << Command << std::endl;
		__MOUT__ << "colorIndex: " << colorIndex << std::endl;

		if(user == "") //should never happen?
		{
			__MOUT_ERR__ << "Invalid user found! user=" << user << std::endl;
			xmldoc.addTextElementToData("Error","Error - Invalid user found.");
			goto CLEANUP;
		}

		std::string fn = (std::string)USER_CONSOLE_COLOR_PREF_PATH + user + "." + (std::string)USERS_PREFERENCES_FILETYPE;

		__MOUT__ << "Save preferences: " << fn << std::endl;
		FILE *fp = fopen(fn.c_str(),"w");
		if(!fp)
			throw std::runtime_error("Could not open file: " + fn);
		fprintf(fp,"color_index %s",colorIndex.c_str());
		fclose(fp);
	}
	else if(Command == "LoadColorChoice")
	{
		__MOUT__ << "Command " << Command << std::endl;

		unsigned int colorIndex;

		if(user == "") //should never happen?
		{
			__MOUT_ERR__ << "Invalid user found! user=" << user << std::endl;
			xmldoc.addTextElementToData("Error","Error - Invalid user found.");
			goto CLEANUP;
		}

		std::string fn = (std::string)USER_CONSOLE_COLOR_PREF_PATH + user + "." + (std::string)USERS_PREFERENCES_FILETYPE;

		__MOUT__ << "Load preferences: " << fn << std::endl;

		FILE *fp = fopen(fn.c_str(),"r");
		if(!fp)
			throw std::runtime_error("Could not open file: " + fn);
		fscanf(fp,"%*s %u",&colorIndex);
		fclose(fp);
		__MOUT__ << "colorIndex: " << colorIndex << std::endl;

		char tmpStr[20];
		sprintf(tmpStr,"%d",colorIndex);
		xmldoc.addTextElementToData("color_index",tmpStr);
	}




	CLEANUP:

	//return xml doc holding server response
	xmldoc.outputXmlDocument((std::ostringstream*)out, false, true); //allow whitespace
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
void ConsoleSupervisor::insertMessageRefresh(HttpXmlDocument *xmldoc,
		const time_t lastUpdateCount, const unsigned int lastUpdateIndex)
{
	//__MOUT__ << std::endl;

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
	xmldoc->addTextElementToData("last_update_count",refreshTempStr_);
	sprintf(refreshTempStr_,"%u",refreshReadPointer_);
	xmldoc->addTextElementToData("last_update_index",refreshTempStr_);

	if(!messages_[refreshReadPointer_].getTime()) //if no data, then no data
		return;

	//else, send all messages since last_update_count, from last_update_index(refreshReadPointer_) on
	if(lastUpdateIndex != (unsigned int)-1 &&  //if not first time, and index-count are valid then start at next message
			messages_[lastUpdateIndex].getCount() == lastUpdateCount)
		refreshReadPointer_ = (lastUpdateIndex+1) % messages_.size();
	else if(messages_[writePointer_].getTime()) //check that writePointer_ message has been initialized, therefore has wrapped around at least once already
	{
		//This means we have had many messages and that some were missed since last update (give as many messages as we can!)
		xmldoc->addTextElementToData("message_overflow","1");
		__MOUT__ << "Overflow was detected!" << std::endl;
		refreshReadPointer_ = (writePointer_+1) % messages_.size();
	}
	else  //user does not have valid index, and writePointer_ has not wrapped around, so give all new messages
		refreshReadPointer_ = 0;

//	__MOUT__ << "refreshReadPointer_: " << refreshReadPointer_ << std::endl;
//	__MOUT__ << "lastUpdateCount: " << lastUpdateCount << std::endl;
//	__MOUT__ << "writePointer_: " << writePointer_ << std::endl;

	//return anything from refreshReadPointer_ to writePointer_
	//all should have a clock greater than lastUpdateClock

	refreshParent_ = xmldoc->addTextElementToData("messages","");

	//output oldest to new (from refreshReadPointer_ to writePointer_-1, inclusive)
	for(/*refreshReadPointer_=<first index to read>*/;
			refreshReadPointer_ != writePointer_;
			refreshReadPointer_ = (refreshReadPointer_+1) % messages_.size())
	{
		if(messages_[refreshReadPointer_].getCount() < lastUpdateCount)
		{
			__MOUT_ERR__ << "Request is out of sync! Message count should be more recent than update clock! " <<
					messages_[refreshReadPointer_].getCount() << " < " <<
					lastUpdateCount << std::endl;
			continue;
		}

		//for all fields, give value
		for(refreshIndex_=0; refreshIndex_ < messages_[refreshReadPointer_].fields.size();++refreshIndex_)
			xmldoc->addTextElementToParent("message_" +
					messages_[refreshReadPointer_].fields[refreshIndex_].fieldName,
					messages_[refreshReadPointer_].getField(refreshIndex_), refreshParent_);

		//give timestamp also
		sprintf(refreshTempStr_,"%lu",messages_[refreshReadPointer_].getTime());
		xmldoc->addTextElementToParent("message_Time",
				refreshTempStr_, refreshParent_);
		//give clock also
		sprintf(refreshTempStr_,"%lu",messages_[refreshReadPointer_].getCount());
		xmldoc->addTextElementToParent("message_Count",
				refreshTempStr_, refreshParent_);
	}
}








