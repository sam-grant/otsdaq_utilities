#include "otsdaq-utilities/SlowControlsDashboard/SlowControlsDashboardSupervisor.h"
#include <dirent.h>    //for DIR
#include <sys/stat.h>  //for stat() quickly checking if file exists
#include <thread>      //for std::thread

#include "otsdaq/PluginMakers/MakeSlowControls.h"
#include "otsdaq/SlowControlsCore/SlowControlsVInterface.h"

#include <boost/regex.hpp>

using namespace ots;

#define CONTROLS_SUPERVISOR_DATA_PATH \
	std::string(__ENV__("SERVICE_DATA_PATH")) + "/ControlsDashboardData/"
#define PAGES_DIRECTORY CONTROLS_SUPERVISOR_DATA_PATH + "pages/"

XDAQ_INSTANTIATOR_IMPL(SlowControlsDashboardSupervisor)

//==============================================================================
SlowControlsDashboardSupervisor::SlowControlsDashboardSupervisor(
    xdaq::ApplicationStub* stub)
    : CoreSupervisorBase(stub)
{
	__SUP_COUT__ << "Constructor." << __E__;

	INIT_MF("." /*directory used is USER_DATA/LOG/.*/);

	// make controls dashboard supervisor directories in case they don't exist
	mkdir(((std::string)(CONTROLS_SUPERVISOR_DATA_PATH)).c_str(), 0755);
	mkdir(((std::string)(PAGES_DIRECTORY)).c_str(), 0755);

	interface_ = NULL;

	init();

	// interface_ = NULL;

	__SUP_COUT__ << "Constructed." << __E__;
}  // end constructor

//==============================================================================
SlowControlsDashboardSupervisor::~SlowControlsDashboardSupervisor(void)
{
	__SUP_COUT__ << "Destructor." << __E__;
	destroy();
	__SUP_COUT__ << "Destructed." << __E__;
}  // end destructor()

//==============================================================================
void SlowControlsDashboardSupervisor::destroy(void)
{
	// called by destructor
	delete interface_;
}  // end destroy()

//==============================================================================
void SlowControlsDashboardSupervisor::init(void)
// called by constructor
{
	UID_ = 0;

	__SUP_COUT__ << std::endl;
	ConfigurationTree node = CorePropertySupervisorBase::getSupervisorTableNode();
	std::string       pluginType;

	try
	{
		pluginType =
		    node.getNode("SlowControlsInterfacePluginType").getValue<std::string>();
	}
	catch(...)
	{
		// failed to get plugin type through supervisor table link, so try app property
		__COUT__ << "Pluging type was not definded through supervisor table, trying "
		            "supervisor property..."
		         << __E__;
		pluginType = CorePropertySupervisorBase::getSupervisorProperty(
		    "ControlsInterfacePluginType");
	}

	__COUTV__(pluginType);

	interface_ =
	    makeSlowControls(pluginType,
	                     CorePropertySupervisorBase::getSupervisorUID(),
	                     CorePropertySupervisorBase::getContextTreeNode(),
	                     CorePropertySupervisorBase::getSupervisorConfigurationPath());
	__COUT__ << std::endl;

	//
	// interface_->initialize();
	std::thread(
	    [](SlowControlsDashboardSupervisor* cs) {

		    // lockout the messages array for the remainder of the scope
		    // this guarantees the reading thread can safely access the messages
		    std::lock_guard<std::mutex> lock(cs->pluginBusyMutex_);

		    cs->interface_->initialize();
	    },
	    this)
	    .detach();  // thread completes after creating, subscribing, and getting
	//	                // parameters for all channels

	//
	// checkSubscription
	std::thread(
	    [](SlowControlsDashboardSupervisor* cs) {

		    // lockout the messages array for the remainder of the scope
		    // this guarantees the reading thread can safely access the messages
		    // std::lock_guard<std::mutex> lock(cs->pluginBusyMutex_);
		    // cs->checkSubscriptions(cs);
	    },
	    this)
	    .detach();  // thread check clients subscription for all channels

	__SUP_COUT__ << "Finished init() w/ interface: " << pluginType << std::endl;

	// add interface plugin to state machine list
	CoreSupervisorBase::theStateMachineImplementation_.push_back(interface_);

}  // end init()

