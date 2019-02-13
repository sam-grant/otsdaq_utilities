#include "otsdaq-utilities/ControlsDashboard/ControlsDashboardSupervisor.h"

#include <dirent.h>    //for DIR
#include <sys/stat.h>  //for stat() quickly checking if file exists

//
//#include "otsdaq-core/MessageFacility/MessageFacility.h"
//#include "otsdaq-core/Macros/CoutMacros.h"
//
//#include <xdaq/NamespaceURI.h>
//#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
//#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"
//#include "otsdaq-core/WebUsersUtilities/WebUsers.h"
//#include "otsdaq-core/SOAPUtilities/SOAPParameters.h"
//#include "otsdaq-core/ConfigurationPluginDataFormats/XDAQContextConfiguration.h"
//
//#include "otsdaq-core/ConfigurationInterface/ConfigurationManager.h"
//
//
//#include <iostream>
//#include <fstream>
//#include <string>
//#include <thread>         // std::this_thread::sleep_for
//#include <chrono>         // std::chrono::seconds
//#include <errno.h>

//#include "EpicsInterface.h.bkup"
#include "otsdaq-core/ControlsCore/ControlsVInterface.h"
#include "otsdaq-core/PluginMakers/MakeControls.h"

using namespace ots;

#define PAGES_DIRECTORY std::string(getenv("SERVICE_DATA_PATH")) + "/ControlsDashboardData/pages/";

XDAQ_INSTANTIATOR_IMPL(ControlsDashboardSupervisor)

//========================================================================================================================
ControlsDashboardSupervisor::ControlsDashboardSupervisor(xdaq::ApplicationStub* stub) : CoreSupervisorBase(stub) {
  INIT_MF("ControlsDashboardSupervisor");

  init();
}

//========================================================================================================================
ControlsDashboardSupervisor::~ControlsDashboardSupervisor(void) { destroy(); }

//========================================================================================================================
// setSupervisorPropertyDefaults
//		override to set defaults for supervisor property values (before user settings override)
void ControlsDashboardSupervisor::setSupervisorPropertyDefaults() {
  CorePropertySupervisorBase::setSupervisorProperty(
      CorePropertySupervisorBase::SUPERVISOR_PROPERTIES.CheckUserLockRequestTypes, "*");
}

//========================================================================================================================
// forceSupervisorPropertyValues
//		override to force supervisor property values (and ignore user settings)
void ControlsDashboardSupervisor::forceSupervisorPropertyValues() {
  CorePropertySupervisorBase::setSupervisorProperty(
      CorePropertySupervisorBase::SUPERVISOR_PROPERTIES.AutomatedRequestTypes, "poll");
}

