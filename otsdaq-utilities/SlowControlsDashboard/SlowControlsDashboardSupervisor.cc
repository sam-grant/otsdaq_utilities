#include "otsdaq-utilities/SlowControlsDashboard/SlowControlsDashboardSupervisor.h"

#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/Macros/CoutHeaderMacros.h"

#include <xdaq/NamespaceURI.h>
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"
#include "otsdaq-core/WebUsersUtilities/WebUsers.h"
#include "otsdaq-core/SOAPUtilities/SOAPParameters.h"


#include <sys/stat.h> //for quickly checking if file exists
#include <dirent.h> //for DIR
#include <iostream>
#include <fstream>
#include <string>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <errno.h>

#include "otsdaq-utilities/SlowControlsDashboard/EpicsInterface.h"
#include "otsdaq-utilities/SlowControlsDashboard/SlowControlsInterface.h"
#define PAGES_DIRECTORY 			std::string(getenv("SERVICE_DATA_PATH")) + "/SlowControlsDashboardData/pages/";

using namespace ots;




XDAQ_INSTANTIATOR_IMPL(SlowControlsDashboardSupervisor)

//========================================================================================================================
SlowControlsDashboardSupervisor::SlowControlsDashboardSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception)
:	xdaq::Application(s   ),
SOAPMessenger  (this),
theRemoteWebUsers_(this)
{
	INIT_MF("SlowControlsDashboardSupervisor");
	std::cout << __COUT_HDR_FL__ << this->getApplicationDescriptor()->getLocalId() << std::endl;

	xgi::bind (this, &SlowControlsDashboardSupervisor::requestHandler, "Default");
	init();

}

//========================================================================================================================
SlowControlsDashboardSupervisor::~SlowControlsDashboardSupervisor(void)
{
	destroy();
}
//========================================================================================================================
void SlowControlsDashboardSupervisor::requestHandler(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
	std::cout << __COUT_HDR_FL__ << std::endl;
	cgicc::Cgicc cgi(in);
	std::cout << __COUT_HDR_FL__ << std::endl;
	std::string Command = CgiDataUtilities::getData(cgi,"RequestType");
	std::cout << __COUT_HDR_FL__ << Command << std::endl;
	std::cout << __COUT_HDR_FL__ << this->getApplicationDescriptor()->getLocalId() << " " << Command << " : end"<< std::endl;


	if(Command == "")
	{
		Default(in, out);	
		return;
	}

	HttpXmlDocument xmldoc;
	uint64_t activeSessionIndex;
	std::string user;
	uint8_t userPermissions;

	//**** start LOGIN GATEWAY CODE ***//
	{
		bool automaticCommand = Command == "poll"; //automatic commands should not refresh cookie code.. only user initiated commands should!
		bool checkLock = true;
		bool getUser = false;

		if(!theRemoteWebUsers_.xmlLoginGateway(
				cgi,out,&xmldoc,theSupervisorsConfiguration_,
				&userPermissions,  		//acquire user's access level (optionally null pointer)
				"0",						//report user's ip address, if known
				!automaticCommand,			//true/false refresh cookie code
				1, //set access level requirement to pass gateway
				checkLock,					//true/false enable check that system is unlocked or this user has the lock
				0,//&userWithLock,			//acquire username with lock (optionally null pointer)
				(getUser?&user:0),				//acquire username of this user (optionally null pointer)
				0,//,&displayName			//acquire user's Display Name
				&activeSessionIndex		//acquire user's session index associated with the cookieCode
		))
		{	//failure
			std::cout << __COUT_HDR_FL__ << "Failed Login Gateway: " <<
			out->str() << std::endl; //print out return string on failure
			return;
		}
	}
	//**** end LOGIN GATEWAY CODE ***//




	//return xml doc holding server response
	std::cout << __COUT_HDR_FL__ << std::endl;

	if(Command == "poll")
	{
		std::string uid = CgiDataUtilities::getOrPostData(cgi,"uid");
		Poll(in, out, &xmldoc, uid);
	}
	else if(Command == "generateUID")
	{
		std::string pvList = CgiDataUtilities::getOrPostData(cgi,"PVList");		
		GenerateUID(in, out, &xmldoc, pvList);
	}
	else if(Command == "GetPVSettings")
	{
		std::string pvList = CgiDataUtilities::getOrPostData(cgi,"PVList");		
		GetPVSettings(in, out, &xmldoc, pvList);
		xmldoc.addTextElementToData("id", CgiDataUtilities::getData(cgi,"id"));
	}
	else if(Command == "getList")
	{
		GetList(in, out, &xmldoc);
	}
	else if(Command == "getPages")
	{
		GetPages(in, out, &xmldoc);
	}
	else if(Command == "loadPage")
	{
		std::string page = CgiDataUtilities::getData(cgi,"Page");
		std::cout << __COUT_HDR_FL__ << this->getApplicationDescriptor()->getLocalId() << " " << page << std::endl;

		loadPage(in, out, &xmldoc, page);
	}


	xmldoc.outputXmlDocument((std::ostringstream*) out, true);


}
//========================================================================================================================
void SlowControlsDashboardSupervisor::init(ConfigurationManager *configManager)
//called by constructor
{
	UID_= 0;

	theSupervisorsConfiguration_.init(getApplicationContext());
	//if(true)
	interface_ = new EpicsInterface();
	//interface_->initialize();
	std::thread([&](){interface_->initialize();}).detach(); //thread completes after creating, subscribing, and getting parameters for all pvs

}
//========================================================================================================================
void SlowControlsDashboardSupervisor::destroy(void)
{
	//called by destructor
	delete interface_;
}