//==============================================================================
// Manage channel subscriptions to Interface
void SlowControlsDashboardSupervisor::checkSubscriptions(
    SlowControlsDashboardSupervisor* cs)
{
	__COUT__ << "checkSubscriptions() initializing..." << std::endl;
	std::vector<std::string> channelList;
	std::vector<int>         channelRefreshRates;
	while(true)
	{
		channelList         = {"FIRST VALUE"};
		channelRefreshRates = {};
		std::map<int, std::set<std::string>>::iterator mapReference =
		    cs->channelDependencyLookupMap_.begin();
		while(mapReference !=
		      cs->channelDependencyLookupMap_
		          .end())  // We have here current list of Channel Dependencies
		{
			for(auto channel : mapReference->second)
			{
				int refreshRate = 1;  // seconds
				if(channel.find(":") != channel.size() - 1)
				{
					try
					{
						refreshRate = stoi(channel.substr(channel.find(":") + 1)) / 1000.;
						channelRefreshRates.push_back(refreshRate);
					}
					catch(const std::exception& e)
					{
						continue;
					}
				}

				channel = channel.substr(0, channel.find(":"));
				__COUT__ << "THREAD actual time: " << std::time(NULL)
				         << "; uidPollTimeMap + 10 * refreshTime: "
				         << cs->uidPollTimeMap_.at(mapReference->first) + 10 * refreshRate
				         << " seconds" << std::endl;
				if(std::time(NULL) >
				   cs->uidPollTimeMap_.at(mapReference->first) + 10 * refreshRate)
				{
					try
					{
						cs->channelDependencyLookupMap_.erase(mapReference->first);
						continue;
					}
					catch(const std::exception& e)
					{
						continue;
					}
				}

				std::vector<std::string>::iterator it =
				    find(channelList.begin(), channelList.end(), channel);
				if(it == channelList.end())
				{
					// cs->interface_->unsubscribe(channel);
					// cs->interface_->subscribe(channel);
					channelList.push_back(channel);
					__COUT__ << "Channel: " << channel << " refreshRate:  " << refreshRate
					         << " seconds" << std::endl;
					__COUT__ << "channelDependencyLookupMap_.size(): "
					         << cs->channelDependencyLookupMap_.size()
					         << " UID: " << mapReference->first
					         << " mapReference->second.size(): "
					         << mapReference->second.size() << std::endl;
				}

				sleep(1);
			}
			mapReference++;
		}
		int minTime = 30;  // seconds
		if(channelRefreshRates.size() > 0)
			minTime =
			    *min_element(channelRefreshRates.begin(), channelRefreshRates.end());
		sleep(minTime);
		__COUT__ << "Loop over channels subscribing - waiting time: " << minTime
		         << " seconds" << std::endl;
	}
}

//==============================================================================
// setSupervisorPropertyDefaults
//		override to set defaults for supervisor property values (before user settings
// override)
void SlowControlsDashboardSupervisor::setSupervisorPropertyDefaults()
{
	CorePropertySupervisorBase::setSupervisorProperty(
	    CorePropertySupervisorBase::SUPERVISOR_PROPERTIES.CheckUserLockRequestTypes,
		"*");
}

//==============================================================================
// forceSupervisorPropertyValues
//		override to force supervisor property values (and ignore user settings)
void SlowControlsDashboardSupervisor::forceSupervisorPropertyValues()
{
	CorePropertySupervisorBase::setSupervisorProperty(
	    CorePropertySupervisorBase::SUPERVISOR_PROPERTIES.AutomatedRequestTypes,
		"poll");
}

//==============================================================================
void SlowControlsDashboardSupervisor::request(const std::string& requestType,
                                              cgicc::Cgicc&      cgiIn,
                                              HttpXmlDocument&   xmlOut,
                                              const WebUsers::RequestUserInfo& userInfo)
{
	try
	{
		if(requestType != "getPages" && !pluginBusyMutex_.try_lock())
		{
			__SUP_SS__ << "Controls plugin is still initializing. Please try again in a "
			              "few minutes!"
			           << __E__;
			__SUP_SS_THROW__;
		}

		__SUP_COUT__ << "User name is " << userInfo.username_ << "." << __E__;
		__SUP_COUT__ << "User permission level for request '" << requestType << "' is "
		             << unsigned(userInfo.permissionLevel_)
		             << "(isAdmin=" << (userInfo.isAdmin() ? "Yes" : "No") << ")."
		             << __E__;

		// handle request per requestType
		handleRequest(requestType, xmlOut, cgiIn, userInfo);
	}
	catch(const std::runtime_error& e)
	{
		__SUP_SS__ << "Error occurred handling request '" << requestType
		           << "': " << e.what() << __E__;
		__SUP_COUT__ << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
	}
	catch(...)
	{
		__SS__ << "Unknown error occurred handling request '" << requestType << "!'"
		       << __E__;
		__SUP_COUT__ << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
	}

	pluginBusyMutex_.unlock();
}  // end request()

