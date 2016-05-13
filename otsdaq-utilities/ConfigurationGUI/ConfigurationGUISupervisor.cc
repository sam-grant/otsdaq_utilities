#include "otsdaq-utilities/ConfigurationGUI/ConfigurationGUISupervisor.h"
#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/Macros/CoutHeaderMacros.h"
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"


#include "otsdaq-core/ConfigurationPluginDataFormats/ConfigurationAliases.h"
#include "otsdaq-core/ConfigurationPluginDataFormats/Configurations.h"
#include "otsdaq-core/ConfigurationPluginDataFormats/DetectorConfiguration.h"
#include "otsdaq-core/ConfigurationPluginDataFormats/DataManagerConfiguration.h"
#include "otsdaq-core/ConfigurationPluginDataFormats/DefaultConfigurations.h"
#include "otsdaq-core/ConfigurationPluginDataFormats/VersionAliases.h"

#include "otsdaq-demo/UserConfigurationDataFormats/FSSRDACsConfiguration.h"
#include "otsdaq-core/ConfigurationDataFormats/ROCDACs.h"


#include <xdaq/NamespaceURI.h>

#include <iostream>
#include <map>
#include <utility>

using namespace ots;

XDAQ_INSTANTIATOR_IMPL(ConfigurationGUISupervisor)

