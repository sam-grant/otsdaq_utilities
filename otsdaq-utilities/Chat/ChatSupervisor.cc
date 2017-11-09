#include "otsdaq-utilities/Chat/ChatSupervisor.h"
#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/Macros/CoutHeaderMacros.h"
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"

#include <xdaq/NamespaceURI.h>

#include <iostream>

using namespace ots;

#undef 	__MF_SUBJECT__
#define __MF_SUBJECT__ "Chat"

XDAQ_INSTANTIATOR_IMPL(ChatSupervisor)

//========================================================================================================================
ChatSupervisor::ChatSupervisor(xdaq::ApplicationStub* stub)
throw (xdaq::exception::Exception)
: xdaq::Application (stub)
, SOAPMessenger     (this)
, theRemoteWebUsers_(this)
{
	INIT_MF("ChatSupervisor");
	xgi::bind (this, &ChatSupervisor::Default, "Default");
	xgi::bind (this, &ChatSupervisor::Chat,    "Chat");

	ChatLastUpdateIndex = 1; //skip 0

	init();
}

//========================================================================================================================
ChatSupervisor::~ChatSupervisor(void)
{
	destroy();
}
//========================================================================================================================
void ChatSupervisor::init(void)
{
	//called by constructor
	theSupervisorDescriptorInfo_.init(getApplicationContext());
}

//========================================================================================================================
void ChatSupervisor::destroy(void)
{
	//called by destructor

}