//==============================================================================
void SlowControlsDashboardSupervisor::handleRequest(
    const std::string                Command,
    HttpXmlDocument&                 xmlOut,
    cgicc::Cgicc&                    cgiIn,
    const WebUsers::RequestUserInfo& userInfo)
{
	// return xml doc holding server response
	__SUP_COUT__ << std::endl;

	if(Command == "poll")
	{
		std::string uid = CgiDataUtilities::getOrPostData(cgiIn, "uid");
		Poll(cgiIn, xmlOut, uid);
	}
	else if(Command == "userActivityHeartbeat")
	{
		//web client code should call this while user is building
		//	in Edit Mode.
		//Do nothing, just keep login alive
	}
	else if(Command == "generateUID")
	{
		std::string channelList = CgiDataUtilities::getOrPostData(cgiIn, "PVList");
		GenerateUID(cgiIn, xmlOut, channelList);
	}
	else if(Command == "isUserAdmin")
	{
		std::string json = std::string("{ \"message\": \"");
		json += (userInfo.isAdmin() ? "Yes" : "No");
		json += "\"}";
		xmlOut.addTextElementToData("JSON", json.c_str());
	}
	else if(Command == "getUserPermissions")
	{
		GetUserPermissions(cgiIn, xmlOut, userInfo);
	}
	else if(Command == "getPVSettings")
	{
		__SUP_COUT__ << "Channel settings requested from server! " << std::endl;
		GetChannelSettings(cgiIn, xmlOut);
		xmlOut.addTextElementToData("id", CgiDataUtilities::getData(cgiIn, "id"));
	}
	else if(Command == "getPVArchiverData")
	{
		__SUP_COUT__ << "Archived Channel data requested from server! " << std::endl;
		GetChannelArchiverData(cgiIn, xmlOut);
		xmlOut.addTextElementToData("id", CgiDataUtilities::getData(cgiIn, "id"));
	}
	else if(Command == "getList")
	{
		__SUP_COUT__ << "Channel List requested from server! " << std::endl;
		GetList(cgiIn, xmlOut);
	}
	else if(Command == "getPages")
	{
		__SUP_COUT__ << "Requesting pages from server! " << std::endl;
		GetPages(cgiIn, xmlOut);
	}
	else if(Command == "loadPage")
	{
		std::string page = CgiDataUtilities::getData(cgiIn, "Page");
		__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << " " << page
		             << std::endl;

		loadPage(cgiIn, xmlOut, page, userInfo);
	}
	else if(Command == "loadPhoebusPage")
	{
		std::string page = CgiDataUtilities::getData(cgiIn, "Page");
		__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << " " << page
		             << std::endl;

		loadPhoebusPage(cgiIn, xmlOut, page, userInfo);
	}
	else if(Command == "createControlsPage")
	{
		SaveControlsPage(cgiIn, xmlOut, userInfo);
	}
	else if(Command == "createPhoebusControlsPage")
	{
		SavePhoebusControlsPage(cgiIn, xmlOut, userInfo);
	}
	//	else if(Command == "savePage")
	//	{
	//		std::string pageName = CgiDataUtilities::getData(cgiIn, "PageName");
	//		std::string page     = CgiDataUtilities::getOrPostData(cgiIn, "Page");
	//		SavePage(cgiIn, xmlOut, pageName, page);
	//	}

	__SUP_COUT__ << "" << std::endl;

	// xmlOut.outputXmlDocument((std::ostringstream*) out, true);
}  // end handleRequest