//========================================================================================================================
ConfigurationGUISupervisor::ConfigurationGUISupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception):
xdaq::Application	(s   ),
SOAPMessenger  		(this),
theRemoteWebUsers_  (this)
{
  INIT_MF("ConfigurationGUI");
	xgi::bind (this, &ConfigurationGUISupervisor::Default, "Default" );
	xgi::bind (this, &ConfigurationGUISupervisor::request, "Request" );
	init();


	//new user gets a config mgr assigned
	//user can fill any of the sub-configs (fill from version or init empty), which becomes the active view for that sub-config
	//if user then edits one of the sub-configs, just before editing, active view is copied to edit view which is version -1
	//if the user saves the configuration, then any sub-configurations with active view -1 get saved with a new version number

	//FIXME TODO
	//LEFT OFF - Proof of Concept
	//	Choose a sub-config
	//	Make a copy of a view
	//	Edit copy
	//	Save as new sub-config version


	//how do we know which version numbers of a KOC exist already?

	mf::LogDebug(__FILE__) << "comment/uncomment here for debugging Configuration!" << "     ";
	return;


	//new user
	std::string userConfigurationManagerIndex = "1";
	userConfigurationManagers_["1"] = new ConfigurationManager();
	userConfigurationManagers_["2"] = new ConfigurationManager();
	ConfigurationManager *cfgMgr = userConfigurationManagers_[userConfigurationManagerIndex];

	std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
	std::map<std::string, ConfigurationKey>	aliasMap = cfgMgr->getConfiguration<ConfigurationAliases>()->getAliasesMap();


	mf::LogDebug(__FILE__) << __COUT_HDR_L__ << "aliasMap size: " << aliasMap.size() << "     ";
	mf::LogDebug(__FILE__) << __COUT_HDR_L__ << "getAllConfigurationInfo size: " << allCfgInfo.size() << "     ";


	std::set<std::string> listOfKocs;
	std::map<std::string, ConfigurationKey>::const_iterator it = aliasMap.begin();
	while (it != aliasMap.end())
	{
		//for each configuration alias and key
		//get KOC version numbers

		mf::LogDebug(__FILE__) << __COUT_HDR_L__ << "Alias: " << it->first << " - Key: " << it->second.key() << "     ";

		listOfKocs = cfgMgr->getConfiguration<Configurations>()->getListOfKocs(it->second.key());
		mf::LogDebug(__FILE__) << __COUT_HDR_L__ << "\tKocs size: " << listOfKocs.size() << "     ";

		for (auto& koc : listOfKocs)
		{
			unsigned int conditionVersion = cfgMgr->getConfiguration<Configurations>()->getConditionVersion(it->second.key(),koc);

			mf::LogDebug(__FILE__) << "\tKoc: " << koc << " Version: " << conditionVersion << "     ";
		}
		++it;

	}


	//Get KOC list
	//for each configuration alias and key
	//Get KOC keys for Alias key




	auto mapIt = allCfgInfo.begin();
	while(mapIt != allCfgInfo.end())
	{
		mf::LogDebug(__FILE__) << __COUT_HDR_L__ << "KOC Alias: " << mapIt->first << "     ";
		mf::LogDebug(__FILE__) << __COUT_HDR_L__ << "\t\tExisting Versions: " << mapIt->second.versions_.size() << "     ";

		//get version key for the current system subconfiguration key
		for (std::set<int>::iterator vit=mapIt->second.versions_.begin(); vit!=mapIt->second.versions_.end(); ++vit)
		{
			mf::LogDebug(__FILE__) << __COUT_HDR_L__ << "\t\t" << *vit << "     ";

		}
		++mapIt;
	}


	return;


	//Choose a sub config
	mf::LogDebug(__FILE__) << "\t\t**************************** Choose a sub config" << "     ";
	std::string chosenSubConfig = "FSSRDACsConfiguration"; //must be less than allCfgInfo.size() //TODO Alfredo: ask why this std::string is hardcoded

	{
		int versionToCopy = 0; //-1 is empty, //must be less than allCfgInfo[chosenSubConfig].versions_.size()

		bool isInDatabase = allCfgInfo[chosenSubConfig].versions_.find(versionToCopy) != allCfgInfo[chosenSubConfig].versions_.end();
		mf::LogDebug(__FILE__) << "Version " << versionToCopy << " is in database: " <<
				(isInDatabase?"YES":"NO") << "     ";

		bool isInConfiguration = (allCfgInfo[chosenSubConfig].configurationPtr_->isStored(versionToCopy));
		mf::LogDebug(__FILE__) << "Version " << versionToCopy << " is loaded: " <<
				(isInConfiguration?"YES":"NO") << "     ";

		if(!isInConfiguration) //load configuration view
			cfgMgr->getVersionedConfigurationByName(chosenSubConfig, versionToCopy);
		else
			allCfgInfo[chosenSubConfig].configurationPtr_->setActiveView(versionToCopy);

		isInConfiguration = (allCfgInfo[chosenSubConfig].configurationPtr_->isStored(versionToCopy));
		mf::LogDebug(__FILE__) << "Version " << versionToCopy << " is loaded: " <<
				(isInConfiguration?"YES":"NO") << "     ";
	}

	//view this version
	//general base class
	{
		//get 'columns' of sub config

		mf::LogDebug(__FILE__) << "\t\t******** view " <<
				allCfgInfo[chosenSubConfig].configurationPtr_->getViewVersion() << "     ";
		ConfigurationView* cfgViewPtr = allCfgInfo[chosenSubConfig].configurationPtr_->getViewP();

		std::vector<ViewColumnInfo> colInfo = cfgViewPtr->getColumnsInfo();

		mf::LogDebug(__FILE__) << "\t\tNumber of Cols " << colInfo.size() << "     ";
		mf::LogDebug(__FILE__) << "\t\tNumber of Rows " << cfgViewPtr->getNumberOfRows() << "     ";
	}


	mf::LogDebug(__FILE__) << "\t\t**************************** Choose a different sub config" << "     ";


	{
		int versionToCopy = 2; //-1 is empty, //must be less than allCfgInfo[chosenSubConfig].versions_.size()

		bool isInDatabase = allCfgInfo[chosenSubConfig].versions_.find(versionToCopy) != allCfgInfo[chosenSubConfig].versions_.end();
		mf::LogDebug(__FILE__) << "Version " << versionToCopy << " is in database: " <<
				(isInDatabase?"YES":"NO") << "     ";

		bool isInConfiguration = (allCfgInfo[chosenSubConfig].configurationPtr_->isStored(versionToCopy));
		mf::LogDebug(__FILE__) << "Version " << versionToCopy << " is loaded: " <<
				(isInConfiguration?"YES":"NO") << "     ";

		if(!isInConfiguration) //load configuration view
			cfgMgr->getVersionedConfigurationByName(chosenSubConfig, versionToCopy);
		else
			allCfgInfo[chosenSubConfig].configurationPtr_->setActiveView(versionToCopy);

		isInConfiguration = (allCfgInfo[chosenSubConfig].configurationPtr_->isStored(versionToCopy));
		mf::LogDebug(__FILE__) << "Version " << versionToCopy << " is loaded: " <<
				(isInConfiguration?"YES":"NO") << "     ";
	}

	{
		//get 'columns' of sub config

		mf::LogDebug(__FILE__) << "\t\t******** view " <<
				allCfgInfo[chosenSubConfig].configurationPtr_->getViewVersion() << "     ";
		ConfigurationView* cfgViewPtr = allCfgInfo[chosenSubConfig].configurationPtr_->getViewP();

		std::vector<ViewColumnInfo> colInfo = cfgViewPtr->getColumnsInfo();

		mf::LogDebug(__FILE__) << "\t\tNumber of Cols " << colInfo.size() << "     ";
		mf::LogDebug(__FILE__) << "\t\tNumber of Rows " << cfgViewPtr->getNumberOfRows() << "     ";
	}


	mf::LogDebug(__FILE__) << "\t\t**************************** Choose a different sub config" << "     ";

	{
		int versionToCopy = 0; //-1 is empty, //must be less than allCfgInfo[chosenSubConfig].versions_.size()

		bool isInDatabase = allCfgInfo[chosenSubConfig].versions_.find(versionToCopy) != allCfgInfo[chosenSubConfig].versions_.end();
		mf::LogDebug(__FILE__) << "Version " << versionToCopy << " is in database: " <<
				(isInDatabase?"YES":"NO") << "     ";

		bool isInConfiguration = (allCfgInfo[chosenSubConfig].configurationPtr_->isStored(versionToCopy));
		mf::LogDebug(__FILE__) << "Version " << versionToCopy << " is loaded: " <<
				(isInConfiguration?"YES":"NO") << "     ";

		if(!isInConfiguration) //load configuration view
			cfgMgr->getVersionedConfigurationByName(chosenSubConfig, versionToCopy);
		else
			allCfgInfo[chosenSubConfig].configurationPtr_->setActiveView(versionToCopy);

		isInConfiguration = (allCfgInfo[chosenSubConfig].configurationPtr_->isStored(versionToCopy));
		mf::LogDebug(__FILE__) << "Version " << versionToCopy << " is loaded: " <<
				(isInConfiguration?"YES":"NO") << "     ";
	}

	{
		//get 'columns' of sub config

		mf::LogDebug(__FILE__) << "\t\t******** view " <<
				allCfgInfo[chosenSubConfig].configurationPtr_->getViewVersion() << "     ";
		ConfigurationView* cfgViewPtr = allCfgInfo[chosenSubConfig].configurationPtr_->getViewP();

		std::vector<ViewColumnInfo> colInfo = cfgViewPtr->getColumnsInfo();

		mf::LogDebug(__FILE__) << "\t\tNumber of Cols " << colInfo.size() << "     ";
		mf::LogDebug(__FILE__) << "\t\tNumber of Rows " << cfgViewPtr->getNumberOfRows() << "     ";
	}


	int versionToCopy = 0;
	//	Make a copy of a view
	mf::LogDebug(__FILE__) << "\t\t**************************** Make a copy of a view" << "     ";
	int temporaryVersion = allCfgInfo[chosenSubConfig].configurationPtr_->createTemporaryView(versionToCopy);
	mf::LogDebug(__FILE__) << "\t\ttemporaryVersion: " << temporaryVersion << "     ";


	//	Edit copy
	mf::LogDebug(__FILE__) << "\t\t**************************** Edit copy" << "     ";

	//view temp version
	//general base class
	{
		//get 'columns' of sub config

		mf::LogDebug(__FILE__) << "\t\t******** Before change" << "     ";
		ConfigurationView* cfgViewPtr = allCfgInfo[chosenSubConfig].configurationPtr_->getTemporaryView(temporaryVersion);

		std::vector<ViewColumnInfo> colInfo = cfgViewPtr->getColumnsInfo();

		std::stringstream ss;
		cfgViewPtr->print(ss);
		mf::LogDebug(__FILE__) << ss.str();


		//edit something

		// add a new row for example

		int r = cfgViewPtr->addRow();
		//set all numbers to 10
		r = cfgViewPtr->addRow();
		for(int c=0;c<(int)cfgViewPtr->getNumberOfColumns();++c)
			if(colInfo[c].getViewType() == "NUMBER")
				cfgViewPtr->setValue(c,r,c);
			else
				cfgViewPtr->setValue(std::string("Hi"),r,c);
		cfgViewPtr->addRow();

		//after change
		mf::LogDebug(__FILE__) << "\t\t******** After change" << "     ";



		std::stringstream ss2;
		cfgViewPtr->print(ss2);
		mf::LogDebug(__FILE__) << ss2.str();


		// add a new col for example
		//		int cret = cfgViewPtr->addColumn("TestCol","TEST_COL","TIMESTAMP WITH TIMEZONE");
		//		mf::LogDebug(__FILE__) << "\t\t******** After change col: " << cret << "     ";
		//		colInfo = cfgViewPtr->getColumnsInfo();
		//		mf::LogDebug(__FILE__) << "\t\tNumber of Cols " << colInfo.size() << "     ";
		//		mf::LogDebug(__FILE__) << "\t\tNumber of Rows " << cfgViewPtr->getNumberOfRows() << "     ";
		//
		//		for(int r=0;r<(int)cfgViewPtr->getNumberOfRows();++r)
		//		{
		//			mf::LogDebug(__FILE__) << "\t\tRow " << r << ": " ;
		//
		//			for(int c=0;c<(int)cfgViewPtr->getNumberOfColumns();++c)
		//				if(colInfo[c].getViewType() == "NUMBER")
		//				{
		//					int num;
		//					cfgViewPtr->getValue(num,r,c);
		//					mf::LogDebug(__FILE__) << "\t " << num;
		//				}
		//				else
		//				{
		//				std::string val;
		//					cfgViewPtr->getValue(val,r,c);
		//					mf::LogDebug(__FILE__) << "\t " << val;
		//				}
		//			
		//		}

	}
	//FSSRDACsConfiguration* subConfig = ((FSSRDACsConfiguration*)(allCfgInfo[chosenSubConfig].configurationPtr_));

	//actually need to load detector configuration!!
	//	std::map<std::string, unsigned int>	FssrNameMap = cfgMgr->getDetectorConfiguration()->getNameToRowMap();
	//	///NOTE: FssrNameMap could be any kind of Roc ... check RocType
	//
	//	std::map<std::string, unsigned int>::const_iterator dit;
	//	dit = FssrNameMap.begin();
	//
	//	if (dit != FssrNameMap.end())
	//	{
	//		ROCDACs	rocDacs = subConfig->getROCDACs(dit->first);
	//		DACList	dacs  = rocDacs.getDACList(); //map of std::string to pair (int, int)
	//
	//		DACList::const_iterator rdit = dacs.begin();
	//		while(rdit != dacs.end())
	//		{
	//			mf::LogDebug(__FILE__) << "\t\tDac Name: " << rdit->first << " - Addr: " << rdit->second.first << ", Val: " << rdit->second.second << "     ";
	//			rocDacs.setDAC(rdit->first,7,1);
	//
	//			rdit++;
	//		}
	//	}

	//	Save as new sub-config version
	mf::LogDebug(__FILE__) << "\t\t**************************** Save as new sub-config version" << "     ";

	int newAssignedVersion = 0;//cfgMgr->saveNewConfiguration(allCfgInfo[chosenSubConfig].configurationPtr_,temporaryVersion);

	mf::LogDebug(__FILE__) << "\t\t newAssignedVersion: " << newAssignedVersion << "     ";




	//proof of concept.. change a KOC version (this makes a new backbone version?!)
	mf::LogDebug(__FILE__) << "\t\t**************************** Edit a KOC for a System Alias" << "     ";

	std::stringstream ss;
	cfgMgr->getConfiguration<ConfigurationAliases>()->print(ss);
	cfgMgr->getConfiguration<DefaultConfigurations>()->print(ss);
	cfgMgr->getConfiguration<Configurations>()->print(ss);
	cfgMgr->getConfiguration<VersionAliases>()->print(ss);
	mf::LogDebug(__FILE__) << ss.str();

	{
		std::string specSystemAlias = "Physics";
		std::string KOCAlias = "FSSRDACsConfiguration";
		int newVersion = 3;

		std::map<std::string, ConfigurationKey> aliasMap = cfgMgr->getConfiguration<ConfigurationAliases>()->getAliasesMap();

		std::map<std::string, ConfigurationKey>::const_iterator it = aliasMap.find(specSystemAlias);
		if(it != aliasMap.end())
		{
			mf::LogDebug(__FILE__) << "Alias: " << it->first << " - Key: " << it->second.key() << "     ";

			mf::LogDebug(__FILE__) << "Alias exists: " << specSystemAlias << "     ";
			mf::LogDebug(__FILE__) << "Sub system alias: " << KOCAlias << "     ";
			mf::LogDebug(__FILE__) << "Changing to new version: " << newVersion << "     ";

			std::set<std::string> listOfKocs;
			listOfKocs = cfgMgr->getConfiguration<Configurations>()->getListOfKocs(it->second.key());
			for (std::set<std::string>::iterator sit=listOfKocs.begin(); sit!=listOfKocs.end(); ++sit)
			{
				unsigned int cv = cfgMgr->getConfiguration<Configurations>()->getConditionVersion(it->second.key(),*sit);

				mf::LogDebug(__FILE__) << "\tKoc: " << *sit << " Version: " << cv << "     ";

				std::set<int> versions = allCfgInfo.find(*sit)->second.versions_;
				mf::LogDebug(__FILE__) << "\t\tAll versions: ";
				for (std::set<int>::iterator vit=versions.begin(); vit!=versions.end(); ++vit)
					mf::LogDebug(__FILE__) << " " << *vit;
				
			}

			//need to change

			mf::LogDebug(__FILE__) << "\t\t**************************** Make temporary backbone" << "     ";

			chosenSubConfig = "ConfigurationAliases";
			int temporaryVersion;
			int versionToCopy = cfgMgr->getConfiguration<ConfigurationAliases>()->getViewVersion();
			//cfgMgr->__GET_CONFIG__(DefaultConfigurations)->print();
			///////////////////////cfgMgr->getConfiguration<Configurations>()->print();
			//cfgMgr->__GET_CONFIG__(VersionAliases)

			mf::LogDebug(__FILE__) << "\t\ttemporaryVersion versionToCopy: " << versionToCopy << "     ";

			assert(allCfgInfo.find(chosenSubConfig) != allCfgInfo.end());

			bool isInDatabase = allCfgInfo[chosenSubConfig].versions_.find(versionToCopy) != allCfgInfo[chosenSubConfig].versions_.end();
			mf::LogDebug(__FILE__) << "Version " << versionToCopy << " is in database: " <<
					(isInDatabase?"YES":"NO") << "     ";

			temporaryVersion = allCfgInfo[chosenSubConfig].configurationPtr_->createTemporaryView(versionToCopy);
			//FSSRDACsConfiguration
			//allCfgInfo["ConfigurationAliases"].configurationPtr_->createTemporaryView(versionToCopy);
			mf::LogDebug(__FILE__) << "\t\ttemporaryVersion ConfigurationAliases: " << temporaryVersion << "     ";




		}
		else
			mf::LogDebug(__FILE__) << "Alias doesnt exist: " << specSystemAlias << "     ";




	}


	/*
   	//cfgMgr->getFSSRDACsConfiguration();


   	//Get KOC list
	//for each configuration alias and key
		//Get KOC keys for Alias key


   	it = aliasMap.begin();

   	std::set<std::string> listOfKocs;

   	while (it != aliasMap.end())
   	{

   		//for each configuration alias and key
   			//get KOC version numbers

		mf::LogDebug(__FILE__) << "Alias: " << it->first << " - Key: " << it->second.key() << "     ";

		//cfgMgr->loadConfiguration(cfgMgr->theConfigurations_,&(it->second));
		//cfgMgr->loadConfiguration(cfgMgr->theDefaultConfigurations_,&(it->second));
		//cfgMgr->loadConfiguration(cfgMgr->theVersionAliases_,&(it->second));
	    //cfgMgr->setupConfigurationGUI(&(it->second));	//load

		if(cfgMgr->getConfigurations())	// get Kocs
		{
			listOfKocs = cfgMgr->getConfigurations()->getListOfKocs(it->second.key());
			mf::LogDebug(__FILE__) << "\tKocs size: " << listOfKocs.size() << "     ";


			for (std::set<std::string>::iterator sit=listOfKocs.begin(); sit!=listOfKocs.end(); ++sit)
			{
				unsigned int cv = cfgMgr->getConfigurations()->getConditionVersion(it->second.key(),*sit);

				mf::LogDebug(__FILE__) << "\tKoc: " << *sit << " Version: " << cv << "     ";

			}
		}


		// get Detector Config
	   	if(cfgMgr->getDetectorConfiguration())
	   	{


			std::map<std::string, unsigned int>	FssrNameMap = cfgMgr->getDetectorConfiguration()->getNameToRowMap();
			///NOTE: FssrNameMap could be any kind of Roc ... check RocType

			std::map<std::string, unsigned int>::const_iterator dit;
			dit = FssrNameMap.begin();

			while (dit != FssrNameMap.end())
			{

				//for each configuration alias and key
					//get KOC version numbers

			   mf::LogDebug(__FILE__) << "\tFssr Name: " << dit->first << " - Row: " << dit->second << "     ";



				// get FSSR Configs
			   	if(cfgMgr->getFSSRDACsConfiguration())
			   	{
			   		ROCDACs	rocDacs = cfgMgr->getFSSRDACsConfiguration()->getROCDACs(dit->first);

			   		DACList	dacs  = rocDacs.getDACList(); //map of std::string to pair (int, int)


			   		DACList::const_iterator rdit = dacs.begin();
			   		while(rdit != dacs.end())
			   		{
						   mf::LogDebug(__FILE__) << "\t\tDac Name: " << rdit->first << " - Addr: " << rdit->second.first << ", Val: " << rdit->second.second << "     ";
							rocDacs.setDAC(rdit->first,7,1);

						   rdit++;

			   		}


			   		dacs  = rocDacs.getDACList(); //map of std::string to pair (int, int)
					rdit = dacs.begin();
					while(rdit != dacs.end())
					{
						   mf::LogDebug(__FILE__) << "\t\tDac Name: " << rdit->first << " - Addr: " << rdit->second.first << ", Val: " << rdit->second.second << "     ";
						   rdit++;

					}

			   	}


			   	dit++;
			}


	   	}


   	   it++;
   	}



   	//createTemporaryView

	 */
	mf::LogDebug(__FILE__) << "done getAliasList" << "     ";


	//clear config managers
	for (std::map<std::string, ConfigurationManager *> ::iterator it=userConfigurationManagers_.begin(); it!=userConfigurationManagers_.end(); ++it)
	{
		mf::LogDebug(__FILE__) << it->first << "     ";
		delete it->second;
		it->second = 0;
	}
	userConfigurationManagers_.clear();

}