//========================================================================================================================
void ChatSupervisor::Default(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{

	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/Chat.html?urn=" << 
			this->getApplicationDescriptor()->getLocalId() <<"'></frameset></html>";
}

//========================================================================================================================
//	Chat
//		Handles Web Interface requests to chat supervisor.
//		Does not refresh cookie for automatic update checks.
void ChatSupervisor::Chat(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	cgicc::Cgicc cgi(in);

	std::string Command = CgiDataUtilities::getData(cgi,"RequestType");
	__COUT__ << "Command: " << Command << std::endl;

	//Commands
	//RefreshChat
	//RefreshUsers
	//SendChat

	HttpXmlDocument xmldoc;

	//**** start LOGIN GATEWAY CODE ***//
	{
		bool automaticCommand = Command == "RefreshLogbook"; //automatic commands should not refresh cookie code.. only user initiated commands should!
		bool checkLock = false;
		bool getUser = (Command == "CreateExperiment") || (Command == "RemoveExperiment") ||
				(Command == "PreviewEntry") || (Command == "AdminRemoveRestoreEntry");

		if(!theRemoteWebUsers_.xmlLoginGateway(
				cgi,
				out,
				&xmldoc,
				theSupervisorDescriptorInfo_
				,0//&userPermissions,  		//acquire user's access level (optionally null pointer)
				,!automaticCommand			//true/false refresh cookie code
				,1 //set access level requirement to pass gateway
				,checkLock					//true/false enable check that system is unlocked or this user has the lock
				,0//requireLock
				,0//&userWithLock,			//acquire username with lock (optionally null pointer)
				,0//(getUser?&user:0)		//acquire username of this user (optionally null pointer)
				,0//,&displayName			//acquire user's Display Name
				,0//&activeSessionIndex		//acquire user's session index associated with the cookieCode
		))
		{	//failure
			__COUT__  << "Failed Login Gateway: " <<
					out->str() << std::endl; //print out return string on failure
			return;
		}
	}
	//**** end LOGIN GATEWAY CODE ***//


	cleanupExpiredChats();

	if(Command == "RefreshChat")
	{
		std::string lastUpdateIndexString = CgiDataUtilities::postData(cgi,"lastUpdateIndex");
		std::string user = CgiDataUtilities::postData(cgi,"user");
		uint64_t lastUpdateIndex;
		sscanf(lastUpdateIndexString.c_str(),"%lu",&lastUpdateIndex);

		insertChatRefresh(&xmldoc,lastUpdateIndex,user);
	}
	else if(Command == "RefreshUsers")
	{
		insertActiveUsers(&xmldoc);
	}
	else if(Command == "SendChat")
	{
		std::string chat = CgiDataUtilities::postData(cgi,"chat");
		std::string user = CgiDataUtilities::postData(cgi,"user");

		escapeChat(chat);

		newChat(chat, user);
	}
	else if(Command == "PageUser")
	{
		std::string topage = CgiDataUtilities::postData(cgi,"topage");
		std::string user = CgiDataUtilities::postData(cgi,"user");

		__COUT__ << "topage = " << topage.substr(0,10) << "... from user = " << user.substr(0,10) << std::endl;

		theRemoteWebUsers_.sendSystemMessage(theSupervisorDescriptorInfo_.getSupervisorDescriptor(),
				topage, user + " is paging you to come chat.");
	}
	else
		__COUT__ << "Command request not recognized." << std::endl;

	//return xml doc holding server response
	xmldoc.outputXmlDocument((std::ostringstream*)out);
}

//========================================================================================================================
//ChatSupervisor::escapeChat()
//	replace html/xhtml reserved characters with equivalent.
//	reserved: ", ', &, <, >
void ChatSupervisor::escapeChat(std::string &chat)
{
	//	char reserved[] = {'"','\'','&','<','>'};
	//std::string replace[] = {"&#34;","&#39;","&#38;","&#60;","&#62;"};
	//	for(uint64_t i=0;i<chat.size();++i)
	//		for(uint64_t j=0;j<chat.size();++j)
	//		if(chat[i] ==
}

//========================================================================================================================
//ChatSupervisor::insertActiveUsers()
void ChatSupervisor::insertActiveUsers(HttpXmlDocument *xmldoc)
{
	xmldoc->addTextElementToData("active_users",
			theRemoteWebUsers_.getActiveUserList(theSupervisorDescriptorInfo_.getSupervisorDescriptor()));
}

//========================================================================================================================
//ChatSupervisor::insertChatRefresh()
//	check if user is new to list (may cause update)
//		each new user causes update to last index
//	if lastUpdateIndex is current, return nothing
//	else return full chat user list and new chats
//	(note: lastUpdateIndex==0 first time and returns only user list. no chats)
void ChatSupervisor::insertChatRefresh(HttpXmlDocument *xmldoc, uint64_t lastUpdateIndex, std::string user)
{
	newUser(user);

	if(!isLastUpdateIndexStale(lastUpdateIndex)) return; 	//	if lastUpdateIndex is current, return nothing

	//return new update index, full chat user list, and new chats!

	char tempStr[50];
	sprintf(tempStr,"%lu",ChatLastUpdateIndex);
	xmldoc->addTextElementToData("last_update_index",tempStr);

	//get all users
	xmldoc->addTextElementToData("chat_users","");
	for(uint64_t i=0;i<ChatUsers_.size();++i)
		xmldoc->addTextElementToParent("chat_user",ChatUsers_[i],"chat_users");

	if(!lastUpdateIndex) //lastUpdateIndex == 0, so just give the <user> entered chat message only
		lastUpdateIndex = ChatHistoryIndex_[ChatHistoryIndex_.size()-1]-1; //new user will then get future chats

	//get all accounts
	xmldoc->addTextElementToData("chat_history","");
	for(uint64_t i=0;i<ChatHistoryEntry_.size();++i) //output oldest to new
	{
		if(isChatOld(ChatHistoryIndex_[i],lastUpdateIndex)) continue;

		xmldoc->addTextElementToParent("chat_entry",ChatHistoryEntry_[i],"chat_history");
		xmldoc->addTextElementToParent("chat_author",ChatHistoryAuthor_[i],"chat_history");
		sprintf(tempStr,"%lu",ChatHistoryTime_[i]);
		xmldoc->addTextElementToParent("chat_time",tempStr,"chat_history");
	}
}

//========================================================================================================================
//ChatSupervisor::newUser()
//	create new user if needed, and increment update
void ChatSupervisor::newUser(std::string user)
{
	for(uint64_t i=0;i<ChatUsers_.size();++i)
		if(ChatUsers_[i] == user)
		{
			ChatUsersTime_[i] = time(0); //update time
			return; //do not add new if found
		}

	__COUT__ << "New user: " << user << std::endl;
	//add and increment
	ChatUsers_.push_back(user);
	ChatUsersTime_.push_back(time(0));
	newChat(user + " joined the chat.","ots");	//add status message to chat, increment update
}

//========================================================================================================================
//ChatSupervisor::newChat()
//	create new chat, and increment update
void ChatSupervisor::newChat(std::string chat, std::string user)
{
	ChatHistoryEntry_.push_back(chat);
	ChatHistoryAuthor_.push_back(user);
	ChatHistoryTime_.push_back(time(0));
	ChatHistoryIndex_.push_back(incrementAndGetLastUpdate());
}

//========================================================================================================================
//ChatSupervisor::isChatNew()
//	return true if chatIndex is older than lastUpdateIndex
bool ChatSupervisor::isChatOld(uint64_t chatIndex, uint64_t last)
{
	return (last - chatIndex < (uint64_t(1) << 62));
}

//========================================================================================================================
//ChatSupervisor::isLastUpdateIndexStale()
bool ChatSupervisor::isLastUpdateIndexStale(uint64_t last)
{
	return ChatLastUpdateIndex != last;
}

//========================================================================================================================
//ChatSupervisor::incrementAndGetLastUpdate()
uint64_t ChatSupervisor::incrementAndGetLastUpdate()
{
	if(!++ChatLastUpdateIndex) ++ChatLastUpdateIndex; //skip 0
	return ChatLastUpdateIndex;
}

//========================================================================================================================
//ChatSupervisor::cleanupExpiredChats()
//	remove expired entries from Chat history and user list
void ChatSupervisor::cleanupExpiredChats()
{
	for(uint64_t i=0;i<ChatHistoryEntry_.size();++i)
		if(i >= CHAT_HISTORY_MAX_ENTRIES ||
				ChatHistoryTime_[i] + CHAT_HISTORY_EXPIRATION_TIME < time(0)) //expired
		{
			removeChatHistoryEntry(i);
			--i; //rewind loop
		}
		else
			break; //chronological order, so first encountered that is still valid exit loop

	for(uint64_t i=0;i<ChatUsers_.size();++i)
		if(ChatUsersTime_[i] + CHAT_HISTORY_EXPIRATION_TIME < time(0)) //expired
		{
			removeChatUserEntry(i);
			--i; //rewind loop
		}
		else
			break; //chronological order, so first encountered that is still valid exit loop
}

//========================================================================================================================
//ChatSupervisor::removeChatHistoryEntry()
void ChatSupervisor::removeChatHistoryEntry(uint64_t i)
{
	ChatHistoryEntry_.erase (ChatHistoryEntry_.begin()+i);
	ChatHistoryTime_.erase (ChatHistoryTime_.begin()+i);
	ChatHistoryAuthor_.erase (ChatHistoryAuthor_.begin()+i);
	ChatHistoryIndex_.erase (ChatHistoryIndex_.begin()+i);
}

//========================================================================================================================
//ChatSupervisor::removeChatHistoryEntry()
void ChatSupervisor::removeChatUserEntry(uint64_t i)
{
	newChat(ChatUsers_[i] + " left the chat.","ots");	//add status message to chat, increment update
	ChatUsers_.erase (ChatUsers_.begin()+i);
	ChatUsersTime_.erase (ChatUsersTime_.begin()+i);
}