//========================================================================================================================
void ControlsDashboardSupervisor::request(const std::string& requestType, cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut,
                                          const WebUsers::RequestUserInfo& userInfo) {
  //	__COUT__ << std::endl;
  //	cgicc::Cgicc cgi(in);
  //	__COUT__ << std::endl;
  //	std::string requestType = CgiDataUtilities::getData(cgi,"RequestType");
  //	__COUT__ << request << std::endl;
  //	__COUT__ << this->getApplicationDescriptor()->getLocalId() << " " << requestType << " : end"<< std::endl;

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
  //		bool automaticCommand = requestType == "poll"; //automatic commands should not refresh cookie code.. only
  //user initiated commands should! 		bool checkLock   = true; 		bool getUser     = false;
  //		bool requireLock = false;
  //
  //		if(!theRemoteWebUsers_.xmlRequestToGateway(
  //				cgi,
  //				out,
  //				&xmldoc,
  //				allSupervisorInfo_,
  //				&userPermissions,  		//acquire user's access level (optionally null pointer)
  //				!automaticCommand,			//true/false refresh cookie code
  //				1, //set access level requirement to pass gateway
  //				checkLock,					//true/false enable check that system is unlocked or this user has the
  //lock 				requireLock,				//true/false requires this user has the
  //lock to proceed
  //				0,//&userWithLock,			//acquire username with lock (optionally null
  //pointer)
  //				//(getUser?&user:0),				//acquire username of this user (optionally null
  //pointer) 				&username,
  //				0,						//acquire user's Display Name
  //				&activeSessionIndex		//acquire user's session index associated with the
  //cookieCode
  //		))
  //		{	//failure
  //			__COUT__ << "Failed Login Gateway: " <<
  //					out->str() << std::endl; //print out return string on failure
  //			return;
  //		}
  //	}
  //	//**** end LOGIN GATEWAY CODE ***//
  //
  //

  // return xml doc holding server response
  __COUT__ << std::endl;

  if (requestType == "poll") {
    std::string uid = CgiDataUtilities::getOrPostData(cgiIn, "uid");
    Poll(cgiIn, xmlOut, uid);
  } else if (requestType == "generateUID") {
    std::string pvList = CgiDataUtilities::getOrPostData(cgiIn, "PVList");
    GenerateUID(cgiIn, xmlOut, pvList);
  } else if (requestType == "GetPVSettings") {
    std::string pvList = CgiDataUtilities::getOrPostData(cgiIn, "PVList");
    GetPVSettings(cgiIn, xmlOut, pvList);
    xmlOut.addTextElementToData("id", CgiDataUtilities::getData(cgiIn, "id"));
  } else if (requestType == "getList") {
    GetList(cgiIn, xmlOut);
  } else if (requestType == "getPages") {
    GetPages(cgiIn, xmlOut);
  } else if (requestType == "loadPage") {
    std::string page = CgiDataUtilities::getData(cgiIn, "Page");
    __COUT__ << this->getApplicationDescriptor()->getLocalId() << " " << page << std::endl;

    loadPage(cgiIn, xmlOut, page);
  }
  __COUT__ << std::endl;

  // xmlOut.outputXmlDocument((std::ostringstream*) out, true);
}
//========================================================================================================================
void ControlsDashboardSupervisor::init(void)
// called by constructor
{
  UID_ = 0;

  __COUT__ << std::endl;
  std::string t = "test";
  std::string nodeName = theConfigurationManager_->__GET_CONFIG__(XDAQContextConfiguration)->getConfigurationName();
  __COUT__ << nodeName << std::endl;
  ConfigurationTree node = theConfigurationManager_->getNode(nodeName);
  __COUT__ << node << std::endl;

  interface_ = makeControls("ControlsOtsInterface", t /*Key Value*/
                            ,
                            node, nodeName);
  __COUT__ << std::endl;
  // interface_->initialize();
  // std::thread([&](){interface_->initialize();}).detach(); //thread completes after creating, subscribing, and getting
  // parameters for all pvs
}
//========================================================================================================================
void ControlsDashboardSupervisor::destroy(void) {
  // called by destructor
  delete interface_;
}