//========================================================================================================================
ConfigurationGUISupervisor::~ConfigurationGUISupervisor(void)
{
	destroy();
}
//========================================================================================================================
void ConfigurationGUISupervisor::init(void)
{
	//called by constructor
	theSupervisorsConfiguration_.init(getApplicationContext());
}

//========================================================================================================================
void ConfigurationGUISupervisor::destroy(void)
{
	//called by destructor
	for (std::map<std::string, ConfigurationManager *> ::iterator it=userConfigurationManagers_.begin(); it!=userConfigurationManagers_.end(); ++it)
	{
		delete it->second;
		it->second = 0;
	}
	userConfigurationManagers_.clear();

	if( ConfigurationInterface::getInstance(true) != 0 )
		delete ConfigurationInterface::getInstance(true);
}

//========================================================================================================================
void ConfigurationGUISupervisor::Default(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	
	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/ConfigurationGUI.html?urn=" <<
			getenv("CONFIGURATION_GUI_SUPERVISOR_ID") <<"'></frameset></html>";

}

//========================================================================================================================
void ConfigurationGUISupervisor::request(xgi::Input * in, xgi::Output * out)
throw (xgi::exception::Exception)
{
	

	cgicc::Cgicc cgi(in);
	std::string Command;
	if((Command = CgiDataUtilities::postData(cgi,"RequestType")) == "")
		Command = cgi("RequestType"); //from GET or POST

	mf::LogDebug(__FILE__) << "Command " << Command << " files: " << cgi.getFiles().size() << "     ";

	//Commands
	//getSystemConfigurations
	//getSubSystemConfigurations
	//getSpecificSystemConfiguration
	//getSpecificSubSystemConfiguration
	//saveSpecificSubSystemConfiguration
	//changeKocVersionForSpecificConfig

	//**** start LOGIN GATEWAY CODE ***//
	//If TRUE, cookie code is good, and refreshed code is in cookieCode, also pointers optionally for uint8_t userPermissions
	//Else, error message is returned in cookieCode
	uint8_t userPermissions;
	std::string cookieCode;
	if((cookieCode = CgiDataUtilities::postData(cgi,"CookieCode")) == "")
		cookieCode = cgi("CookieCode"); //from GET or POST

	//comment to remove security
	bool AutomaticRefresh = 0;
	std::string userWithLock;
	if(!theRemoteWebUsers_.cookieCodeIsActiveForRequest(theSupervisorsConfiguration_.getSupervisorDescriptor(),
			cookieCode, &userPermissions, "0", !AutomaticRefresh, &userWithLock)) //only refresh cookie if not automatic refresh
	{
		*out << cookieCode;
		mf::LogDebug(__FILE__) << "Invalid Cookie Code" << "     ";
		return;
	}
	//**** end LOGIN GATEWAY CODE ***//

	//**** start LOCK GATEWAY CODE ***//
	std::string username = "";
	uint64_t activeSessionIndex;
	theRemoteWebUsers_.getUserInfoForCookie(theSupervisorsConfiguration_.getSupervisorDescriptor(),
			cookieCode, &username, 0, &activeSessionIndex);
	if(userWithLock != "" && userWithLock != username)
	{
		*out << RemoteWebUsers::REQ_USER_LOCKOUT_RESPONSE;
		mf::LogDebug(__FILE__) << "User " << username << " is locked out. " << userWithLock << " has lock." << "     ";
		return;
	}
	//**** end LOCK GATEWAY CODE ***//

	if(userPermissions < USER_PERMISSIONS_THRESHOLD)
	{
		*out << RemoteWebUsers::REQ_NO_PERMISSION_RESPONSE;
		mf::LogDebug(__FILE__) << "User " << username << " has insufficient permissions: " << userPermissions << "." << "     ";
		return;
	}

	HttpXmlDocument xmldoc(cookieCode);

	//acquire user's configuration manager based on username & activeSessionIndex

	std::string  backboneVersionStr = cgi("backboneVersion");		  	//from GET
	int		backboneVersion = (backboneVersionStr == "")?-1:atoi(backboneVersionStr.c_str()); //default to latest
	mf::LogDebug(__FILE__) << "ConfigurationManager backboneVersion Version req \t\t" << backboneVersionStr << "     ";
	ConfigurationManager* cfgMgr = refreshUserSession(username, activeSessionIndex, backboneVersion);
	mf::LogDebug(__FILE__) << "ConfigurationManager backboneVersion Version Loaded \t\t" << backboneVersion << "     ";

	char tmpIntStr[100];
	DOMElement* parentEl;

	//return this information always
	//<backboneVersion=xxx />
	sprintf(tmpIntStr,"%u",backboneVersion);
	xmldoc.addTextElementToData("backboneVersion", tmpIntStr); //add to response

	if(Command == "getSystemConfigurations")
	{
		//return this information
		//<configuration alias=xxx key=xxx>
		//	<koc alias=xxx key=xxx />
		//	<koc alias=xxx key=xxx />
		//	...
		//</configuration>
		//<configuration alias=xxx key=xxx>...</configuration>
		//...

		std::map<std::string, ConfigurationKey> aliasMap = cfgMgr->getConfiguration<ConfigurationAliases>()->getAliasesMap();
		std::map<std::string, ConfigurationKey>::const_iterator it = aliasMap.begin();

		std::set<std::string> listOfKocs;

		while (it != aliasMap.end())
		{
			//for each configuration alias and key
			//get KOC version numbers

			//mf::LogDebug(__FILE__) << "Alias: " << it->first << " - Key: " << it->second.key() << "     ";

			//add system configuration alias and key
			xmldoc.addTextElementToData("SystemConfigurationAlias", it->first);
			sprintf(tmpIntStr,"%u",it->second.key());
			xmldoc.addTextElementToData("SystemConfigurationKey", tmpIntStr);
			parentEl = xmldoc.addTextElementToData("SystemConfigurationKOCs", "");

			//get KOCs alias and version for the current system configuration key
			assert(cfgMgr->getConfiguration<Configurations>());
			{
				listOfKocs = cfgMgr->getConfiguration<Configurations>()->getListOfKocs(it->second.key());
				//mf::LogDebug(__FILE__) << "\tKocs size: " << listOfKocs.size() << "     ";

				for (std::set<std::string>::iterator sit=listOfKocs.begin(); sit!=listOfKocs.end(); ++sit)
				{
					unsigned int cv = cfgMgr->getConfiguration<Configurations>()->getConditionVersion(it->second.key(),*sit);

					//mf::LogDebug(__FILE__) << "\tKoc: " << *sit << " Version: " << cv << "     ";

					xmldoc.addTextElementToParent("KOC_alias", *sit, parentEl);
					sprintf(tmpIntStr,"%u",cv);
					xmldoc.addTextElementToParent("KOC_version", tmpIntStr, parentEl);
				}
			}
			++it;
		}

	}
	else if(Command == "getSubSystemConfigurations")
	{
		//return this information
		//<subconfiguration alias=xxx>
		//	<version key=xxx />
		//	<version key=xxx />
		//	...
		//</subconfiguration>
		//<subconfiguration alias=xxx>...</subconfiguration>
		//...

		std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
		std::map<std::string, ConfigurationInfo>::const_iterator it = allCfgInfo.begin();

		while(it != allCfgInfo.end())
		{
			//for each subconfiguration alias
			//get existing version keys

			//mf::LogDebug(__FILE__) << "Alias: " << it->first << " - Key: " << it->second.key() << "     ";

			//add system subconfiguration alias
			xmldoc.addTextElementToData("SystemSubConfigurationAlias", it->first);
			parentEl = xmldoc.addTextElementToData("SystemSubConfigurationVersions", "");

			//get version key for the current system subconfiguration key
			for (std::set<int>::iterator vit=it->second.versions_.begin(); vit!=it->second.versions_.end(); ++vit)
			{
				//mf::LogDebug(__FILE__) << "\t\t" << *vit << "     ";

				sprintf(tmpIntStr,"%d",*vit);
				xmldoc.addTextElementToParent("VersionKey", tmpIntStr, parentEl);
			}

			++it;
		}
	}
	else if(Command == "getSpecificSystemConfiguration")
	{
		//give the detail of specific System Configuration specified

		//Find historical alias keys by
		//loading all historical configurations
		//figure out all configurations versions
		//open file, and check for alias name and associated extract key

		//return this information
		//<configuration alias=xxx key=xxx>
		//	<historical key=xxx>
		//	<historical key=xxx>
		//	....
		//	<koc alias=xxx key=xxx>
		//		<version key=xxx>
		//		<version key=xxx>
		//		...
		//	</koc>
		//	<koc alias=xxx key=xxx>
		//	...
		//</configuration>

		std::string alias = cgi("alias"); //from GET

		mf::LogDebug(__FILE__) << "getSpecificSystemConfiguration: " << alias << "     ";

		std::map<std::string, ConfigurationKey> aliasMap = cfgMgr->getConfiguration<ConfigurationAliases>()->getAliasesMap();

		std::map<std::string, ConfigurationKey>::const_iterator it = aliasMap.find(alias);
		if(it != aliasMap.end())
		{
			xmldoc.addTextElementToData("SystemConfigurationAlias", it->first);

			std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
			std::set<int> versions;

			std::set<std::string> listOfKocs;

			DOMElement* parentElKoc;

			//get all KOC alias and version numbers

			//mf::LogDebug(__FILE__) << "Alias: " << it->first << " - Key: " << it->second.key() << "     ";

			//add system configuration alias and key
			xmldoc.addTextElementToData("SystemConfigurationAlias", it->first);
			sprintf(tmpIntStr,"%u",it->second.key());
			xmldoc.addTextElementToData("SystemConfigurationKey", tmpIntStr);
			parentEl = xmldoc.addTextElementToData("SystemConfigurationKOCs", "");

			//get KOCs alias and version for the current system configuration key
			assert(cfgMgr->getConfiguration<Configurations>());
			{
				listOfKocs = cfgMgr->getConfiguration<Configurations>()->getListOfKocs(it->second.key());
				//mf::LogDebug(__FILE__) << "\tKocs size: " << listOfKocs.size() << "     ";

				for (std::set<std::string>::iterator sit=listOfKocs.begin(); sit!=listOfKocs.end(); ++sit)
				{
					//current version
					unsigned int cv = cfgMgr->getConfiguration<Configurations>()->getConditionVersion(it->second.key(),*sit);

					//all existing versions
					versions = allCfgInfo.find(*sit)->second.versions_;

					//mf::LogDebug(__FILE__) << "\tKoc: " << *sit << " Version: " << cv << "     ";

					xmldoc.addTextElementToParent("KOC_alias", *sit, parentEl);
					sprintf(tmpIntStr,"%u",cv);
					parentElKoc = xmldoc.addTextElementToParent("KOC_currentVersion", tmpIntStr, parentEl);
					for (std::set<int>::iterator vit=versions.begin(); vit!=versions.end(); ++vit)
					{
						//mf::LogDebug(__FILE__) << "\t\t" << *vit << "     ";
						if(*vit == (int)cv) continue;
						sprintf(tmpIntStr,"%d",*vit);
						xmldoc.addTextElementToParent("KOC_existingVersion", tmpIntStr, parentElKoc);
					}
				}
			}


			//check for other existing backbone versions - they all (3 backbone pieces) better match!!
			//	(so we can just check one of them to get the list of historical versions: Configurations)
			//  (The 3 backbone pieces are Configurations -- ConfigurationAliases -- DefaultConfigurations

			versions = allCfgInfo.find(Configurations::staticConfigurationName_)->second.versions_;

			for (std::set<int>::iterator vit=versions.begin(); vit!=versions.end(); ++vit)
			{
				mf::LogDebug(__FILE__) << "Configurations Version \t\t" << *vit << "     ";
				if(*vit != backboneVersion)
				{
					//found a different configurations version then current version:
					//load it
					//check for alias

					cfgMgr->loadConfigurationBackbone(*vit);

					std::map<std::string, ConfigurationKey> aliasMap = cfgMgr->getConfiguration<ConfigurationAliases>()->getAliasesMap();

					it = aliasMap.find(alias);
					if(it != aliasMap.end())
					{
						//found a historical version of alias
						sprintf(tmpIntStr,"%u",it->second.key());
						xmldoc.addTextElementToData("HistoricalSystemConfigurationKey", tmpIntStr);

					}
				}
			}
		}
	}
	else if(Command == "getSpecificSubSystemConfiguration")
	{
		//give the detail of specific Sub-System Configuration specified
		//	by subAlias and version

		//return existing versions
		//return column headers
		//return number of rows
		//from dataOffset
		//first CHUNK_SIZE rows

		//TODO!!
		//return this information
		//<subconfiguration alias=xxx version=xxx rowCount=xxx chunkReq=xxx chunkSz=xxx>
		//	<existing version=xxx>
		//	<existing version=xxx>
		//	....
		//	<colhdr alias=xxx>
		//	<colhdr alias=xxx>
		//	....
		//	<rowdata>
		//		<cell value=xxx>
		//		<cell value=xxx>
		//		....
		//	</rowdata>
		//	<rowdata>
		//		....
		//	</rowdata>
		//	....
		//</subconfiguration>

		std::string 	subAlias = cgi("subAlias"); 			//from GET
		std::string  versionStr = cgi("version");		  	//from GET
		int		version = (versionStr == "")?0:atoi(versionStr.c_str());
		int		dataOffset = atoi(cgi("dataOffset").c_str());	//from GET
		int		chunkSize = atoi(cgi("chunkSize").c_str());	//from GET

		mf::LogDebug(__FILE__) << "getSpecificSubSystemConfiguration: " << subAlias << " version: " << version
				<< " chunkSize: " << chunkSize << " dataOffset: " << dataOffset << "     ";

		//verify alias and version exists
		std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
		std::map<std::string, ConfigurationInfo>::const_iterator it = allCfgInfo.find(subAlias);

		if(it == allCfgInfo.end())
		{
			mf::LogDebug(__FILE__) << "SubSystemConfiguration not found" << "     ";
		}
		else if(it->second.versions_.find(version) == it->second.versions_.end())
		{
			mf::LogDebug(__FILE__) << "Version not found" << "     ";
		}
		else
		{
			//load current version
			bool isInConfiguration = (allCfgInfo[subAlias].configurationPtr_->isStored(version));
			mf::LogDebug(__FILE__) << "Version " << version << " is loaded: " <<
					(isInConfiguration?"YES":"NO") << "     ";

			if(!isInConfiguration) //load configuration view
				cfgMgr->getVersionedConfigurationByName(subAlias, version);
			else
				allCfgInfo[subAlias].configurationPtr_->setActiveView(version);

			isInConfiguration = (allCfgInfo[subAlias].configurationPtr_->isStored(version));
			mf::LogDebug(__FILE__) << "Version " << version << " is loaded: " <<
					(isInConfiguration?"YES":"NO") << "     ";

			if(!isInConfiguration)
			{
				mf::LogDebug(__FILE__) << "Version could not be loaded" << "     ";
			}
			else
			{

				xmldoc.addTextElementToData("SubSystemConfigurationAlias", it->first);
				sprintf(tmpIntStr,"%d",version);
				xmldoc.addTextElementToData("SubSystemConfigurationVersion", tmpIntStr);
				parentEl = xmldoc.addTextElementToData("SubSystemConfigurationVersions", "");
				for (std::set<int>::iterator vit=it->second.versions_.begin(); vit!=it->second.versions_.end(); ++vit)
				{
					sprintf(tmpIntStr,"%d",*vit);
					xmldoc.addTextElementToParent("VersionKey", tmpIntStr, parentEl);
				}

				//view version
				{
					//get 'columns' of sub config

					mf::LogDebug(__FILE__) << "\t\t******** view " <<
							allCfgInfo[subAlias].configurationPtr_->getViewVersion() << "     ";
					ConfigurationView* cfgViewPtr = allCfgInfo[subAlias].configurationPtr_->getViewP();

					parentEl = xmldoc.addTextElementToData("CurrentVersionColumnHeaders", "");
					std::vector<ViewColumnInfo> colInfo = cfgViewPtr->getColumnsInfo();
					for(int i=0;i<(int)colInfo.size();++i)	//column headers and types
					{
						mf::LogDebug(__FILE__) << "\t\tCol " << i << ": " << colInfo[i].getName() << " "
								<< colInfo[i].getViewName() << " " << colInfo[i].getViewType() << "     ";

						xmldoc.addTextElementToParent("ColumnHeader", colInfo[i].getName(), parentEl);
						xmldoc.addTextElementToParent("ColumnType", colInfo[i].getViewType(), parentEl);
					}

					parentEl = xmldoc.addTextElementToData("CurrentVersionRows", "");

					for(int r=0;r<(int)cfgViewPtr->getNumberOfRows();++r)
					{
						//mf::LogDebug(__FILE__) << "\t\tRow " << r << ": " ;

						sprintf(tmpIntStr,"%d",r);
						DOMElement* tmpParentEl = xmldoc.addTextElementToParent("Row", tmpIntStr, parentEl);

						for(int c=0;c<(int)cfgViewPtr->getNumberOfColumns();++c)
							if(colInfo[c].getViewType() == "NUMBER")
							{
								int num;
								cfgViewPtr->getValue(num,r,c);
								//mf::LogDebug(__FILE__) << "\t " << num;

								sprintf(tmpIntStr,"%d",num);
								xmldoc.addTextElementToParent("Entry", tmpIntStr, tmpParentEl);
							}
							else
							{
								std::string val;
								cfgViewPtr->getValue(val,r,c);
								//mf::LogDebug(__FILE__) << "\t " << val;

								xmldoc.addTextElementToParent("Entry", val, tmpParentEl);
							}
						//
					}
				}
			}


		}


	}
	else if(Command == "saveSpecificSubSystemConfiguration")
	{
		mf::LogDebug(__FILE__) << "saveSpecificSubSystemConfiguration" << "     ";
		//TODO
		// CHECK MUST HAVE LOCK!!

		//save the detail of specific Sub-System Configuration specified
		//	by subAlias and version

		//starting from dataOffset
		//save first CHUNK_SIZE rows
		std::string 	subAlias = cgi("subAlias"); 			//from GET
		int		version = atoi(cgi("version").c_str());	//from GET
		int		dataOffset = atoi(cgi("dataOffset").c_str());	//from GET
		int		chunkSize = atoi(cgi("chunkSize").c_str());	//from GET

		std::string	data = CgiDataUtilities::postData(cgi,"data"); //from POST
		//data format: commas and semi-colons indicate new row
		//r0c0,r0c1,...,r0cN,;r1c0,...



		//verify alias and version exists
		std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
		std::map<std::string, ConfigurationInfo>::const_iterator it = allCfgInfo.find(subAlias);


		mf::LogDebug(__FILE__) << "getSpecificSubSystemConfiguration: " << subAlias << " version: " << version
				<< " chunkSize: " << chunkSize << " dataOffset: " << dataOffset << "     ";

		mf::LogDebug(__FILE__) << "data: " << data << "     ";

		//FIXME CHECK MUST HAVE LOCK!!
		// CHECK MUST HAVE LOCK!!
		// CHECK MUST HAVE LOCK!!
		// CHECK MUST HAVE LOCK!!
		// CHECK MUST HAVE LOCK!!
		if(it == allCfgInfo.end())
		{
			mf::LogDebug(__FILE__) << "SubSystemConfiguration not found" << "     ";

			xmldoc.addTextElementToData("Error", "SubSystemConfiguration not found");
		}
		else if(it->second.versions_.find(version) == it->second.versions_.end())
		{
			mf::LogDebug(__FILE__) << "Version not found" << "     ";

			xmldoc.addTextElementToData("Error", "Version not found");
		}
		else
		{
			//load current version
			bool isInConfiguration = (allCfgInfo[subAlias].configurationPtr_->isStored(version));
			mf::LogDebug(__FILE__) << "Version " << version << " is loaded: " <<
					(isInConfiguration?"YES":"NO") << "     ";

			if(!isInConfiguration) //load configuration view
				cfgMgr->getVersionedConfigurationByName(subAlias, version);
			else
				allCfgInfo[subAlias].configurationPtr_->setActiveView(version);

			isInConfiguration = (allCfgInfo[subAlias].configurationPtr_->isStored(version));
			mf::LogDebug(__FILE__) << "Version " << version << " is loaded: " <<
					(isInConfiguration?"YES":"NO") << "     ";

			if(!isInConfiguration)
			{
				mf::LogDebug(__FILE__) << "Version could not be loaded" << "     ";

				xmldoc.addTextElementToData("Error", "Version could not be loaded");
			}
			else
			{
				int temporaryVersion = allCfgInfo[subAlias].configurationPtr_->createTemporaryView(version);

				mf::LogDebug(__FILE__) << "\t\ttemporaryVersion: " << temporaryVersion << "     ";
				ConfigurationView* cfgViewPtr = allCfgInfo[subAlias].configurationPtr_->getTemporaryView(temporaryVersion);
				std::vector<ViewColumnInfo> colInfo = cfgViewPtr->getColumnsInfo();

				//while there are row entries in the data.. replace
				// data range from [dataOffset, dataOffset+chunkSize-1]
				// ... note if less rows, this means rows were deleted
				// ... if more, then rows were added.
				int r = dataOffset;
				int c = 0;

				//int cellNum;
				//string cellStr;

				int i = 0; //use to parse data std::string
				int j = data.find(',',i); //find next cell delimiter
				int k = data.find(';',i); //find next row delimiter

				while(k != (int)(std::string::npos))
				{
					if(r >= (int)cfgViewPtr->getNumberOfRows())
					{
						cfgViewPtr->addRow();
						mf::LogDebug(__FILE__) << "Row added" << "     ";
					}

					while(j < k && j != (int)(std::string::npos))
					{
						//mf::LogDebug(__FILE__) << r << "|" << c << "][" << i << "|" << k << "][";
						//mf::LogDebug(__FILE__) << data.substr(i,j-i) << "|";
						if(colInfo[c].getViewType() == "NUMBER")
						{
							//mf::LogDebug(__FILE__) << atoi(data.substr(i,j-i).c_str()) << "|";
							cfgViewPtr->setValue(atoi(data.substr(i,j-i).c_str()),r,c);
							//cfgViewPtr->getValue(cellNum,r,c);
							//mf::LogDebug(__FILE__) << cellNum << " ";
						}
						else
						{
							//mf::LogDebug(__FILE__) << data.substr(i,j-i) << "|";
							cfgViewPtr->setValue(data.substr(i,j-i),r,c);
							//cfgViewPtr->getValue(cellStr,r,c);
							//mf::LogDebug(__FILE__) << cellStr << " ";
						}
						i=j+1;
						j = data.find(',',i); //find next cell delimiter
						++c;
					}
					++r;
					c = 0;
					

					i = k+1;
					j = data.find(',',i); //find next cell delimiter
					k = data.find(';',i); //find new row delimiter
				}

				//delete excess rows
				while(r < (int)cfgViewPtr->getNumberOfRows())
				{
					cfgViewPtr->deleteRow(r);
					mf::LogDebug(__FILE__) << "Row deleted" << "     ";
				}


				mf::LogDebug(__FILE__) << "\t\t**************************** Save as new sub-config version" << "     ";

				int newAssignedVersion = cfgMgr->saveNewConfiguration(allCfgInfo[subAlias].configurationPtr_,temporaryVersion);

				xmldoc.addTextElementToData("savedAlias", subAlias);
				sprintf(tmpIntStr,"%d",newAssignedVersion);
				xmldoc.addTextElementToData("savedVersion", tmpIntStr);

				mf::LogDebug(__FILE__) << "\t\t newAssignedVersion: " << newAssignedVersion << "     ";

			}
		}
	}
	else if(Command == "changeKocVersionForSpecificConfig")
	{
		mf::LogDebug(__FILE__) << "changeKocVersionForSpecificConfig" << "     ";
		//TODO
	}
	else
		mf::LogDebug(__FILE__) << "Command request not recognized." << "     ";



	//return xml doc holding server response
	xmldoc.outputXmlDocument((std::ostringstream*) out, false);
}