//==============================================================================
void SlowControlsDashboardSupervisor::Poll(cgicc::Cgicc&    cgiIn,
                                           HttpXmlDocument& xmlOut,
                                           std::string      UID)
{
	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << " "
	             << "Polling on UID:" << UID << std::endl;

	std::map<int, std::set<std::string>>::iterator mapReference;

	if(UID != "" && (mapReference = channelDependencyLookupMap_.find(std::stoi(UID))) !=
	                    channelDependencyLookupMap_
	                        .end())  // We have their current list of Channel Dependencies
	{
		uidPollTimeMap_.at(std::stoi(UID)) = std::time(NULL);
		std::string JSONMessage            = "{ ";

		for(auto channel : mapReference->second)
		{
			channel = channel.substr(0, channel.find(":"));

			__SUP_COUT__ << channel << std::endl;

			std::array<std::string, 4> channelInformation =
			    interface_->getCurrentValue(channel);

			__SUP_COUT__ << channel << ": " << channelInformation[1] << " : "
			             << channelInformation[3] << std::endl;

			if(channelInformation[0] != "NO_CHANGE")
			{
				//__SUP_COUT__ << "Reached" <<  std::endl;
				JSONMessage += "\"" + channel + "\": {";

				/*if(channelInfo->mostRecentBufferIndex - 1 < 0)
				    {
				    std::string value =
				   channelInfo->dataCache[channelInfo->dataCache.size()].second
				    std::string time =
				    }*/

				JSONMessage += "\"Timestamp\":\"" + channelInformation[0] + "\",";
				JSONMessage += "\"Value\":\"" + channelInformation[1] + "\",";
				JSONMessage += "\"Status\":\"" + channelInformation[2] + "\",";
				JSONMessage += "\"Severity\":\"" + channelInformation[3] + "\"},";
			}
			else
			{
				__SUP_COUT__ << "No change in value since last poll: " << channel
				             << std::endl;
			}

			// Handle Channels that disconnect, etc
			if(channelInformation[3] == "INVALID")
			{
				interface_->unsubscribe(channel);
				interface_->subscribe(channel);
			}

			//__SUP_COUT__ << channel  << ":" << (channelInfo?"Good":"Bad") << std::endl;
			//__SUP_COUT__ << channel  << ":" << channelInfo->mostRecentBufferIndex -1 <<
			// std::endl;
			//__SUP_COUT__ << channel << " : " <<
			// channelInfo->dataCache[(channelInfo->mostRecentBufferIndex -1)].second <<
			// std::endl;
		}

		JSONMessage = JSONMessage.substr(0, JSONMessage.length() - 1);
		JSONMessage += "}";
		__SUP_COUT__ << JSONMessage << std::endl;
		xmlOut.addTextElementToData("JSON", JSONMessage);  // add to response

		/*for (std::set<unsigned long>::iterator it = mapReference->second->begin();
		    it != mapReference->second.end(); ++it)
		    {
		        //__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << it <<
		    std::endl;
		    }*/
		/*std::string fakeData = 	std::string("{")
		                            + "\"Mu2e_CompStatus_daq01/system_temperature\":
		    \"40.5\","
		                            + "\"Mu2e_CompStatus_daq01/load_one\": \"378.2\","
		                            + "\"Mu2e_Weather_2/timestamp\":
		    \"11.14.45.2016.4.8\","
		                            + "\"Mu2e_CompStatus_daq01/system_temperature\":
		    \"43.4\","
		                            + "\"Mu2e_CompStatus_daq01/load_one\":\"80\","
		                            + "\"Mu2e_Weather_2/timestamp\":
		    \"11.14.45.2016.4.8\""
		                            + "}";
		    xmlOut.addTextElementToData("JSON", fakeData); //add to response
		 */
	}
	else  // UID is not in our map so force them to generate a new one
	{
		xmlOut.addTextElementToData("JSON",
		                            "{ \"message\": \"NOT_FOUND\"}");  // add to response
	}
}

//==============================================================================
void SlowControlsDashboardSupervisor::GetChannelSettings(cgicc::Cgicc&    cgiIn,
                                                         HttpXmlDocument& xmlOut)
{
	std::string channelList = CgiDataUtilities::postData(cgiIn, "PVList");

	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << " "
	             << "Getting settings for " << channelList << std::endl;

	std::string JSONMessage = "{ ";

	std::string channel;
	size_t      pos = 0;
	size_t      nextPos;
	size_t      lastIndex = channelList.find_last_of(",");
	std::cout << "**********************" << channelList.size() << std::endl;
	if(channelList.size() > 0)
	{
		while((nextPos = channelList.find(",", pos)) != std::string::npos)
		{
			channel = channelList.substr(pos, nextPos - pos);

			__SUP_COUT__ << channel << std::endl;

			std::array<std::string, 9> channelSettings = interface_->getSettings(channel);

			JSONMessage += "\"" + channel + "\": {";
			JSONMessage += "\"Units\": \"" + channelSettings[0] + "\",";
			JSONMessage += "\"Upper_Display_Limit\": \"" + channelSettings[1] + "\",";
			JSONMessage += "\"Lower_Display_Limit\": \"" + channelSettings[2] + "\",";
			JSONMessage += "\"Upper_Alarm_Limit\": \"" + channelSettings[3] + "\",";
			JSONMessage += "\"Upper_Warning_Limit\": \"" + channelSettings[4] + "\",";
			JSONMessage += "\"Lower_Warning_Limit\": \"" + channelSettings[5] + "\",";
			JSONMessage += "\"Lower_Alarm_Limit\": \"" + channelSettings[6] + "\",";
			JSONMessage += "\"Upper_Control_Limit\": \"" + channelSettings[7] + "\",";
			JSONMessage += "\"Lower_Control_Limit\": \"" + channelSettings[8] + "\"},";

			pos = nextPos + 1;
		}

		JSONMessage = JSONMessage.substr(0, JSONMessage.length() - 1);
		JSONMessage += "}";

		__SUP_COUT__ << JSONMessage << std::endl;
		xmlOut.addTextElementToData("JSON", JSONMessage);  // add to response
	}
	else
	{
		__SUP_COUT__ << "Did not find any settings because Channel list is length zero!"
		             << std::endl;

		xmlOut.addTextElementToData(
		    "JSON", "{ \"message\": \"GetPVSettings\"}");  // add to response
	}
}

