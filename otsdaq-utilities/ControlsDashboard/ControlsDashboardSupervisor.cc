#include "otsdaq-utilities/ControlsDashboard/ControlsDashboardSupervisor.h"
#include <dirent.h>    //for DIR
#include <sys/stat.h>  //for stat() quickly checking if file exists
#include <thread>      //for std::thread

#include "otsdaq-core/PluginMakers/MakeSlowControls.h"
#include "otsdaq-core/SlowControlsCore/SlowControlsVInterface.h"

using namespace ots;

#define CONTROLS_SUPERVISOR_DATA_PATH \
	std::string(__ENV__("SERVICE_DATA_PATH")) + "/ControlsDashboardData/"
#define PAGES_DIRECTORY CONTROLS_SUPERVISOR_DATA_PATH + "pages/"

XDAQ_INSTANTIATOR_IMPL(ControlsDashboardSupervisor)

//========================================================================================================================
ControlsDashboardSupervisor::ControlsDashboardSupervisor(xdaq::ApplicationStub* stub)
    : CoreSupervisorBase(stub)
{
	__SUP_COUT__ << "Constructor." << __E__;

	INIT_MF("ControlsDashboardSupervisor");

	// make controls dashboard supervisor directories in case they don't exist
	mkdir(((std::string)(CONTROLS_SUPERVISOR_DATA_PATH)).c_str(), 0755);
	mkdir(((std::string)(PAGES_DIRECTORY)).c_str(), 0755);

	interface_ = NULL;

	init();

	// interface_ = NULL;

	__SUP_COUT__ << "Constructed." << __E__;
}  // end constructor

//========================================================================================================================
ControlsDashboardSupervisor::~ControlsDashboardSupervisor(void)
{
	__SUP_COUT__ << "Destructor." << __E__;
	destroy();
	__SUP_COUT__ << "Destructed." << __E__;
}  // end destructor()

//========================================================================================================================
void ControlsDashboardSupervisor::destroy(void)
{
	// called by destructor
	delete interface_;
}  // end destroy()

//========================================================================================================================
void ControlsDashboardSupervisor::init(void)
// called by constructor
{
	UID_ = 0;

	__SUP_COUT__ << std::endl;
	ConfigurationTree node = CorePropertySupervisorBase::getSupervisorTableNode();
	std::string       pluginType =
	    CorePropertySupervisorBase::getSupervisorProperty("ControlsInterfacePluginType");

	std::string supervisorConfigurationPath =
	    "/" + CorePropertySupervisorBase::getContextUID() + "/LinkToApplicationTable/" +
	    CorePropertySupervisorBase::getSupervisorUID();
	//
	interface_ = makeSlowControls(pluginType,
	                              CorePropertySupervisorBase::getSupervisorUID(),
	                              CorePropertySupervisorBase::getContextTreeNode(),
	                              supervisorConfigurationPath);
	__COUT__ << std::endl;
	//
	//
	// interface_->initialize();
	std::thread(
	    [](ControlsDashboardSupervisor* cs) {

		    // lockout the messages array for the remainder of the scope
		    // this guarantees the reading thread can safely access the messages
		    std::lock_guard<std::mutex> lock(cs->pluginBusyMutex_);

		    cs->interface_->initialize();

	    },
	    this)
	    .detach();  // thread completes after creating, subscribing, and getting
	                //	                // parameters for all pvs
	__SUP_COUT__ << "Finished init() w/ interface: " << pluginType << std::endl;

}  // end init()

//========================================================================================================================
// setSupervisorPropertyDefaults
//		override to set defaults for supervisor property values (before user settings
// override)
void ControlsDashboardSupervisor::setSupervisorPropertyDefaults()
{
	CorePropertySupervisorBase::setSupervisorProperty(
	    CorePropertySupervisorBase::SUPERVISOR_PROPERTIES.CheckUserLockRequestTypes, "*");
}