//========================================================================================================================
void SlowControlsDashboardSupervisor::Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
	std::cout << __COUT_HDR_FL__ << this->getApplicationDescriptor()->getLocalId() << std::endl;

	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/SlowControlsDashboard.html?urn=" <<
	this->getApplicationDescriptor()->getLocalId() <<"' /></frameset></html>";
}
//========================================================================================================================
void SlowControlsDashboardSupervisor::Poll(xgi::Input * in, xgi::Output * out, HttpXmlDocument *xmldoc, std::string UID ) throw (xgi::exception::Exception)
{

	std::cout << __COUT_HDR_FL__ << this->getApplicationDescriptor()->getLocalId() << " "	<< "Polling on UID:" << UID << std::endl;

	std::map<int, std::set<std::string>>::iterator mapReference;

	if( UID != "" && (mapReference = pvDependencyLookupMap_.find(std::stoi(UID))) != pvDependencyLookupMap_.end()) //We have their current list of PV Dependencies
	{
		std::string JSONMessage = "{ ";

		for(auto pv : mapReference->second)
		{
			std::cout << __COUT_HDR_FL__ << pv << std::endl;
			//PVInfo * pvInfo = interface_->mapOfPVInfo_.find(pv)->second;
			//std::cout << __COUT_HDR_FL__ << pv  << ":" << (pvInfo?"Good":"Bad") << std::endl;
			//interface_->getCurrentPVValue(pv);
			std::array<std::string, 4> pvInformation = interface_->getCurrentPVValue(pv);

			std::cout << __COUT_HDR_FL__ << pv << ": " << pvInformation[1] <<  " : " << pvInformation[3] << std::endl;

			
			if(pvInformation[0] != "NO_CHANGE")
			{
				//std::cout << __COUT_HDR_FL__ << "Reached" <<  std::endl;
				JSONMessage += "\"" + pv + "\": {";

				/*if(pvInfo->mostRecentBufferIndex - 1 < 0)
			{
				std::string value = pvInfo->dataCache[pvInfo->dataCache.size()].second
				std::string time = 
			}*/

				JSONMessage += "\"Timestamp\" : \""  + pvInformation[0] + "\",";
				JSONMessage += "\"Value\"     : \""  + pvInformation[1] + "\",";
				JSONMessage += "\"Status\"    : \""  + pvInformation[2] + "\",";
				JSONMessage += "\"Severity\"  : \""  + pvInformation[3] + "\"},";
				
			}
			else
			{
				std::cout << __COUT_HDR_FL__ << "No change in value since last poll: " <<  pv << std::endl;
			}

			//Handle Channels that disconnect, etc
			if(pvInformation[3] == "INVALID")
			{
				interface_->subscribe(pv);
			}
			
			
			//std::cout << __COUT_HDR_FL__ << pv  << ":" << (pvInfo?"Good":"Bad") << std::endl;
			//std::cout << __COUT_HDR_FL__ << pv  << ":" << pvInfo->mostRecentBufferIndex -1 << std::endl;
			//std::cout << __COUT_HDR_FL__ << pv << " : " << pvInfo->dataCache[(pvInfo->mostRecentBufferIndex -1)].second << std::endl;

		}

		JSONMessage = JSONMessage.substr(0, JSONMessage.length() - 1);
		JSONMessage += "}";
		std::cout << __COUT_HDR_FL__ << JSONMessage << std::endl;
		xmldoc->addTextElementToData("JSON", JSONMessage); //add to response

		/*for (std::set<unsigned long>::iterator it = mapReference->second->begin(); it != mapReference->second.end(); ++it)
		{
			//std::cout << __COUT_HDR_FL__ << this->getApplicationDescriptor()->getLocalId() << it << std::endl;


		}*/
		/*std::string fakeData = 	std::string("{")
								+ "\"Mu2e_CompStatus_daq01/system_temperature\": \"40.5\","
								+ "\"Mu2e_CompStatus_daq01/load_one\": \"378.2\","
								+ "\"Mu2e_Weather_2/timestamp\": \"11.14.45.2016.4.8\","
								+ "\"Mu2e_CompStatus_daq01/system_temperature\": \"43.4\","
								+ "\"Mu2e_CompStatus_daq01/load_one\":\"80\","
								+ "\"Mu2e_Weather_2/timestamp\": \"11.14.45.2016.4.8\""
								+ "}";
		xmldoc->addTextElementToData("JSON", fakeData); //add to response*/





	}
	else // UID is not in our map so force them to generate a new one
	{
		xmldoc->addTextElementToData("JSON", "{ \"message\": \"NOT_FOUND\"}"); //add to response
	}
}
//========================================================================================================================
void SlowControlsDashboardSupervisor::GetPVSettings(xgi::Input * in, xgi::Output * out, HttpXmlDocument *xmldoc, std::string pvList ) throw (xgi::exception::Exception)
{
	std::cout << __COUT_HDR_FL__ << this->getApplicationDescriptor()->getLocalId() << " "	<< "Getting settings for " << pvList << std::endl;

	std::string JSONMessage = "{ ";

	std::string pv;
	size_t pos = 0;
	size_t nextPos;
	size_t lastIndex = pvList.find_last_of(",");
	std::cout << "**********************" << pvList.size() << std::endl;
	if(pvList.size() > 0)
	{

		while((nextPos = pvList.find(",", pos)) != std::string::npos)
		{
			pv = pvList.substr(pos, nextPos-pos);


			std::cout << __COUT_HDR_FL__ << pv << std::endl;

			std::array<std::string, 9> pvSettings = interface_->getPVSettings(pv);


			JSONMessage += "\"" + pv + "\": {";			
			JSONMessage += "\"Units              \": \""  + pvSettings[0] + "\",";
			JSONMessage += "\"Upper_Display_Limit\": \""  + pvSettings[1] + "\",";
			JSONMessage += "\"Lower_Display_Limit\": \""  + pvSettings[2] + "\",";
			JSONMessage += "\"Upper_Alarm_Limit  \": \""  + pvSettings[3] + "\",";
			JSONMessage += "\"Upper_Warning_Limit\": \""  + pvSettings[4] + "\",";
			JSONMessage += "\"Lower_Warning_Limit\": \""  + pvSettings[5] + "\",";
			JSONMessage += "\"Lower_Alarm_Limit  \": \""  + pvSettings[6] + "\",";
			JSONMessage += "\"Upper_Control_Limit\": \""  + pvSettings[7] + "\",";
			JSONMessage += "\"Lower_Control_Limit\": \""  + pvSettings[8] + "\"},";

			pos = nextPos + 1;
		}

		JSONMessage = JSONMessage.substr(0, JSONMessage.length() - 1);
		JSONMessage += "}";

		std::cout << __COUT_HDR_FL__ << JSONMessage << std::endl;
		xmldoc->addTextElementToData("JSON", JSONMessage); //add to response

	}
	else
	{
		xmldoc->addTextElementToData("JSON", "{ \"message\": \"GetPVSettings\"}"); //add to response
	}




}
//========================================================================================================================
void SlowControlsDashboardSupervisor::GenerateUID(xgi::Input * in, xgi::Output * out, HttpXmlDocument *xmldoc, std::string pvlist ) throw (xgi::exception::Exception)
{

	std::cout << __COUT_HDR_FL__ << this->getApplicationDescriptor()->getLocalId() << " "	<< "Generating UID" << std::endl;

	std::set<std::string> pvDependencies;
	std::string uid;
	std::string pv;
	size_t pos = 0;
	size_t nextPos;
	size_t lastIndex = pvlist.find_last_of(",");

	if(pvlist.size() > 0)
	{
		//pvlist.substr(2);
		std::cout << __COUT_HDR_FL__ << pvlist << std::endl;

		while((nextPos = pvlist.find(",", pos)) != std::string::npos)
		{
			pv = pvlist.substr(pos, nextPos-pos);
			//std::cout << __COUT_HDR_FL__ << UID_ << ":" << pos << "-" << nextPos << " ->" << pv << std::endl;
			pvDependencies.insert(pv);
			pos = nextPos + 1;
		}

		pvDependencyLookupMap_.insert(std::pair<int, std::set<std::string>> (++UID_, pvDependencies) );

		uid = (std::string("{ \"message\": \"") + std::to_string(UID_) +"\"}");
	}else
	{
		std::cout << __COUT_HDR_FL__ << this->getApplicationDescriptor()->getLocalId() << " PVList invalid: "	<< pvlist << std::endl;
		uid = "{ \"message\": \"-1\"}";
	}


	std::cout << __COUT_HDR_FL__ << this->getApplicationDescriptor()->getLocalId() << " NEW UID: "	<< UID_ << std::endl;

	xmldoc->addTextElementToData("JSON", uid); //add to response


}
//========================================================================================================================
void SlowControlsDashboardSupervisor::GetList(xgi::Input * in, xgi::Output * out, HttpXmlDocument *xmldoc ) throw (xgi::exception::Exception)
{

	std::cout << __COUT_HDR_FL__ << this->getApplicationDescriptor()->getLocalId() << " "	<< interface_->getList("JSON") << std::endl;

	xmldoc->addTextElementToData("JSON", interface_->getList("JSON")); //add to response

}
//========================================================================================================================
void SlowControlsDashboardSupervisor::GetPages(xgi::Input * in, xgi::Output * out, HttpXmlDocument *xmldoc ) throw (xgi::exception::Exception)
{
	/*DIR * dir;
	struct dirent * ent;
	std::string pathToPages = PAGES_DIRECTORY;

	std::vector<std::string> pages;

	std::cout << __COUT_HDR_FL__ << this->getApplicationDescriptor()->getLocalId() << "Path to pages: " << pathToPages << std::endl;
	if((dir = opendir (pathToPages.c_str())) != NULL)
	{
		while((ent = readdir(dir)) != NULL)
		{
			pages.push_back(ent->d_name);
			std::cout << __COUT_HDR_FL__ << this->getApplicationDescriptor()->getLocalId() << " GetPages"	<< ent->d_name << std::endl;
		}
		closedir(dir);
	}
	else
	{
		std::cout << __COUT_HDR_FL__ << this->getApplicationDescriptor()->getLocalId() << "Could not open directory: " << pathToPages << std::endl;
		return;
	}*/
	std::vector<std::string> pages;

	listFiles("", true, &pages);

	std::string returnJSON = "[";
	for(auto it = pages.begin(); it != pages.end(); it++)
	{
		if(*it != "." && *it != "..")
			returnJSON += "\"" + *it + "\", ";
	}

	returnJSON.resize(returnJSON.size()-2);
	returnJSON += "]";     

	std::cout << returnJSON << std::endl;

	xmldoc->addTextElementToData("JSON", returnJSON); //add to response

}
//========================================================================================================================
void SlowControlsDashboardSupervisor::loadPage(xgi::Input * in, xgi::Output * out, HttpXmlDocument *xmldoc, std::string page ) throw (xgi::exception::Exception)
{
	//FIXME Filter out malicious attacks i.e. ../../../../../ stuff
	struct stat buffer;
	if(page.find("..") != std::string::npos)
	{
		std::cout << __COUT_HDR_FL__ << this->getApplicationDescriptor()->getLocalId() << "Error! Request using '..': " << page << std::endl;
	}
	else if (page.find("~") != std::string::npos)
	{
		std::cout << __COUT_HDR_FL__ << this->getApplicationDescriptor()->getLocalId() << "Error! Request using '~': " << page << std::endl;
	}
	else if(!(stat(page.c_str(), &buffer) == 0))
	{
		std::cout << __COUT_HDR_FL__ << this->getApplicationDescriptor()->getLocalId() << "Error! File not found: " << page << std::endl;
	}

	std::string file = PAGES_DIRECTORY
	file += "/" + page;
	std::cout << __COUT_HDR_FL__ << this->getApplicationDescriptor()->getLocalId() << "Trying to load page: "	<< page << std::endl;
	std::cout << __COUT_HDR_FL__ << this->getApplicationDescriptor()->getLocalId() << "Trying to load page: "	<< file << std::endl;
	//read file 
	//for each line in file

	std::ifstream infile(file);
	std::cout << "Reading file" << std::endl;
	std::string JSONpage = "";
	for(std::string line; getline(infile, line);)
	{
		std::cout << line << std::endl;
		JSONpage += line;
	}
	std::cout << "Finished reading file" << std::endl;

	xmldoc->addTextElementToData("JSON", JSONpage); //add to response

}
//========================================================================================================================
void SlowControlsDashboardSupervisor::Subscribe(xgi::Input * in, xgi::Output * out, HttpXmlDocument *xmldoc ) throw (xgi::exception::Exception)
{


}
//========================================================================================================================
void SlowControlsDashboardSupervisor::Unsubscribe(xgi::Input * in, xgi::Output * out, HttpXmlDocument *xmldoc ) throw (xgi::exception::Exception)
{


}
//========================================================================================================================
//========================================================================================================================
//================================================== UTILITIES ===========================================================
//========================================================================================================================
bool SlowControlsDashboardSupervisor::isDir(std::string dir)
{
	struct stat fileInfo;
	stat(dir.c_str(), &fileInfo);
	if (S_ISDIR(fileInfo.st_mode))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void SlowControlsDashboardSupervisor::listFiles(std::string baseDir, bool recursive, std::vector<std::string> * pages )
{
	std::string base = PAGES_DIRECTORY;
	base += baseDir;

	DIR *dp;
	struct dirent *dirp;
	if ((dp = opendir(base.c_str())) == NULL) {
		std::cout << "[ERROR: " << errno << " ] Couldn't open " << base << "." << std::endl;
		return;
	}
	else
	{
		while ((dirp = readdir(dp)) != NULL) {
			if (dirp->d_name != std::string(".") && dirp->d_name != std::string(".."))
			{
				if (isDir(base + dirp->d_name) == true && recursive == true)
				{
					//pages->push_back(baseDir + dirp->d_name);
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