//==============================================================================
void SlowControlsDashboardSupervisor::GetChannelArchiverData(cgicc::Cgicc&    cgiIn,
                                                             HttpXmlDocument& xmlOut)
{
	__SUP_COUT__ << "Requesting archived data!" << std::endl;

	std::string channelList = CgiDataUtilities::postData(cgiIn, "PVList");

	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << " "
	             << "Getting History for " << channelList << std::endl;

	std::string JSONMessage = "{ ";

	std::string channel;
	size_t      pos = 0;
	size_t      nextPos;
	size_t      lastIndex = channelList.find_last_of(",");
	std::cout << "channelList.size(): " << channelList.size() << std::endl;
	if(channelList.size() > 0)
	{
		while((nextPos = channelList.find(",", pos)) != std::string::npos)
		{
			channel = channelList.substr(pos, nextPos - pos);

			__SUP_COUT__ << channel << std::endl;

			std::vector<std::vector<std::string>> channelInformation =
			    interface_->getChannelHistory(channel);
			__SUP_COUT__ << channel << ": " << channelInformation[0][1] << " : "
			             << channelInformation[0][3] << std::endl;

			for(auto channelData : channelInformation)
			{
				std::string JSONMessage = "{ ";
				JSONMessage += "\"" + channel + "\": {";
				JSONMessage += "\"Timestamp\":\"" + channelData[0] + "\",";
				JSONMessage += "\"Value\":\"" + channelData[1] + "\",";
				JSONMessage += "\"Status\":\"" + channelData[2] + "\",";
				JSONMessage += "\"Severity\":\"" + channelData[3] + "\"},";

				JSONMessage = JSONMessage.substr(0, JSONMessage.length() - 1);
				JSONMessage += "}";
				__SUP_COUT__ << JSONMessage << std::endl;
				xmlOut.addTextElementToData("JSON", JSONMessage);  // add to response
			}

			pos = nextPos + 1;
		}

		JSONMessage = JSONMessage.substr(0, JSONMessage.length() - 1);
		JSONMessage += "}";

		__SUP_COUT__ << JSONMessage << std::endl;
		xmlOut.addTextElementToData("JSON", JSONMessage);  // add to response
	}
	else
	{
		__SUP_COUT__ << "Did not find any data because Channel list is length zero!"
		             << std::endl;

		xmlOut.addTextElementToData(
		    "JSON", "{ \"message\": \"GetChannelArchiverData\"}");  // add to response
	}

	/*
	        // FAKE NEWS RESPONSE
	        //		xmlOut.addTextElementToData(
	        //		    "JSON",
	        //					"{    \"ROOM:LI30:1:OUTSIDE_TEMP\": {
	        //					        \"nanos\": 823158037,
	        //					        \"secs\": 1540229999,
	        //					        \"severity\": 0,
	        //					        \"status\": 0,
	        //					        \"val\": 60.358551025390625
	        //					    },
	        //					    \"VPIO:IN20:111:VRAW\": {
	        //					        \"nanos\": 754373158,
	        //					        \"secs\": 1540229999,
	        //					        \"severity\": 0,
	        //					        \"status\": 0,
	        //					        \"val\": 5.529228687286377
	        //					    },
	        //					    \"YAGS:UND1:1005:Y_BM_CTR\": {
	        //					        \"nanos\": 164648807,
	        //					        \"secs\": 1537710595,
	        //					        \"severity\": 0,
	        //					        \"status\": 0,
	        //					        \"val\": 0.008066000000000002
	        //					    }
	        //					}");  // add to response

	        return;

	        // Where parameters
	        std::string data_retrieval_url     = "";
	        std::string data_retrieval_servlet = "";
	        std::string mime_type              = ".json";

	        // What/when paramaeters
	        std::string channel, from, to;

	        // Optional parameters
	        //	boolean fetchLatestMetadata, donotchunk;
	        //	std::string timeranges;
	        //	int ca_count, ca_how;

	        std::string req = data_retrieval_url + data_retrieval_servlet + mime_type +
	       "?" + "channel=" + channel + "&from=" + from + "&to=" + to;

	        std::string response = "";

	        return;
	 */

}  // end GetChannelArchiverData()

//==============================================================================
void SlowControlsDashboardSupervisor::GetUserPermissions(
    cgicc::Cgicc&                    cgiIn,
    HttpXmlDocument&                 xmlOut,
    const WebUsers::RequestUserInfo& userInfo)
{
	return;
}