//========================================================================================================================
//	refreshUserSession
//		Finds/creates the active user session based on username & actionSessionIndex
//
//		Returns a configurationMangager instance dedictated to the user.
//		This configurationManager will have at least empty instances of all base configurations (no null pointers)
//		and will load the backbone configurations to specified backboneVersion
//
//		If backboneVersion is -1, then latest, and backboneVersion passed by reference will be updated
ConfigurationManager* ConfigurationGUISupervisor::refreshUserSession(std::string username, uint64_t activeSessionIndex, int &backboneVersion)
{
	std::stringstream ssMapKey;
	ssMapKey << username << ":" << activeSessionIndex;
	std::string mapKey = ssMapKey.str();
	mf::LogDebug(__FILE__) << mapKey << " ... current size: " << userConfigurationManagers_.size() << "     ";
	//create new config mgr if not one for active session index
	if(userConfigurationManagers_.find(mapKey) == userConfigurationManagers_.end())
	{
		userConfigurationManagers_[mapKey] = new ConfigurationManager();

		//update configuration info for each new configuration manager
		//	IMPORTANTLY this also fills all configuration manager pointers with instances,
		//	so we are not dealing with changing pointers later on
		userConfigurationManagers_[mapKey]->getAllConfigurationInfo();	//load empty instance of everything important
	}

	//load backbone configurations always based on backboneVersion
	//if backboneVersion is -1, then latest
	backboneVersion = userConfigurationManagers_[mapKey]->loadConfigurationBackbone(backboneVersion);

	time_t now = time(0);
	//update active sessionIndex last use time
	userLastUseTime_[mapKey] = now;

	//check for stale sessions and remove them (so config user maps do not grow forever)
	for (std::map<std::string, time_t> ::iterator it=userLastUseTime_.begin(); it!=userLastUseTime_.end(); ++it)
		if(now - it->second > CONFIGURATION_MANAGER_EXPIRATION_TIME) // expired!
		{
			mf::LogDebug(__FILE__) << now << ":" << it->second << " = " << now - it->second << "     ";
			delete userConfigurationManagers_[it->first]; //call destructor
			assert(userConfigurationManagers_.erase(it->first));	//erase by key
			userLastUseTime_.erase(it);								//erase by iterator

			it=userLastUseTime_.begin(); //fail safe.. reset it, to avoid trying to understand what happens with the next iterator
		}

	return userConfigurationManagers_[mapKey];
}