//========================================================================================================================
void ControlsDashboardSupervisor::Poll(cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut, std::string UID) {
  __COUT__ << this->getApplicationDescriptor()->getLocalId() << " "
           << "Polling on UID:" << UID << std::endl;

  std::map<int, std::set<std::string>>::iterator mapReference;

  if (UID != "" && (mapReference = pvDependencyLookupMap_.find(std::stoi(UID))) !=
                       pvDependencyLookupMap_.end())  // We have their current list of PV Dependencies
  {
    std::string JSONMessage = "{ ";

    for (auto pv : mapReference->second) {
      __COUT__ << pv << std::endl;
      // PVInfo * pvInfo = interface_->mapOfPVInfo_.find(pv)->second;
      //__COUT__ << pv  << ":" << (pvInfo?"Good":"Bad") << std::endl;
      // interface_->getCurrentPVValue(pv);
      std::array<std::string, 4> pvInformation = interface_->getCurrentValue(pv);

      __COUT__ << pv << ": " << pvInformation[1] << " : " << pvInformation[3] << std::endl;

      if (pvInformation[0] != "NO_CHANGE") {
        //__COUT__ << "Reached" <<  std::endl;
        JSONMessage += "\"" + pv + "\": {";

        /*if(pvInfo->mostRecentBufferIndex - 1 < 0)
{
        std::string value = pvInfo->dataCache[pvInfo->dataCache.size()].second
        std::string time =
}*/

        JSONMessage += "\"Timestamp\" : \"" + pvInformation[0] + "\",";
        JSONMessage += "\"Value\"     : \"" + pvInformation[1] + "\",";
        JSONMessage += "\"Status\"    : \"" + pvInformation[2] + "\",";
        JSONMessage += "\"Severity\"  : \"" + pvInformation[3] + "\"},";

      } else {
        __COUT__ << "No change in value since last poll: " << pv << std::endl;
      }

      // Handle Channels that disconnect, etc
      if (pvInformation[3] == "INVALID") {
        interface_->subscribe(pv);
      }

      //__COUT__ << pv  << ":" << (pvInfo?"Good":"Bad") << std::endl;
      //__COUT__ << pv  << ":" << pvInfo->mostRecentBufferIndex -1 << std::endl;
      //__COUT__ << pv << " : " << pvInfo->dataCache[(pvInfo->mostRecentBufferIndex -1)].second << std::endl;
    }

    JSONMessage = JSONMessage.substr(0, JSONMessage.length() - 1);
    JSONMessage += "}";
    __COUT__ << JSONMessage << std::endl;
    xmlOut.addTextElementToData("JSON", JSONMessage);  // add to response

    /*for (std::set<unsigned long>::iterator it = mapReference->second->begin(); it != mapReference->second.end(); ++it)
    {
            //__COUT__ << this->getApplicationDescriptor()->getLocalId() << it << std::endl;


    }*/
    /*std::string fakeData = 	std::string("{")
                                                    + "\"Mu2e_CompStatus_daq01/system_temperature\": \"40.5\","
                                                    + "\"Mu2e_CompStatus_daq01/load_one\": \"378.2\","
                                                    + "\"Mu2e_Weather_2/timestamp\": \"11.14.45.2016.4.8\","
                                                    + "\"Mu2e_CompStatus_daq01/system_temperature\": \"43.4\","
                                                    + "\"Mu2e_CompStatus_daq01/load_one\":\"80\","
                                                    + "\"Mu2e_Weather_2/timestamp\": \"11.14.45.2016.4.8\""
                                                    + "}";
    xmlOut.addTextElementToData("JSON", fakeData); //add to response*/

  } else  // UID is not in our map so force them to generate a new one
  {
    xmlOut.addTextElementToData("JSON", "{ \"message\": \"NOT_FOUND\"}");  // add to response
  }
}
//========================================================================================================================
void ControlsDashboardSupervisor::GetPVSettings(cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut, std::string pvList) {
  __COUT__ << this->getApplicationDescriptor()->getLocalId() << " "
           << "Getting settings for " << pvList << std::endl;

  std::string JSONMessage = "{ ";

  std::string pv;
  size_t pos = 0;
  size_t nextPos;
  size_t lastIndex = pvList.find_last_of(",");
  std::cout << "**********************" << pvList.size() << std::endl;
  if (pvList.size() > 0) {
    while ((nextPos = pvList.find(",", pos)) != std::string::npos) {
      pv = pvList.substr(pos, nextPos - pos);

      __COUT__ << pv << std::endl;

      std::array<std::string, 9> pvSettings = interface_->getSettings(pv);

      JSONMessage += "\"" + pv + "\": {";
      JSONMessage += "\"Units              \": \"" + pvSettings[0] + "\",";
      JSONMessage += "\"Upper_Display_Limit\": \"" + pvSettings[1] + "\",";
      JSONMessage += "\"Lower_Display_Limit\": \"" + pvSettings[2] + "\",";
      JSONMessage += "\"Upper_Alarm_Limit  \": \"" + pvSettings[3] + "\",";
      JSONMessage += "\"Upper_Warning_Limit\": \"" + pvSettings[4] + "\",";
      JSONMessage += "\"Lower_Warning_Limit\": \"" + pvSettings[5] + "\",";
      JSONMessage += "\"Lower_Alarm_Limit  \": \"" + pvSettings[6] + "\",";
      JSONMessage += "\"Upper_Control_Limit\": \"" + pvSettings[7] + "\",";
      JSONMessage += "\"Lower_Control_Limit\": \"" + pvSettings[8] + "\"},";

      pos = nextPos + 1;
    }

    JSONMessage = JSONMessage.substr(0, JSONMessage.length() - 1);
    JSONMessage += "}";

    __COUT__ << JSONMessage << std::endl;
    xmlOut.addTextElementToData("JSON", JSONMessage);  // add to response

  } else {
    xmlOut.addTextElementToData("JSON", "{ \"message\": \"GetPVSettings\"}");  // add to response
  }
}
//========================================================================================================================
void ControlsDashboardSupervisor::GenerateUID(cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut, std::string pvlist) {
  __COUT__ << this->getApplicationDescriptor()->getLocalId() << " "
           << "Generating UID" << std::endl;

  std::set<std::string> pvDependencies;
  std::string uid;
  std::string pv;
  size_t pos = 0;
  size_t nextPos;
  size_t lastIndex = pvlist.find_last_of(",");

  if (pvlist.size() > 0) {
    // pvlist.substr(2);
    __COUT__ << pvlist << std::endl;

    while ((nextPos = pvlist.find(",", pos)) != std::string::npos) {
      pv = pvlist.substr(pos, nextPos - pos);
      //__COUT__ << UID_ << ":" << pos << "-" << nextPos << " ->" << pv << std::endl;
      pvDependencies.insert(pv);
      pos = nextPos + 1;
    }

    pvDependencyLookupMap_.insert(std::pair<int, std::set<std::string>>(++UID_, pvDependencies));

    uid = (std::string("{ \"message\": \"") + std::to_string(UID_) + "\"}");
  } else {
    __COUT__ << this->getApplicationDescriptor()->getLocalId() << " PVList invalid: " << pvlist << std::endl;
    uid = "{ \"message\": \"-1\"}";
  }

  __COUT__ << this->getApplicationDescriptor()->getLocalId() << " NEW UID: " << UID_ << std::endl;

  xmlOut.addTextElementToData("JSON", uid);  // add to response
}
//========================================================================================================================
void ControlsDashboardSupervisor::GetList(cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut) {
  __COUT__ << this->getApplicationDescriptor()->getLocalId() << std::endl;
  std::cout << " " << interface_->getList("JSON") << std::endl;

  xmlOut.addTextElementToData("JSON", interface_->getList("JSON"));  // add to response
}
//========================================================================================================================
void ControlsDashboardSupervisor::GetPages(cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut) {
  /*DIR * dir;
  struct dirent * ent;
  std::string pathToPages = PAGES_DIRECTORY;

  std::vector<std::string> pages;

  __COUT__ << this->getApplicationDescriptor()->getLocalId() << "Path to pages: " << pathToPages << std::endl;
  if((dir = opendir (pathToPages.c_str())) != NULL)
  {
          while((ent = readdir(dir)) != NULL)
          {
                  pages.push_back(ent->d_name);
                  __COUT__ << this->getApplicationDescriptor()->getLocalId() << " GetPages"	<< ent->d_name <<
  std::endl;
          }
          closedir(dir);
  }
  else
  {
          __COUT__ << this->getApplicationDescriptor()->getLocalId() << "Could not open directory: " << pathToPages <<
  std::endl; return;
  }*/
  std::vector<std::string> pages;

  listFiles("", true, &pages);

  std::string returnJSON = "[";
  for (auto it = pages.begin(); it != pages.end(); it++) {
    if (*it != "." && *it != "..") returnJSON += "\"" + *it + "\", ";
  }
  if (returnJSON.size() > 2 && returnJSON.compare("[") != 0) {
    __COUT__ << "Found pages on server!" << std::endl;
    returnJSON.resize(returnJSON.size() - 2);
    returnJSON += "]";
  } else {
    // No pages on the server
    __COUT__ << "No pages found on server!" << std::endl;
    returnJSON = "[\"None\"]";
  }
  std::cout << returnJSON << std::endl;

  xmlOut.addTextElementToData("JSON", returnJSON);  // add to response
}
//========================================================================================================================
void ControlsDashboardSupervisor::loadPage(cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut, std::string page) {
  // FIXME Filter out malicious attacks i.e. ../../../../../ stuff
  struct stat buffer;
  if (page.find("..") != std::string::npos) {
    __COUT__ << this->getApplicationDescriptor()->getLocalId() << "Error! Request using '..': " << page << std::endl;
  } else if (page.find("~") != std::string::npos) {
    __COUT__ << this->getApplicationDescriptor()->getLocalId() << "Error! Request using '~': " << page << std::endl;
  } else if (!(stat(page.c_str(), &buffer) == 0)) {
    __COUT__ << this->getApplicationDescriptor()->getLocalId() << "Error! File not found: " << page << std::endl;
  }

  std::string file = PAGES_DIRECTORY file += "/" + page;
  __COUT__ << this->getApplicationDescriptor()->getLocalId() << "Trying to load page: " << page << std::endl;
  __COUT__ << this->getApplicationDescriptor()->getLocalId() << "Trying to load page: " << file << std::endl;
  // read file
  // for each line in file

  std::ifstream infile(file);
  std::cout << "Reading file" << std::endl;
  std::string JSONpage = "";
  for (std::string line; getline(infile, line);) {
    std::cout << line << std::endl;
    JSONpage += line;
  }
  std::cout << "Finished reading file" << std::endl;

  xmlOut.addTextElementToData("JSON", JSONpage);  // add to response
}
//========================================================================================================================
void ControlsDashboardSupervisor::Subscribe(cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut) {}
//========================================================================================================================
void ControlsDashboardSupervisor::Unsubscribe(cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut) {}
//========================================================================================================================
//========================================================================================================================
//================================================== UTILITIES
//===========================================================
//========================================================================================================================
bool ControlsDashboardSupervisor::isDir(std::string dir) {
  struct stat fileInfo;
  stat(dir.c_str(), &fileInfo);
  if (S_ISDIR(fileInfo.st_mode)) {
    return true;
  } else {
    return false;
  }
}
//========================================================================================================================
void ControlsDashboardSupervisor::listFiles(std::string baseDir, bool recursive, std::vector<std::string>* pages) {
  std::string base = PAGES_DIRECTORY;
  base += baseDir;

  DIR* dp;
  struct dirent* dirp;
  if ((dp = opendir(base.c_str())) == NULL) {
    std::cout << "[ERROR: " << errno << " ] Couldn't open " << base << "." << std::endl;
    return;
  } else {
    while ((dirp = readdir(dp)) != NULL) {
      if (dirp->d_name != std::string(".") && dirp->d_name != std::string("..")) {
        if (isDir(base + dirp->d_name) == true && recursive == true) {
          // pages->push_back(baseDir + dirp->d_name);
          std::cout << "[DIR]\t" << baseDir << dirp->d_name << "/" << std::endl;
          listFiles(baseDir + dirp->d_name + "/", true, pages);
        } else {
          pages->push_back(baseDir + dirp->d_name);
          std::cout << "[FILE]\t" << baseDir << dirp->d_name << std::endl;
        }
      }
    }
    closedir(dp);
  }
}