//==============================================================================
void SlowControlsDashboardSupervisor::GenerateUID(cgicc::Cgicc&    cgiIn,
                                                  HttpXmlDocument& xmlOut,
                                                  std::string      channellist)
{
	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << " "
	             << "Generating UID" << std::endl;

	std::set<std::string> channelDependencies;
	std::string           uid;
	std::string           channel;
	size_t                pos = 0;
	size_t                nextPos;
	size_t                lastIndex = channellist.find_last_of(",");

	if(channellist.size() > 0)
	{
		// channellist.substr(2);
		__SUP_COUT__ << channellist << std::endl;

		while((nextPos = channellist.find(",", pos)) != std::string::npos)
		{
			channel = channellist.substr(pos, nextPos - pos);
			//__SUP_COUT__ << UID_ << ":" << pos << "-" << nextPos << " ->" << channel <<
			// std::endl;
			channelDependencies.insert(channel);
			pos = nextPos + 1;
		}

		channelDependencyLookupMap_.insert(
		    std::pair<int, std::set<std::string>>(++UID_, channelDependencies));

		uid = (std::string("{ \"message\": \"") + std::to_string(UID_) + "\"}");

		uidPollTimeMap_.insert(std::pair<int, long int>(UID_, std::time(NULL)));
	}
	else
	{
		__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId()
		             << " ChannelList invalid: " << channellist << std::endl;
		uid = "{ \"message\": \"-1\"}";
	}

	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << " NEW UID: " << UID_
	             << std::endl;

	xmlOut.addTextElementToData("JSON", uid);  // add to response
}  // end GenerateUID()

//==============================================================================
void SlowControlsDashboardSupervisor::GetList(cgicc::Cgicc&    cgiIn,
                                              HttpXmlDocument& xmlOut)
{
	if(interface_ != NULL)
	{
		__SUP_COUT__ << "Interface is defined! Attempting to get list!" << std::endl;
		//	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() <<
		// std::endl;
		std::cout << " " << interface_->getList("JSON") << std::endl;

		xmlOut.addTextElementToData("JSON",
		                            interface_->getList("JSON"));  // add to response
	}
	else
	{
		__SUP_COUT__ << "Interface undefined! Failed to get list!" << std::endl;
		xmlOut.addTextElementToData("JSON", "[\"None\"]");
	}
}  // end GetList()

//==============================================================================
void SlowControlsDashboardSupervisor::GetPages(cgicc::Cgicc&    cgiIn,
                                               HttpXmlDocument& xmlOut)
{
	/*DIR * dir;
	    struct dirent * ent;
	    std::string pathToPages = CONTROLS_SUPERVISOR_DATA_PATH;

	    std::vector<std::string> pages;

	    __SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << "Path to pages:
	    " << pathToPages << std::endl; if((dir = opendir (pathToPages.c_str())) != NULL)
	    {
	        while((ent = readdir(dir)) != NULL)
	        {
	            pages.push_back(ent->d_name);
	            __SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << "
	    GetPages"
	    << ent->d_name << std::endl;
	        }
	        closedir(dir);
	    }
	    else
	    {
	        __SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << "Could not
	    open directory: " << pathToPages << std::endl; return;
	    }
	 */
	std::vector<std::string> pages;

	listFiles("", true, &pages);

	std::string returnJSON = "[";
	for(auto it = pages.begin(); it != pages.end(); it++)
	{
		if(*it != "." && *it != "..")
			returnJSON += "\"" + *it + "\", ";
	}
	if(returnJSON.size() > 2 && returnJSON.compare("[") != 0)
	{
		__SUP_COUT__ << "Found pages on server!" << std::endl;
		returnJSON.resize(returnJSON.size() - 2);
		returnJSON += "]";
	}
	else
	{
		// No pages on the server
		__SUP_COUT__ << "No pages found on server!" << std::endl;
		returnJSON = "[\"None\"]";
	}
	std::cout << returnJSON << std::endl;

	xmlOut.addTextElementToData("JSON", returnJSON);  // add to response
}  // end GetPages()