//========================================================================================================================
// forceSupervisorPropertyValues
//		override to force supervisor property values (and ignore user settings)
void ControlsDashboardSupervisor::forceSupervisorPropertyValues()
{
	CorePropertySupervisorBase::setSupervisorProperty(
	    CorePropertySupervisorBase::SUPERVISOR_PROPERTIES.AutomatedRequestTypes, "poll");
}

//========================================================================================================================
void ControlsDashboardSupervisor::request(const std::string&               requestType,
                                          cgicc::Cgicc&                    cgiIn,
                                          HttpXmlDocument&                 xmlOut,
                                          const WebUsers::RequestUserInfo& userInfo)
{
	//	__SUP_COUT__ << std::endl;
	//	cgicc::Cgicc cgi(in);
	//	__SUP_COUT__ << std::endl;
	//	std::string requestType = CgiDataUtilities::getData(cgi,"RequestType");
	//	__SUP_COUT__ << request << std::endl;
	//	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << " " <<
	// requestType << " : end"<< std::endl;

	//	if(requestType == "")
	//	{
	//		Default(in, out);
	//		return;
	//	}
	//
	//	HttpXmlDocument xmldoc;
	//	uint64_t activeSessionIndex;
	//	std::string user;
	//	uint8_t userPermissions;
	//
	//	//**** start LOGIN GATEWAY CODE ***//
	//	{
	//		bool automaticCommand = requestType == "poll"; //automatic commands should not
	// refresh cookie code.. only user initiated commands should! 		bool checkLock   =
	// true; 		bool getUser     = false; 		bool requireLock = false;
	//
	//		if(!theRemoteWebUsers_.xmlRequestToGateway(
	//				cgi,
	//				out,
	//				&xmldoc,
	//				allSupervisorInfo_,
	//				&userPermissions,  		//acquire user's access level (optionally null
	// pointer) 				!automaticCommand,			//true/false refresh cookie
	// code 				1, //set access level requirement to pass gateway
	//				checkLock,					//true/false enable check that system is unlocked
	//or  this user has the lock 				requireLock,				//true/false
	//requires this user has the lock to  proceed 				0,//&userWithLock,
	////acquire username with lock (optionally null  pointer)
	//				//(getUser?&user:0),				//acquire username of this user (optionally
	//null  pointer) 				&username,
	//				0,						//acquire user's Display Name
	//				&activeSessionIndex		//acquire user's session index associated with
	//the  cookieCode
	//		))
	//		{	//failure
	//			__SUP_COUT__ << "Failed Login Gateway: " <<
	//					out->str() << std::endl; //print out return string on failure
	//			return;
	//		}
	//	}
	//	//**** end LOGIN GATEWAY CODE ***//
	//
	//
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
		             << unsigned(userInfo.permissionLevel_) << "." << __E__;

		// handle request per requestType
		handleRequest(requestType, xmlOut, cgiIn, userInfo.username_);
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
//========================================================================================================================
void ControlsDashboardSupervisor::handleRequest(const std::string  Command,
                                                HttpXmlDocument&   xmlOut,
                                                cgicc::Cgicc&      cgiIn,
                                                const std::string& username)
{
	// return xml doc holding server response
	__SUP_COUT__ << std::endl;

	if(Command == "poll")
	{
		std::string uid = CgiDataUtilities::getOrPostData(cgiIn, "uid");
		Poll(cgiIn, xmlOut, uid);
	}
	else if(Command == "generateUID")
	{
		std::string pvList = CgiDataUtilities::getOrPostData(cgiIn, "PVList");
		GenerateUID(cgiIn, xmlOut, pvList);
	}
	else if(Command == "getPVSettings")
	{
		__SUP_COUT__ << "PV settings requested from server! " << std::endl;
		GetPVSettings(cgiIn, xmlOut);
		xmlOut.addTextElementToData("id", CgiDataUtilities::getData(cgiIn, "id"));
	}
	else if(Command == "getList")
	{
		__SUP_COUT__ << "PV List requested from server! " << std::endl;
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

		loadPage(cgiIn, xmlOut, page);
	}
	else if(Command == "createControlsPage")
	{
		SaveControlsPage(cgiIn, xmlOut);
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

//========================================================================================================================
void ControlsDashboardSupervisor::Poll(cgicc::Cgicc&    cgiIn,
                                       HttpXmlDocument& xmlOut,
                                       std::string      UID)
{
	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << " "
	             << "Polling on UID:" << UID << std::endl;

	std::map<int, std::set<std::string>>::iterator mapReference;

	if(UID != "" &&
	   (mapReference = pvDependencyLookupMap_.find(std::stoi(UID))) !=
	       pvDependencyLookupMap_.end())  // We have their current list of PV Dependencies
	{
		std::string JSONMessage = "{ ";

		for(auto pv : mapReference->second)
		{
			__SUP_COUT__ << pv << std::endl;
			// PVInfo * pvInfo = interface_->mapOfPVInfo_.find(pv)->second;
			//__SUP_COUT__ << pv  << ":" << (pvInfo?"Good":"Bad") << std::endl;
			// interface_->getCurrentPVValue(pv);
			std::array<std::string, 4> pvInformation = interface_->getCurrentValue(pv);
			__SUP_COUT__ << pv << ": " << pvInformation[1] << " : " << pvInformation[3]
			             << std::endl;

			if(pvInformation[0] != "NO_CHANGE")
			{
				//__SUP_COUT__ << "Reached" <<  std::endl;
				JSONMessage += "\"" + pv + "\": {";

				/*if(pvInfo->mostRecentBufferIndex - 1 < 0)
			{
				std::string value = pvInfo->dataCache[pvInfo->dataCache.size()].second
				std::string time =
			}*/

				JSONMessage += "\"Timestamp\":\"" + pvInformation[0] + "\",";
				JSONMessage += "\"Value\":\"" + pvInformation[1] + "\",";
				JSONMessage += "\"Status\":\"" + pvInformation[2] + "\",";
				JSONMessage += "\"Severity\":\"" + pvInformation[3] + "\"},";
			}
			else
			{
				__SUP_COUT__ << "No change in value since last poll: " << pv << std::endl;
			}

			// Handle Channels that disconnect, etc
			if(pvInformation[3] == "INVALID")
			{
				interface_->subscribe(pv);
			}

			//__SUP_COUT__ << pv  << ":" << (pvInfo?"Good":"Bad") << std::endl;
			//__SUP_COUT__ << pv  << ":" << pvInfo->mostRecentBufferIndex -1 << std::endl;
			//__SUP_COUT__ << pv << " : " <<
			// pvInfo->dataCache[(pvInfo->mostRecentBufferIndex -1)].second << std::endl;
		}

		JSONMessage = JSONMessage.substr(0, JSONMessage.length() - 1);
		JSONMessage += "}";
		__SUP_COUT__ << JSONMessage << std::endl;
		xmlOut.addTextElementToData("JSON", JSONMessage);  // add to response

		/*for (std::set<unsigned long>::iterator it = mapReference->second->begin(); it !=
		mapReference->second.end(); ++it)
		{
		    //__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << it <<
		std::endl;


		}*/
		/*std::string fakeData = 	std::string("{")
		                        + "\"Mu2e_CompStatus_daq01/system_temperature\":
		\"40.5\","
		                        + "\"Mu2e_CompStatus_daq01/load_one\": \"378.2\","
		                        + "\"Mu2e_Weather_2/timestamp\": \"11.14.45.2016.4.8\","
		                        + "\"Mu2e_CompStatus_daq01/system_temperature\":
		\"43.4\","
		                        + "\"Mu2e_CompStatus_daq01/load_one\":\"80\","
		                        + "\"Mu2e_Weather_2/timestamp\": \"11.14.45.2016.4.8\""
		                        + "}";
		xmlOut.addTextElementToData("JSON", fakeData); //add to response*/
	}
	else  // UID is not in our map so force them to generate a new one
	{
		xmlOut.addTextElementToData("JSON",
		                            "{ \"message\": \"NOT_FOUND\"}");  // add to response
	}
}
//========================================================================================================================
void ControlsDashboardSupervisor::GetPVSettings(cgicc::Cgicc&    cgiIn,
                                                HttpXmlDocument& xmlOut)
{
	
	std::string pvList = CgiDataUtilities::postData(cgiIn, "PVList");


	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << " "
	             << "Getting settings for " << pvList << std::endl;
	             


	std::string JSONMessage = "{ ";

	std::string pv;
	size_t      pos = 0;
	size_t      nextPos;
	size_t      lastIndex = pvList.find_last_of(",");
	std::cout << "**********************" << pvList.size() << std::endl;
	if(pvList.size() > 0)
	{
		while((nextPos = pvList.find(",", pos)) != std::string::npos)
		{
			pv = pvList.substr(pos, nextPos - pos);

			__SUP_COUT__ << pv << std::endl;

			std::array<std::string, 9> pvSettings = interface_->getSettings(pv);

			JSONMessage += "\"" + pv + "\": {";
			JSONMessage += "\"Units\": \"" 					+ pvSettings[0] + "\",";
			JSONMessage += "\"Upper_Display_Limit\": \"" 	+ pvSettings[1] + "\",";
			JSONMessage += "\"Lower_Display_Limit\": \"" 	+ pvSettings[2] + "\",";
			JSONMessage += "\"Upper_Alarm_Limit\": \"" 		+ pvSettings[3] + "\",";
			JSONMessage += "\"Upper_Warning_Limit\": \"" 	+ pvSettings[4] + "\",";
			JSONMessage += "\"Lower_Warning_Limit\": \"" 	+ pvSettings[5] + "\",";
			JSONMessage += "\"Lower_Alarm_Limit\": \"" 		+ pvSettings[6] + "\",";
			JSONMessage += "\"Upper_Control_Limit\": \"" 	+ pvSettings[7] + "\",";
			JSONMessage += "\"Lower_Control_Limit\": \"" 	+ pvSettings[8] + "\"},";

			pos = nextPos + 1;
		}

		JSONMessage = JSONMessage.substr(0, JSONMessage.length() - 1);
		JSONMessage += "}";

		__SUP_COUT__ << JSONMessage << std::endl;
		xmlOut.addTextElementToData("JSON", JSONMessage);  // add to response
	}
	else
	{
		__SUP_COUT__ << "Did not find any settings because PV list is length zero!" << std::endl;

		xmlOut.addTextElementToData(
		    "JSON", "{ \"message\": \"GetPVSettings\"}");  // add to response
	}
}
//========================================================================================================================
void ControlsDashboardSupervisor::GenerateUID(cgicc::Cgicc&    cgiIn,
                                              HttpXmlDocument& xmlOut,
                                              std::string      pvlist)
{
	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << " "
	             << "Generating UID" << std::endl;

	std::set<std::string> pvDependencies;
	std::string           uid;
	std::string           pv;
	size_t                pos = 0;
	size_t                nextPos;
	size_t                lastIndex = pvlist.find_last_of(",");

	if(pvlist.size() > 0)
	{
		// pvlist.substr(2);
		__SUP_COUT__ << pvlist << std::endl;

		while((nextPos = pvlist.find(",", pos)) != std::string::npos)
		{
			pv = pvlist.substr(pos, nextPos - pos);
			//__SUP_COUT__ << UID_ << ":" << pos << "-" << nextPos << " ->" << pv <<
			// std::endl;
			pvDependencies.insert(pv);
			pos = nextPos + 1;
		}

		pvDependencyLookupMap_.insert(
		    std::pair<int, std::set<std::string>>(++UID_, pvDependencies));

		uid = (std::string("{ \"message\": \"") + std::to_string(UID_) + "\"}");
	}
	else
	{
		__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId()
		             << " PVList invalid: " << pvlist << std::endl;
		uid = "{ \"message\": \"-1\"}";
	}

	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << " NEW UID: " << UID_
	             << std::endl;

	xmlOut.addTextElementToData("JSON", uid);  // add to response
}
//========================================================================================================================
void ControlsDashboardSupervisor::GetList(cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut)
{
	if(interface_ != NULL)
	{
		__SUP_COUT__ << "Interface is defined! Attempting to get list!" << std::endl;
		//	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << std::endl;
		std::cout << " " << interface_->getList("JSON") << std::endl;

		xmlOut.addTextElementToData("JSON",
		                            interface_->getList("JSON"));  // add to response
	}
	else
	{
		__SUP_COUT__ << "Interface undefined! Failed to get list!" << std::endl;
		xmlOut.addTextElementToData("JSON", "[\"None\"]");
	}
}
//========================================================================================================================
void ControlsDashboardSupervisor::GetPages(cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut)
{
	/*DIR * dir;
	struct dirent * ent;
	std::string pathToPages = CONTROLS_SUPERVISOR_DATA_PATH;

	std::vector<std::string> pages;

	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << "Path to pages: " <<
	pathToPages << std::endl; if((dir = opendir (pathToPages.c_str())) != NULL)
	{
	    while((ent = readdir(dir)) != NULL)
	    {
	        pages.push_back(ent->d_name);
	        __SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << " GetPages"
	<< ent->d_name << std::endl;
	    }
	    closedir(dir);
	}
	else
	{
	    __SUP_COUT__ << this->getApplicationDescriptor()->getLocalId() << "Could not open
	directory: " << pathToPages << std::endl; return;
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
}
//========================================================================================================================
void ControlsDashboardSupervisor::loadPage(cgicc::Cgicc&    cgiIn,
                                           HttpXmlDocument& xmlOut,
                                           std::string      page)
{
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

	std::string file = CONTROLS_SUPERVISOR_DATA_PATH;
	file += CgiDataUtilities::decodeURIComponent(page);
	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId()
	             << "Trying to load page: " << page << std::endl;
	__SUP_COUT__ << this->getApplicationDescriptor()->getLocalId()
	             << "Trying to load page: " << file << std::endl;
	// read file
	// for each line in file

	std::ifstream infile(file);
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
}
//========================================================================================================================
void ControlsDashboardSupervisor::SaveControlsPage(cgicc::Cgicc&    cgiIn,
                                                   HttpXmlDocument& xmlOut)
{
	__SUP_COUT__ << "ControlsDashboard wants to create a Controls Page!" << __E__;

	std::string controlsPageName = CgiDataUtilities::postData(cgiIn, "Name");
	std::string pageString       = CgiDataUtilities::postData(cgiIn, "Page");
	std::string Time             = CgiDataUtilities::postData(cgiIn, "Time");
	std::string Notes =
	    CgiDataUtilities::decodeURIComponent(CgiDataUtilities::postData(cgiIn, "Notes"));
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
		fullPath = (std::string)CONTROLS_SUPERVISOR_DATA_PATH + "publicPages/";
	else
		fullPath = (std::string)CONTROLS_SUPERVISOR_DATA_PATH + "privatePages/";

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
//========================================================================================================================
void ControlsDashboardSupervisor::Subscribe(cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut)
{
}
//========================================================================================================================
void ControlsDashboardSupervisor::Unsubscribe(cgicc::Cgicc&    cgiIn,
                                              HttpXmlDocument& xmlOut)
{
}
//========================================================================================================================
//========================================================================================================================
//================================================== UTILITIES
//===========================================================
//========================================================================================================================
bool ControlsDashboardSupervisor::isDir(std::string dir)
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
//========================================================================================================================
void ControlsDashboardSupervisor::listFiles(std::string               baseDir,
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