//==============================================================================
void SlowControlsDashboardSupervisor::loadPage(cgicc::Cgicc&                    cgiIn,
                                               HttpXmlDocument&                 xmlOut,
                                               std::string                      page,
                                               const WebUsers::RequestUserInfo& userInfo)
{
	page = StringMacros::decodeURIComponent(page);

	// FIXME Filter out malicious attacks i.e. ../../../../../ stuff
	struct stat buffer;
	if(page.find("..") != std::string::npos)
	{
		__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId()
		             << "Error! Request using '..': " << page << std::endl;
	}
	else if(page.find("~") != std::string::npos)
	{
		__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId()
		             << "Error! Request using '~': " << page << std::endl;
	}
	else if(!(stat(page.c_str(), &buffer) == 0))
	{
		__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId()
		             << "Error! File not found: " << page << std::endl;
	}
	// Remove double / in path

	__SUP_COUT__ << page << std::endl;

	if(page.at(0) == '/')
	{
		__SUP_COUT__ << "First character is '/'" << std::endl;
		page.erase(page.begin(), page.begin() + 1);
		__SUP_COUT__ << page << std::endl;
	}

	std::string file = CONTROLS_SUPERVISOR_DATA_PATH;
	file += page;
	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId()
	             << "Trying to load page: " << page << std::endl;
	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId()
	             << "Trying to load page: " << file << std::endl;
	// read file
	// for each line in file

	std::ifstream infile(file);
	if(infile.fail())
	{
		std::cout << "Failed reading file: " << file << std::endl;

		xmlOut.addTextElementToData("Time", "[\"Not Found\"]");   // add to response
		xmlOut.addTextElementToData("Notes", "[\"Not Found\"]");  // add to response
		xmlOut.addTextElementToData(
		    "Page", StringMacros::encodeURIComponent(page));  // add to response
		return;
	}
	std::cout << "Reading file" << std::endl;

	std::string time         = "";
	std::string notes        = "";
	std::string controlsPage = "";

	for(std::string line; getline(infile, line);)
	{
		std::cout << line << std::endl;
		if(!line.substr(0, 5).compare("Time:"))
		{
			time = line.substr(6);
		}
		else if(!line.substr(0, 6).compare("Notes:"))
		{
			notes = line.substr(7);
		}
		else if(!line.substr(0, 5).compare("Page:"))
		{
			controlsPage = line.substr(6);
		}
	}
	std::cout << "Finished reading file" << std::endl;
	__SUP_COUTV__(time);
	__SUP_COUTV__(notes);
	__SUP_COUTV__(controlsPage);

	xmlOut.addTextElementToData("Time", time);          // add to response
	xmlOut.addTextElementToData("Notes", notes);        // add to response
	xmlOut.addTextElementToData("Page", controlsPage);  // add to response
}  // end loadPage()

void SlowControlsDashboardSupervisor::loadPhoebusPage(
    cgicc::Cgicc&                    cgiIn,
    HttpXmlDocument&                 xmlOut,
    std::string                      page,
    const WebUsers::RequestUserInfo& userInfo)
{
	page = StringMacros::decodeURIComponent(page);

	// FIXME Filter out malicious attacks i.e. ../../../../../ stuff
	struct stat buffer;
	if(page.find("..") != std::string::npos)
	{
		__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId()
		             << "Error! Request using '..': " << page << std::endl;
	}
	else if(page.find("~") != std::string::npos)
	{
		__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId()
		             << "Error! Request using '~': " << page << std::endl;
	}
	else if(!(stat(page.c_str(), &buffer) == 0))
	{
		__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId()
		             << "Error! File not found: " << page << std::endl;
	}
	// Remove double / in path

	__SUP_COUT__ << page << std::endl;

	if(page.at(0) == '/')
	{
		__SUP_COUT__ << "First character is '/'" << std::endl;
		page.erase(page.begin(), page.begin() + 1);
		__SUP_COUT__ << page << std::endl;
	}

	std::string file = CONTROLS_SUPERVISOR_DATA_PATH;
	file += page;
	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId()
	             << "Trying to load page: " << page << std::endl;
	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId()
	             << "Trying to load page: " << file << std::endl;

	// read file
	std::cout << "Reading file" << std::endl;
	std::ifstream infile(file);
	if(infile.fail())
	{
		std::cout << "Failed reading file: " << file << std::endl;
		xmlOut.addTextElementToData(
		    "Page", StringMacros::encodeURIComponent(page));  // add to response
		return;
	}

	std::string xml;
	for(std::string line; getline(infile, line);)
	{
		xml += line + "\n";
	}
	__SUP_COUT__ << xml << std::endl;
	xmlOut.addTextElementToData("PHOEBUS", xml);

}  // end loadPhoebusPage()

//==============================================================================
void SlowControlsDashboardSupervisor::SaveControlsPage(
    cgicc::Cgicc&                    cgiIn,
    HttpXmlDocument&                 xmlOut,
    const WebUsers::RequestUserInfo& userInfo)
{
	__SUP_COUT__ << "ControlsDashboard wants to create a Controls Page!" << __E__;

	std::string controlsPageName = CgiDataUtilities::postData(cgiIn, "Name");
	std::string pageString       = CgiDataUtilities::postData(cgiIn, "Page");
	std::string Time             = CgiDataUtilities::postData(cgiIn, "Time");
	std::string Notes =
	    StringMacros::decodeURIComponent(CgiDataUtilities::postData(cgiIn, "Notes"));
	std::string isControlsPagePublic = CgiDataUtilities::postData(cgiIn, "isPublic");

	__SUP_COUTV__(controlsPageName);
	__SUP_COUTV__(pageString);
	__SUP_COUTV__(Notes);
	__SUP_COUTV__(Time);
	__SUP_COUTV__(isControlsPagePublic);

	if(controlsPageName == "")
		return;

	__SUP_COUTV__(CONTROLS_SUPERVISOR_DATA_PATH);

	std::string fullPath;
	if(isControlsPagePublic == "true")
		fullPath = (std::string)CONTROLS_SUPERVISOR_DATA_PATH + "public/";
	else
		fullPath = (std::string)CONTROLS_SUPERVISOR_DATA_PATH + "private/";

	__SUP_COUTV__(fullPath);

	std::string file = fullPath + controlsPageName;

	__SUP_COUTV__("Saving Controls Page to: " + file);

	std::string extension = file.substr(file.length() - 4, 4);
	if(extension != ".dat")
	{
		__SUP_COUT__ << "Extension : " << extension << std::endl;
		file += std::string(".dat");
	}
	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId()
	             << "Trying to save page: " << controlsPageName << std::endl;
	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId()
	             << "Trying to save page as: " << file << std::endl;
	// read file
	// for each line in file

	std::ofstream outputFile;
	outputFile.open(file);
	outputFile << "Time: " << Time << "\n";
	outputFile << "Notes: " << Notes << "\n";
	outputFile << "Page: " << pageString;
	outputFile.close();

	std::cout << "Finished writing file" << std::endl;

	return;
}

//==============================================================================
void SlowControlsDashboardSupervisor::SavePhoebusControlsPage(
    cgicc::Cgicc&                    cgiIn,
    HttpXmlDocument&                 xmlOut,
    const WebUsers::RequestUserInfo& userInfo)
{
	__SUP_COUT__ << "ControlsDashboard wants to create a Controls Page!" << __E__;

	std::string controlsPageName     = CgiDataUtilities::postData(cgiIn, "Name");
	std::string pageString           = CgiDataUtilities::postData(cgiIn, "Page");
	std::string isControlsPagePublic = CgiDataUtilities::postData(cgiIn, "isPublic");

	__SUP_COUTV__(controlsPageName);
	__SUP_COUTV__(pageString);
	__SUP_COUTV__(isControlsPagePublic);

	if(controlsPageName == "")
		return;

	__SUP_COUTV__(CONTROLS_SUPERVISOR_DATA_PATH);

	std::string fullPath;
	if(isControlsPagePublic == "true")
		fullPath = (std::string)CONTROLS_SUPERVISOR_DATA_PATH + "public/";
	else
		fullPath = (std::string)CONTROLS_SUPERVISOR_DATA_PATH + "private/";

	__SUP_COUTV__(fullPath);

	std::string file = fullPath + controlsPageName;

	__SUP_COUTV__("Saving Controls Page to: " + file);

	std::string extension = file.substr(file.length() - 4, 4);
	if(extension != ".bob")
	{
		__SUP_COUT__ << "Extension : " << extension << std::endl;
		file += std::string(".bob");
	}
	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId()
	             << "Trying to save page: " << controlsPageName << std::endl;
	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId()
	             << "Trying to save page as: " << file << std::endl;
	// read file
	// for each line in file

	std::ofstream outputFile;
	outputFile.open(file);
	outputFile << pageString << "\n";
	outputFile.close();

	std::cout << "Finished writing file" << std::endl;

	return;
}

//==============================================================================
void SlowControlsDashboardSupervisor::Subscribe(cgicc::Cgicc&    cgiIn,
                                                HttpXmlDocument& xmlOut)
{
}

//==============================================================================
void SlowControlsDashboardSupervisor::Unsubscribe(cgicc::Cgicc&    cgiIn,
                                                  HttpXmlDocument& xmlOut)
{
}

//==============================================================================
//==============================================================================
//================================================== UTILITIES
//===========================================================
//==============================================================================
bool SlowControlsDashboardSupervisor::isDir(std::string dir)
{
	struct stat fileInfo;
	stat(dir.c_str(), &fileInfo);
	if(S_ISDIR(fileInfo.st_mode))
	{
		return true;
	}
	else
	{
		return false;
	}
}

//==============================================================================
void SlowControlsDashboardSupervisor::listFiles(std::string               baseDir,
                                                bool                      recursive,
                                                std::vector<std::string>* pages)
{
	std::string base = CONTROLS_SUPERVISOR_DATA_PATH;
	base += baseDir;

	DIR*           dp;
	struct dirent* dirp;
	if((dp = opendir(base.c_str())) == NULL)
	{
		std::cout << "[ERROR: " << errno << " ] Couldn't open " << base << "."
		          << std::endl;
		return;
	}
	else
	{
		while((dirp = readdir(dp)) != NULL)
		{
			if(dirp->d_name != std::string(".") && dirp->d_name != std::string(".."))
			{
				if(isDir(base + dirp->d_name) == true && recursive == true)
				{
					// pages->push_back(baseDir + dirp->d_name);
					std::cout << "[DIR]\t" << baseDir << dirp->d_name << "/" << std::endl;
					listFiles(baseDir + dirp->d_name + "/", true, pages);
				}
				else
				{
					pages->push_back(baseDir + dirp->d_name);
					std::cout << "[FILE]\t" << baseDir << dirp->d_name << std::endl;
				}
			}
		}
		closedir(dp);
	}
}
