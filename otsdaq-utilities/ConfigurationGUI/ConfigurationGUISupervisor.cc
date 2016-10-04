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

#include "otsdaq-core/ConfigurationDataFormats/ROCDACs.h"


#include <xdaq/NamespaceURI.h>

#include <iostream>
#include <map>
#include <utility>



using namespace ots;

#undef 	__MF_SUBJECT__
#define __MF_SUBJECT__ "CfgGUI"


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

	std::cout << __COUT_HDR_FL__ << "Initializing..." << std::endl;
	init();


	//new user gets a config mgr assigned
	//user can fill any of the sub-configs (fill from version or init empty), which becomes the active view for that sub-config
	//if user then edits one of the sub-configs, just before editing, active view is copied to edit view which is version -1
	//if the user saves the configuration, then any sub-configurations with active view -1 get saved with a new version number


	//how do we know which version numbers of a KOC exist already?
	//getAllConfigurationInfo fills the version list for each KOC found in "Configurations"

	//FIXME TODO
	//LEFT OFF - Proof of Concept
	//	Choose a sub-config
	//	Make a copy of a view
	//	Edit copy
	//	Save as new sub-config version



	__MOUT__ << "comment/uncomment here for debugging Configuration!" << std::endl;

	__MOUT__ << "To prove the concept..." << std::endl;

	return;

	testXDAQContext(); //test new config

	return;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//behave like a new user

	std::string userConfigurationManagerIndex = "1";
	userConfigurationManagers_["1"] = new ConfigurationManagerRW("ExampleUser1");
	userConfigurationManagers_["2"] = new ConfigurationManagerRW("ExampleUser2");
	ConfigurationManagerRW *cfgMgr = userConfigurationManagers_[userConfigurationManagerIndex];

	std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
	__MOUT__ << "All config info loaded." << std::endl;
	std::map<std::string, ConfigurationGroupKey>	aliasMap = cfgMgr->__GET_CONFIG__(ConfigurationAliases)->getAliasesMap();


	__MOUT__ << "aliasMap size: " << aliasMap.size() << std::endl;
	__MOUT__ << "getAllConfigurationInfo size: " << allCfgInfo.size() << std::endl;


	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//for each configuration alias and key
	//get KOC version numbers (this is the version "conditioned" by the alias-key pair)

	std::set<std::string> listOfKocs;
	std::map<std::string, ConfigurationGroupKey>::const_iterator it = aliasMap.begin();
	while (it != aliasMap.end())
	{

		__MOUT__ << "Alias: " << it->first << " - Key: " << it->second.key() << std::endl;

		listOfKocs = cfgMgr->__GET_CONFIG__(Configurations)->getListOfKocs(it->second);
		__MOUT__ << "\tKocs size: " << listOfKocs.size() << std::endl;

		for (auto& koc : listOfKocs)
		{
			ConfigurationVersion conditionVersion = cfgMgr->__GET_CONFIG__(Configurations)->getConditionVersion(it->second,koc);

			__MOUT__ << "\tKoc: " << koc << " Version: " << conditionVersion << std::endl;
		}
		++it;

	}
	//return;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//For each existing KOC (comes from anything in "Configurations")
	//get existing versions

	auto mapIt = allCfgInfo.begin();
	while(mapIt != allCfgInfo.end())
	{
		__MOUT__ << "KOC Alias: " << mapIt->first << std::endl;
		__MOUT__ << "\t\tExisting Versions: " << mapIt->second.versions_.size() << std::endl;

		//get version key for the current system subconfiguration key
		for(auto &version:mapIt->second.versions_)
			__MOUT__ << "\t\t" << version << std::endl;
		++mapIt;
	}
	//return;


	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//Choose a sub config
	mf::LogDebug("cfgGUI");
	__MOUT__ << "\t\t**************************** Choose a sub config" << std::endl;
	std::string chosenSubConfig = "FSSRDACsConfiguration"; //must be less than allCfgInfo.size()

	{
		ConfigurationVersion versionToCopy(ConfigurationVersion::DEFAULT); //-1 is empty, //must be less than allCfgInfo[chosenSubConfig].versions_.size()

		//check if is an existing version
		bool isInDatabase = allCfgInfo[chosenSubConfig].versions_.find(versionToCopy) != allCfgInfo[chosenSubConfig].versions_.end();
		__MOUT__ << "Version " << versionToCopy << " is in database: " <<
				(isInDatabase?"YES":"NO") << std::endl;

		//check if version is already loaded
		bool isInConfiguration = (allCfgInfo[chosenSubConfig].configurationPtr_->isStored(versionToCopy));
		__MOUT__ << "Version " << versionToCopy << " is loaded: " <<
				(isInConfiguration?"YES":"NO") << std::endl;

		//load configuration view and set as active view
		if(!isInConfiguration)
			cfgMgr->getVersionedConfigurationByName(chosenSubConfig, versionToCopy);
		else
			allCfgInfo[chosenSubConfig].configurationPtr_->setActiveView(versionToCopy);

		//verify version is loaded now
		isInConfiguration = (allCfgInfo[chosenSubConfig].configurationPtr_->isStored(versionToCopy));
		__MOUT__ << "Version " << versionToCopy << " is loaded: " <<
				(isInConfiguration?"YES":"NO") << std::endl;
	}

	//return;


	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//view this version
	//general base class
	{
		//get 'columns' of sub config

		__MOUT__ << "\t\t******** view version " <<
				allCfgInfo[chosenSubConfig].configurationPtr_->getViewVersion() << std::endl;
		ConfigurationView* cfgViewPtr = allCfgInfo[chosenSubConfig].configurationPtr_->getViewP();

		std::vector<ViewColumnInfo> colInfo = cfgViewPtr->getColumnsInfo();

		__MOUT__ << "\t\tNumber of Cols " << colInfo.size() << std::endl;
		__MOUT__ << "\t\tNumber of Rows " << cfgViewPtr->getNumberOfRows() << std::endl;
	}

	//return;


	__MOUT__ << "\t\t**************************** Choose a different sub config" << std::endl;

	{
		ConfigurationVersion versionToCopy(2); //-1 is empty, //must be less than allCfgInfo[chosenSubConfig].versions_.size()

		//check if is an existing version
		bool isInDatabase = allCfgInfo[chosenSubConfig].versions_.find(versionToCopy) != allCfgInfo[chosenSubConfig].versions_.end();
		__MOUT__ << "Version " << versionToCopy << " is in database: " <<
				(isInDatabase?"YES":"NO") << std::endl;

		//check if version is already loaded
		bool isInConfiguration = (allCfgInfo[chosenSubConfig].configurationPtr_->isStored(versionToCopy));
		__MOUT__ << "Version " << versionToCopy << " is loaded: " <<
				(isInConfiguration?"YES":"NO") << std::endl;

		//load configuration view and set as active view
		if(!isInConfiguration)
			cfgMgr->getVersionedConfigurationByName(chosenSubConfig, versionToCopy);
		else
			allCfgInfo[chosenSubConfig].configurationPtr_->setActiveView(versionToCopy);

		//verify version is loaded now
		isInConfiguration = (allCfgInfo[chosenSubConfig].configurationPtr_->isStored(versionToCopy));
		__MOUT__ << "Version " << versionToCopy << " is loaded: " <<
				(isInConfiguration?"YES":"NO") << std::endl;

	}

	{
		//get 'columns' of sub config

		__MOUT__ << "\t\t******** view version " <<
				allCfgInfo[chosenSubConfig].configurationPtr_->getViewVersion() << std::endl;
		ConfigurationView* cfgViewPtr = allCfgInfo[chosenSubConfig].configurationPtr_->getViewP();

		std::vector<ViewColumnInfo> colInfo = cfgViewPtr->getColumnsInfo();

		__MOUT__ << "\t\tNumber of Cols " << colInfo.size() << std::endl;
		__MOUT__ << "\t\tNumber of Rows " << cfgViewPtr->getNumberOfRows() << std::endl;
	}

	//return;

	__MOUT__ << "\t\t**************************** Choose a different sub config" << std::endl;

	{
		ConfigurationVersion versionToCopy(ConfigurationVersion::DEFAULT); //-1 is empty, //must be less than allCfgInfo[chosenSubConfig].versions_.size()
		//check if is an existing version
		bool isInDatabase = allCfgInfo[chosenSubConfig].versions_.find(versionToCopy) != allCfgInfo[chosenSubConfig].versions_.end();
		__MOUT__ << "Version " << versionToCopy << " is in database: " <<
				(isInDatabase?"YES":"NO") << std::endl;

		//check if version is already loaded
		bool isInConfiguration = (allCfgInfo[chosenSubConfig].configurationPtr_->isStored(versionToCopy));
		__MOUT__ << "Version " << versionToCopy << " is loaded: " <<
				(isInConfiguration?"YES":"NO") << std::endl;

		//load configuration view and set as active view
		if(!isInConfiguration)
			cfgMgr->getVersionedConfigurationByName(chosenSubConfig, versionToCopy);
		else
			allCfgInfo[chosenSubConfig].configurationPtr_->setActiveView(versionToCopy);

		//verify version is loaded now
		isInConfiguration = (allCfgInfo[chosenSubConfig].configurationPtr_->isStored(versionToCopy));
		__MOUT__ << "Version " << versionToCopy << " is loaded: " <<
				(isInConfiguration?"YES":"NO") << std::endl;
	}

	{
		//get 'columns' of sub config

		__MOUT__ << "\t\t******** view version " <<
				allCfgInfo[chosenSubConfig].configurationPtr_->getViewVersion() << std::endl;
		ConfigurationView* cfgViewPtr = allCfgInfo[chosenSubConfig].configurationPtr_->getViewP();

		std::vector<ViewColumnInfo> colInfo = cfgViewPtr->getColumnsInfo();

		__MOUT__ << "\t\tNumber of Cols " << colInfo.size() << std::endl;
		__MOUT__ << "\t\tNumber of Rows " << cfgViewPtr->getNumberOfRows() << std::endl;
	}

	//return;

	ConfigurationVersion versionToCopy(ConfigurationVersion::DEFAULT);
	//	Make a copy of a view
	__MOUT__ << "\t\t**************************** Make a copy of a view" << std::endl;
	ConfigurationVersion temporaryVersion = allCfgInfo[chosenSubConfig].configurationPtr_->createTemporaryView(versionToCopy);
	__MOUT__ << "\t\ttemporaryVersion: " << temporaryVersion << std::endl;


	//	Edit copy
	__MOUT__ << "\t\t**************************** Edit copy" << std::endl;

	//view temp version
	//general base class
	{
		//get 'columns' of sub config

		__MOUT__ << "\t\t******** Before change" << std::endl;
		ConfigurationView* cfgViewPtr = allCfgInfo[chosenSubConfig].configurationPtr_->getTemporaryView(temporaryVersion);

		std::vector<ViewColumnInfo> colInfo = cfgViewPtr->getColumnsInfo();

		std::stringstream ss;
		cfgViewPtr->print(ss);
		__MOUT__ << ss.str() << std::endl;


		//edit something

		// add a new row for example

		int r = cfgViewPtr->addRow();
		//set all numbers to 10
		r = cfgViewPtr->addRow();
		for(int c=0;c<(int)cfgViewPtr->getNumberOfColumns();++c)
			if(colInfo[c].getDataType() == "NUMBER")
				cfgViewPtr->setValue(c,r,c);
			else
				cfgViewPtr->setValue(std::string("Hi"),r,c);
		cfgViewPtr->addRow();

		//after change
		__MOUT__ << "\t\t******** After change" << std::endl;



		std::stringstream ss2;
		cfgViewPtr->print(ss2);
		__MOUT__ << ss2.str() << std::endl;


		// add a new col for example
		//		int cret = cfgViewPtr->addColumn("TestCol","TEST_COL","TIMESTAMP WITH TIMEZONE");
		//		__MOUT__ << "\t\t******** After change col: " << cret << std::endl;
		//		colInfo = cfgViewPtr->getColumnsInfo();
		//		__MOUT__ << "\t\tNumber of Cols " << colInfo.size() << std::endl;
		//		__MOUT__ << "\t\tNumber of Rows " << cfgViewPtr->getNumberOfRows() << std::endl;
		//
		//		for(int r=0;r<(int)cfgViewPtr->getNumberOfRows();++r)
		//		{
		//			__MOUT__ << "\t\tRow " << r << ": "  << std::endl;
		//
		//			for(int c=0;c<(int)cfgViewPtr->getNumberOfColumns();++c)
		//				if(colInfo[c].getViewType() == "NUMBER")
		//				{
		//					int num;
		//					cfgViewPtr->getValue(num,r,c);
		//					__MOUT__ << "\t " << num << std::endl;
		//				}
		//				else
		//				{
		//				std::string val;
		//					cfgViewPtr->getValue(val,r,c);
		//					__MOUT__ << "\t " << val << std::endl;
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
	//			__MOUT__ << "\t\tDac Name: " << rdit->first << " - Addr: " << rdit->second.first << ", Val: " << rdit->second.second << std::endl;
	//			rocDacs.setDAC(rdit->first,7,1);
	//
	//			rdit++;
	//		}
	//	}

	//	Save as new sub-config version
	__MOUT__ << "\t\t**************************** Save as new sub-config version" << std::endl;

	ConfigurationVersion newAssignedVersion(ConfigurationVersion::DEFAULT);//cfgMgr->saveNewConfiguration(allCfgInfo[chosenSubConfig].configurationPtr_,temporaryVersion);
	//int newAssignedVersion = cfgMgr->saveNewConfiguration(allCfgInfo[chosenSubConfig].configurationPtr_,temporaryVersion);

	__MOUT__ << "\t\t newAssignedVersion: " << newAssignedVersion << std::endl;

	//return;



	//proof of concept.. change a KOC version (this makes a new backbone version?!)
	__MOUT__ << "\t\t**************************** Edit a KOC for a System Alias" << std::endl;

	std::stringstream ss;
	cfgMgr->__GET_CONFIG__(ConfigurationAliases)->print(ss);
	cfgMgr->__GET_CONFIG__(DefaultConfigurations)->print(ss);
	cfgMgr->__GET_CONFIG__(Configurations)->print(ss);
	cfgMgr->__GET_CONFIG__(VersionAliases)->print(ss);
	__MOUT__ << ss.str() << std::endl;

	{
		std::string groupName = "Physics";
		std::string KOCAlias = "FSSRDACsConfiguration";
		ConfigurationVersion newVersion(3);

		std::map<std::string, ConfigurationGroupKey> aliasMap = cfgMgr->__GET_CONFIG__(ConfigurationAliases)->getAliasesMap();

		std::map<std::string, ConfigurationGroupKey>::const_iterator it = aliasMap.find(groupName);
		if(it != aliasMap.end())
		{
			__MOUT__ << "Alias: " << it->first << " - Key: " << it->second.key() << std::endl;

			__MOUT__ << "Alias exists: " << groupName << std::endl;
			__MOUT__ << "Sub system alias: " << KOCAlias << std::endl;
			__MOUT__ << "Changing to new version: " << newVersion << std::endl;

			std::set<std::string> listOfKocs;
			listOfKocs = cfgMgr->__GET_CONFIG__(Configurations)->getListOfKocs(it->second);
			for (std::set<std::string>::iterator sit=listOfKocs.begin(); sit!=listOfKocs.end(); ++sit)
			{
				ConfigurationVersion cv = cfgMgr->__GET_CONFIG__(Configurations)->getConditionVersion(it->second,*sit);

				__MOUT__ << "\tKoc: " << *sit << " Version: " << cv << std::endl;

				std::set<ConfigurationVersion> versions = allCfgInfo.find(*sit)->second.versions_;
				__MOUT__ << "\t\tAll versions: " << std::endl;
				for (auto &version:versions)
					__MOUT__ << " " << version << std::endl;

			}


			//Make copy for all backbone members to temporary versions!
			__MOUT__ << "\t\t**************************** Make temporary backbone" << std::endl;

			chosenSubConfig = "Configurations";
			ConfigurationVersion temporaryVersion;
			ConfigurationVersion versionToCopy = cfgMgr->__GET_CONFIG__(ConfigurationAliases)->getViewVersion();
			//cfgMgr->__GET_CONFIG__(DefaultConfigurations)->print();
			///////////////////////cfgMgr->__GET_CONFIG__(Configurations)->print();
			//cfgMgr->__GET_CONFIG__(VersionAliases)

			__MOUT__ << "\t\ttemporaryVersion versionToCopy: " << versionToCopy << std::endl;

			assert(allCfgInfo.find(chosenSubConfig) != allCfgInfo.end());

			bool isInDatabase = allCfgInfo[chosenSubConfig].versions_.find(versionToCopy) != allCfgInfo[chosenSubConfig].versions_.end();
			__MOUT__ << "Version " << versionToCopy << " is in database: " <<
					(isInDatabase?"YES":"NO") << std::endl;

			//temporaryVersion = allCfgInfo[chosenSubConfig].configurationPtr_->createTemporaryView(versionToCopy);
			temporaryVersion = cfgMgr->createTemporaryBackboneView(versionToCopy);
			__MOUT__ << "\t\ttemporaryVersion Backbone: " << temporaryVersion << std::endl;
			__MOUT__ << "\t\t(Note: it is not the) active version Backbone: " <<
					cfgMgr->__GET_CONFIG__(ConfigurationAliases)->getViewVersion()  << std::endl;


			//edit the backbone however you want
			__MOUT__ << "\t\t**************************** Make changes to backbone" << std::endl;

			ConfigurationView* cfgViewPtr = allCfgInfo[chosenSubConfig].configurationPtr_->getTemporaryView(temporaryVersion);


			__MOUT__ << "\t\t******** Before change" << std::endl;
			std::stringstream ss;
			cfgViewPtr->print(ss);
			__MOUT__ << ss.str() << std::endl;


			cfgMgr->setKOCVersionForSpecificConfiguration(allCfgInfo,temporaryVersion,groupName,KOCAlias,newVersion);

			__MOUT__ << "\t\t******** After change" << std::endl;
			ss.str(""); //clear stringstream (note: clear() just clears error state)
			cfgViewPtr->print(ss);
			__MOUT__ << ss.str() << std::endl;

			//Save temporary backbone view to new version
			__MOUT__ << "\t\t******** Saving new version" << std::endl;
			ConfigurationVersion newBbVersion = cfgMgr->saveNewBackbone(temporaryVersion);
			__MOUT__ << "\t\tNew backbone: " << newBbVersion << std::endl;
		}
		else
			__MOUT__ << "Alias doesnt exist: " << groupName << std::endl;




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

		__MOUT__ << "Alias: " << it->first << " - Key: " << it->second.key() << std::endl;

		//cfgMgr->loadConfiguration(cfgMgr->theConfigurations_,&(it->second));
		//cfgMgr->loadConfiguration(cfgMgr->theDefaultConfigurations_,&(it->second));
		//cfgMgr->loadConfiguration(cfgMgr->theVersionAliases_,&(it->second));
	    //cfgMgr->setupConfigurationGUI(&(it->second));	//load

		if(cfgMgr->getConfigurations())	// get Kocs
		{
			listOfKocs = cfgMgr->getConfigurations()->getListOfKocs(it->second.key());
			__MOUT__ << "\tKocs size: " << listOfKocs.size() << std::endl;


			for (std::set<std::string>::iterator sit=listOfKocs.begin(); sit!=listOfKocs.end(); ++sit)
			{
				unsigned int cv = cfgMgr->getConfigurations()->getConditionVersion(it->second.key(),*sit);

				__MOUT__ << "\tKoc: " << *sit << " Version: " << cv << std::endl;

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

			   __MOUT__ << "\tFssr Name: " << dit->first << " - Row: " << dit->second << std::endl;



				// get FSSR Configs
			   	if(cfgMgr->getFSSRDACsConfiguration())
			   	{
			   		ROCDACs	rocDacs = cfgMgr->getFSSRDACsConfiguration()->getROCDACs(dit->first);

			   		DACList	dacs  = rocDacs.getDACList(); //map of std::string to pair (int, int)


			   		DACList::const_iterator rdit = dacs.begin();
			   		while(rdit != dacs.end())
			   		{
						   __MOUT__ << "\t\tDac Name: " << rdit->first << " - Addr: " << rdit->second.first << ", Val: " << rdit->second.second << std::endl;
							rocDacs.setDAC(rdit->first,7,1);

						   rdit++;

			   		}


			   		dacs  = rocDacs.getDACList(); //map of std::string to pair (int, int)
					rdit = dacs.begin();
					while(rdit != dacs.end())
					{
						   __MOUT__ << "\t\tDac Name: " << rdit->first << " - Addr: " << rdit->second.first << ", Val: " << rdit->second.second << std::endl;
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
	__MOUT__ << "done getAliasList" << std::endl;


	//clear config managers
	for (std::map<std::string, ConfigurationManagerRW *> ::iterator it=userConfigurationManagers_.begin(); it!=userConfigurationManagers_.end(); ++it)
	{
		__MOUT__ << it->first << std::endl;
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
	for (std::map<std::string, ConfigurationManagerRW *> ::iterator it=userConfigurationManagers_.begin(); it!=userConfigurationManagers_.end(); ++it)
	{
		delete it->second;
		it->second = 0;
	}
	userConfigurationManagers_.clear();

	//NOTE: Moved to ConfigurationGUISupervisor [FIXME is this correct?? should we use shared_ptr??]
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
	std::string Command = CgiDataUtilities::getOrPostData(cgi,"RequestType");//from GET or POST

	__MOUT__ << "Command " << Command << " files: " << cgi.getFiles().size() << std::endl;

	//Commands
	//getConfigurationGroups
	//getConfigurations
	//getSpecificConfigurationGroup
	//saveNewConfigurationGroup
	//getSpecificConfiguration
	//saveSpecificConfiguration
	//changeKocVersionForSpecificConfig
	//getTreeView

	HttpXmlDocument xmldoc;
	uint8_t userPermissions;
	std::string userWithLock;
	std::string userName;
	uint64_t activeSessionIndex;

	//**** start LOGIN GATEWAY CODE ***//
	{
		bool automaticCommands = 0; //automatic commands should not refresh cookie code.. only user initiated commands should!
		bool checkLock = true;
		bool lockRequired = true;

		if(!theRemoteWebUsers_.xmlLoginGateway(
				cgi,out,&xmldoc,theSupervisorsConfiguration_,
				&userPermissions,  			//acquire user's access level (optionally null pointer)
				!automaticCommands,			//true/false refresh cookie code
				USER_PERMISSIONS_THRESHOLD, //set access level requirement to pass gateway
				checkLock,					//true/false enable check that system is unlocked or this user has the lock
				lockRequired,				//true/false requires this user has the lock to proceed
				&userWithLock,				//acquire username with lock (optionally null pointer)
				&userName					//acquire username of this user (optionally null pointer)
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


	//acquire user's configuration manager based on username & activeSessionIndex
	__MOUT__ << std::endl;
	std::string  backboneVersionStr = CgiDataUtilities::getData(cgi,"backboneVersion");		  	//from GET
	ConfigurationVersion backboneVersion(
			(backboneVersionStr == "")?-1:atoi(backboneVersionStr.c_str())); //default to latest
	__MOUT__ << "ConfigurationManagerRW backboneVersion Version req \t\t" << backboneVersionStr << std::endl;
	ConfigurationManagerRW* cfgMgr = refreshUserSession(userName, activeSessionIndex, backboneVersion);
	__MOUT__ << "ConfigurationManagerRW backboneVersion Version Loaded \t\t" << backboneVersion << std::endl;

	//return this information always
	//<backboneVersion=xxx />
	xmldoc.addTextElementToData("backboneVersion", backboneVersion.toString()); //add to response

	if(Command == "getConfigurationGroups")
	{
		handleConfigurationGroupsXML(xmldoc,cfgMgr);
	}
	else if(Command == "getConfigurations")
	{
		handleConfigurationsXML(xmldoc,cfgMgr);
	}
	else if(Command == "getSpecificConfigurationGroup")
	{
		std::string alias = CgiDataUtilities::getData(cgi,"alias"); //from GET

		__MOUT__ << "getSpecificConfigurationGroup: " << alias << std::endl;

		handleGetConfigurationGroupXML(xmldoc,cfgMgr,alias,
				ConfigurationVersion(ConfigurationVersion::DEFAULT)); //FIXME: to allow for non-default version
	}
	else if(Command == "saveNewConfigurationGroup")
	{
		std::string groupName = CgiDataUtilities::getData(cgi,"groupName"); //from GET
		std::string configList = CgiDataUtilities::postData(cgi,"configList"); //from POST
		__MOUT__ << "saveNewConfigurationGroup: " << groupName << std::endl;
		__MOUT__ << "configList: " << configList << std::endl;

		handleCreateConfigurationGroupXML(xmldoc,cfgMgr,groupName,configList);
	}
	else if(Command == "getSpecificConfiguration")
	{
		std::string		configName = CgiDataUtilities::getData(cgi,"configName"); 			//from GET
		std::string  	versionStr = CgiDataUtilities::getData(cgi,"version");		  	//from GET
		int				dataOffset = CgiDataUtilities::getDataAsInt(cgi,"dataOffset");	//from GET
		int				chunkSize = CgiDataUtilities::getDataAsInt(cgi,"chunkSize");	//from GET

		__MOUT__ << "getSpecificConfiguration: " << configName << " versionStr: " << versionStr
				<< " chunkSize: " << chunkSize << " dataOffset: " << dataOffset << std::endl;

		ConfigurationVersion version;
		std::map<std::string, ConfigurationInfo> allCfgInfo =	cfgMgr->getAllConfigurationInfo();
		if(versionStr == "" && //take latest version if no version specified
				allCfgInfo[configName].versions_.size())
			version = *(allCfgInfo[configName].versions_.rbegin());
		else					//else take specified version
			version = atoi(versionStr.c_str());

		__MOUT__ << " version: " << version << std::endl;

		handleGetConfigurationXML(xmldoc,cfgMgr,configName,ConfigurationVersion(version));
	}
	else if(Command == "saveSpecificConfiguration")
	{
		std::string 	configName 	= CgiDataUtilities::getData	    (cgi,"configName"); 	//from GET
		int				version 	= CgiDataUtilities::getDataAsInt(cgi,"version");	//from GET
		int				dataOffset 	= CgiDataUtilities::getDataAsInt(cgi,"dataOffset");	//from GET
		//int				chunkSize 	= CgiDataUtilities::getDataAsInt(cgi,"chunkSize");	//from GET
		int				temporary 	= CgiDataUtilities::getDataAsInt(cgi,"temporary");	//from GET

		std::string	data = CgiDataUtilities::postData(cgi,"data"); //from POST
		//data format: commas and semi-colons indicate new row
		//r0c0,r0c1,...,r0cN,;r1c0,...

		__MOUT__ << "getSpecificConfiguration: " << configName << " version: " << version
				<< " temporary: " << temporary << " dataOffset: " << dataOffset << std::endl;

		__MOUT__ << "data: " << data << std::endl;

		handleCreateConfigurationXML(xmldoc,cfgMgr,configName,ConfigurationVersion(version),
				temporary,data,dataOffset);
	}
	else if(Command == "changeKocVersionForSpecificConfig")
	{
		__MOUT__ << "changeKocVersionForSpecificConfig" << std::endl;

		//need 	std::string groupName = "Physics";
		//std::string KOCAlias = "FSSRDACsConfiguration";
		//int newVersion = 3;

		std::string 	groupName = CgiDataUtilities::getData(cgi,"groupName"); 	//from GET
		std::string 	KOCAlias = CgiDataUtilities::getData(cgi,"configName"); 			//from GET
		int				newVersion = CgiDataUtilities::getDataAsInt(cgi,"version");		//from GET

		//TODO
		std::string chosenSubConfig;
		std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();

		{

			std::map<std::string, ConfigurationGroupKey> aliasMap = cfgMgr->__GET_CONFIG__(ConfigurationAliases)->getAliasesMap();

			std::map<std::string, ConfigurationGroupKey>::const_iterator it = aliasMap.find(groupName);
			if(it != aliasMap.end())
			{
				__MOUT__ << "Alias: " << it->first << " - Key: " << it->second.key() << std::endl;

				__MOUT__ << "Alias exists: " << groupName << std::endl;
				__MOUT__ << "Sub system alias: " << KOCAlias << std::endl;
				__MOUT__ << "Changing to new version: " << newVersion << std::endl;

				std::set<std::string> listOfKocs;
				listOfKocs = cfgMgr->__GET_CONFIG__(Configurations)->getListOfKocs(it->second);
				for (std::set<std::string>::iterator sit=listOfKocs.begin(); sit!=listOfKocs.end(); ++sit)
				{
					ConfigurationVersion cv = cfgMgr->__GET_CONFIG__(Configurations)->getConditionVersion(it->second,*sit);

					__MOUT__ << "\tKoc: " << *sit << " Version: " << cv << std::endl;

					std::set<ConfigurationVersion> versions = allCfgInfo.find(*sit)->second.versions_;
					__MOUT__ << "\t\tAll versions: " << std::endl;
					for (auto &version:versions)
						__MOUT__ << " " << version << std::endl;

				}


				//Make copy for all backbone members to temporary versions!
				__MOUT__ << "\t\t**************************** Make temporary backbone" << std::endl;

				chosenSubConfig = "Configurations";
				ConfigurationVersion temporaryVersion;
				ConfigurationVersion versionToCopy = cfgMgr->__GET_CONFIG__(ConfigurationAliases)->getViewVersion();
				//cfgMgr->__GET_CONFIG__(DefaultConfigurations)->print();
				///////////////////////cfgMgr->__GET_CONFIG__(Configurations)->print();
				//cfgMgr->__GET_CONFIG__(VersionAliases)

				__MOUT__ << "\t\ttemporaryVersion versionToCopy: " << versionToCopy << std::endl;

				assert(allCfgInfo.find(chosenSubConfig) != allCfgInfo.end());

				bool isInDatabase = allCfgInfo[chosenSubConfig].versions_.find(versionToCopy) != allCfgInfo[chosenSubConfig].versions_.end();
				__MOUT__ << "Version " << versionToCopy << " is in database: " <<
						(isInDatabase?"YES":"NO") << std::endl;

				//temporaryVersion = allCfgInfo[chosenSubConfig].configurationPtr_->createTemporaryView(versionToCopy);
				temporaryVersion = cfgMgr->createTemporaryBackboneView(versionToCopy);
				__MOUT__ << "\t\ttemporaryVersion Backbone: " << temporaryVersion << std::endl;
				__MOUT__ << "\t\t(Note: it is not the) active version Backbone: " <<
						cfgMgr->__GET_CONFIG__(ConfigurationAliases)->getViewVersion()  << std::endl;


				//edit the backbone however you want
				__MOUT__ << "\t\t**************************** Make changes to backbone" << std::endl;

				ConfigurationView* cfgViewPtr = allCfgInfo[chosenSubConfig].configurationPtr_->getTemporaryView(temporaryVersion);


				__MOUT__ << "\t\t******** Before change" << std::endl;
				std::stringstream ss;
				cfgViewPtr->print(ss);
				__MOUT__ << ss.str() << std::endl;


				cfgMgr->setKOCVersionForSpecificConfiguration(allCfgInfo,
						temporaryVersion,groupName,KOCAlias,ConfigurationVersion(newVersion));

				__MOUT__ << "\t\t******** After change" << std::endl;
				ss.str(""); //clear stringstream (note: clear() just clears error state)
				cfgViewPtr->print(ss);
				__MOUT__ << ss.str() << std::endl;

				//Save temporary backbone view to new version
				__MOUT__ << "\t\t******** Saving new version" << std::endl;
				ConfigurationVersion newBbVersion = cfgMgr->saveNewBackbone(temporaryVersion);
				__MOUT__ << "\t\tNew backbone: " << newBbVersion << std::endl;
			}
			else
				__MOUT__ << "Alias doesnt exist: " << groupName << std::endl;

			//reply with resulting sub system config
			handleGetConfigurationGroupXML(xmldoc,cfgMgr,groupName,ConfigurationVersion(ConfigurationVersion::DEFAULT)); //FIXME.. should not always be DEFAULT!! should come from request

		}
	}
	else if(Command == "getTreeView")
	{
		std::string 	configGroup 	= CgiDataUtilities::getData(cgi,"configGroup");
		std::string 	configGroupKey 	= CgiDataUtilities::getData(cgi,"configGroupKey");
		std::string 	startPath 		= CgiDataUtilities::postData(cgi,"startPath");
		int				depth	 		= CgiDataUtilities::getDataAsInt(cgi,"depth");

		__MOUT__ << "configGroup: " << configGroup << std::endl;
		__MOUT__ << "configGroupKey: " << configGroupKey << std::endl;
		__MOUT__ << "startPath: " << startPath << std::endl;
		__MOUT__ << "depth: " << depth << std::endl;
		handleFillTreeViewXML(xmldoc,cfgMgr,configGroup,ConfigurationGroupKey(configGroupKey),startPath,depth);
	}
	else if(Command == "activateGlobalConfig")
	{
		std::string 	configGroup 	= CgiDataUtilities::getData(cgi,"configGroup");

		__MOUT__ << "Activating config: " << configGroup << std::endl;
		//		if(configGroup == "")
		//			configGroup = cfgMgr->getActiveGlobalConfiguration();
		//		else
		//			configGroup = cfgMgr->setActiveGlobalConfiguration(configGroup);
		//		xmldoc.addTextElementToData("activeConfig", configGroup);
	}
	else
		__MOUT__ << "Command request not recognized." << std::endl;


	//return xml doc holding server response
	xmldoc.outputXmlDocument((std::ostringstream*) out, true);
}

//========================================================================================================================
//handleFillTreeViewXML
//	returns xml tree from path for given depth
//
//parameters
//	configGroupName (full name with key)
//	starting node path
//	depth from starting node path
//
void ConfigurationGUISupervisor::handleFillTreeViewXML(HttpXmlDocument &xmldoc, ConfigurationManagerRW *cfgMgr,
		const std::string &configGroup, const ConfigurationGroupKey &configGroupKey,
		const std::string &startPath, int depth)
{
	//return xml
	//	<configGroup="configGroup"/>
	//	<tree="path">
	//		<node="...">
	//			<node="...">
	//				<node="...">
	//					<value="...">
	//				</node>
	//				<node="...">
	//					<value="...">
	//				</node>
	//			</node>
	//			<node="...">
	//				<value="..">
	//			</node>
	//		...
	//		</node>
	//	</tree>

	//return the startPath as root "tree" element
	//	and then display all children if depth > 0
	xmldoc.addTextElementToData("configGroup", configGroup);
	xmldoc.addTextElementToData("configGroupKey", configGroupKey.toString());

	try {
		cfgMgr->loadConfigurationGroup(configGroup,configGroupKey);

		__MOUT__ << "!" << std::endl;

		DOMElement* parentEl = xmldoc.addTextElementToData("tree", startPath);

		if(depth == 0) return; //already returned root node in itself

		__MOUT__ << "!" << std::endl;
		std::map<std::string,ConfigurationTree> rootMap;
		if(startPath == "/") //then consider the configurationManager the root node
			rootMap = cfgMgr->getChildren();
		else
			rootMap = cfgMgr->getNode(startPath).getChildren();

		__MOUT__ << "!" << std::endl;
		for(auto &treePair:rootMap)
			recursiveTreeToXML(treePair.second,depth-1,xmldoc,parentEl);
	}
	catch(std::runtime_error& e)
	{
		__MOUT__ << "Error detected!\n\n " << e.what() << std::endl;
		xmldoc.addTextElementToData("Error", "Error generating XML tree! " + std::string(e.what()));
	}
	catch(...)
	{
		__MOUT__ << "Error detected!" << std::endl;
		xmldoc.addTextElementToData("Error", "Error generating XML tree!");
	}
}

//==============================================================================
//recursiveToXml
//	output tree to XML from this node for desired depth
//	depth of 0 means output only this node's value
//	depth of 1 means include this node's children's values, etc..
//	depth of -1(unsigned int) effectively means output full tree
void ConfigurationGUISupervisor::recursiveTreeToXML(const ConfigurationTree &t, unsigned int depth, HttpXmlDocument &xmldoc,
		DOMElement* parentEl)
{
	__MOUT__ << t.getValueAsString() << std::endl;
	if(t.isValueNode())
	{
		parentEl = xmldoc.addTextElementToParent("node", t.getValueName(), parentEl);
		xmldoc.addTextElementToParent("value", t.getValueAsString(), parentEl);
	}
	else
	{
		if(t.isLinkNode())
		{
			if(t.isDisconnected())
			{
				parentEl = xmldoc.addTextElementToParent("node", t.getValueName(), parentEl);
				xmldoc.addTextElementToParent("value", t.getValueAsString(), parentEl);
				return;
			}
			parentEl = xmldoc.addTextElementToParent("node", t.getValueName(), parentEl);
			xmldoc.addTextElementToParent(
					(t.isGroupLink()?"Group":"U") +	std::string("ID"),
					t.getValueAsString(), parentEl);
		}
		else
			parentEl = xmldoc.addTextElementToParent("node", t.getValueAsString(), parentEl);

		//if depth>=1 toXml all children
		//child.toXml(depth-1)
		if(depth >= 1)
		{
			auto C = t.getChildren();
			for(auto &c:C)
				recursiveTreeToXML(c.second,depth-1,xmldoc,parentEl);
		}
	}
}

//========================================================================================================================
//handleGetConfigurationGroupXML
//
//give the detail of specific System Configuration specified
//Find historical alias keys by
//loading all historical configurations
//figure out all configurations versions
//open file, and check for alias name and associated extract key
//
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
void ConfigurationGUISupervisor::handleGetConfigurationGroupXML(HttpXmlDocument &xmldoc,
		ConfigurationManagerRW *cfgMgr, const std::string &alias, ConfigurationVersion backboneVersion)
{
	char tmpIntStr[100];
	DOMElement* parentEl;


	//steps:
	//	extract configuration name and version from alias


	//	extract configuration name and version from alias
	//		version is delimited by _v
	int i;
	for(i=alias.length()-1;i>=0;--i)
		if(alias[i] == 'v' && i>1 && alias[i-1] == '_')
			break; //found version indicator

	if(i < 0)
	{
		xmldoc.addTextElementToData("Error", "Invalid configuration alias requested, " + alias);
		return;
	}

	std::string configName = alias.substr(0,i-1);
	std::string version = alias.substr(i+1);


	__MOUT__ << "configName=" << configName << std::endl;
	__MOUT__ << "version=" << version << std::endl;




	return;

	std::map<std::string, ConfigurationGroupKey> aliasMap = cfgMgr->__GET_CONFIG__(ConfigurationAliases)->getAliasesMap();

	std::map<std::string, ConfigurationGroupKey>::const_iterator it = aliasMap.find(alias);
	if(it != aliasMap.end())
	{
		xmldoc.addTextElementToData("ConfigurationGroupName", it->first);

		std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
		std::set<ConfigurationVersion> versions;

		std::set<std::string> listOfKocs;

		DOMElement* parentElKoc;

		//get all KOC alias and version numbers

		//__MOUT__ << "Alias: " << it->first << " - Key: " << it->second.key() << std::endl;

		//add system configuration alias and key
		xmldoc.addTextElementToData("ConfigurationGroupName", it->first);
		sprintf(tmpIntStr,"%u",it->second.key());
		xmldoc.addTextElementToData("ConfigurationGroupKey", tmpIntStr);
		parentEl = xmldoc.addTextElementToData("SystemConfigurationKOCs", "");

		//get KOCs alias and version for the current system configuration key
		assert(cfgMgr->__GET_CONFIG__(Configurations));
		{
			listOfKocs = cfgMgr->__GET_CONFIG__(Configurations)->getListOfKocs(it->second);
			//__MOUT__ << "\tKocs size: " << listOfKocs.size() << std::endl;

			for (std::set<std::string>::iterator sit=listOfKocs.begin(); sit!=listOfKocs.end(); ++sit)
			{
				//current version
				ConfigurationVersion cv = cfgMgr->__GET_CONFIG__(Configurations)->getConditionVersion(it->second,*sit);

				//all existing versions
				versions = allCfgInfo.find(*sit)->second.versions_;

				//__MOUT__ << "\tKoc: " << *sit << " Version: " << cv << std::endl;

				xmldoc.addTextElementToParent("MemberName", *sit, parentEl);
				parentElKoc = xmldoc.addTextElementToParent("KOC_currentVersion", cv.toString(), parentEl);
				for (auto &version:versions)
				{
					//__MOUT__ << "\t\t" << *vit << std::endl;
					if(version == cv) continue;
					xmldoc.addTextElementToParent("KOC_existingVersion", version.toString(), parentElKoc);
				}
			}
		}


		//check for other existing backbone versions - they all (3 backbone pieces) better match!!
		//	(so we can just check one of them to get the list of historical versions: Configurations)
		//  (The 3 backbone pieces are Configurations -- ConfigurationAliases -- DefaultConfigurations

		versions = allCfgInfo.find("Configurations")->second.versions_;

		for (auto &version:versions)
		{
			__MOUT__ << "Configurations Version \t\t" << version << std::endl;
			if(version != backboneVersion)
			{
				//found a different configurations version then current version:
				//load it
				//check for alias

				cfgMgr->loadConfigurationBackbone(version);

				std::map<std::string, ConfigurationGroupKey> aliasMap = cfgMgr->__GET_CONFIG__(ConfigurationAliases)->getAliasesMap();

				it = aliasMap.find(alias);
				if(it != aliasMap.end())
				{
					//found a historical version of alias
					sprintf(tmpIntStr,"%u",it->second.key());
					xmldoc.addTextElementToData("HistoricalConfigurationGroupKey", tmpIntStr);

				}
			}
		}
	}
}

//========================================================================================================================
//handleGetConfigurationXML
//
//	if INVALID or version does not exists, default to mock-up
//
//give the detail of specific Sub-System Configuration specified
//	by configName and version
//
//if no version selected, default to latest version
//if no versions exists, default to mock-up
//
//return existing versions
//return column headers
//return number of rows
//from dataOffset
//first CHUNK_SIZE rows
//
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
void ConfigurationGUISupervisor::handleGetConfigurationXML(HttpXmlDocument &xmldoc,
		ConfigurationManagerRW *cfgMgr, const std::string &configName, ConfigurationVersion version)
{
	char tmpIntStr[100];
	DOMElement* parentEl;

	std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
	if(allCfgInfo[configName.c_str()].versions_.find(version) ==
			allCfgInfo[configName.c_str()].versions_.end())
	{
		__MOUT__ << "Version not found, so using mockup." << std::endl;
		version = ConfigurationVersion(); //use INVALID
	}

	xmldoc.addTextElementToData("ConfigurationName", configName);	//table name
	xmldoc.addTextElementToData("ConfigurationVersion", version.toString());	//table version

	//existing table versions
	parentEl = xmldoc.addTextElementToData("ConfigurationVersions", "");
	for (auto &v:allCfgInfo[configName.c_str()].versions_)
		xmldoc.addTextElementToParent("VersionKey", v.toString(), parentEl);


	//table columns and then rows (from config view)

	//get view pointer
	ConfigurationView* cfgViewPtr;
	if(version.isInvalid()) //use mockup
		cfgViewPtr = cfgMgr->getConfigurationByName(configName)->getMockupViewP();
	else					//use view version
		cfgViewPtr = cfgMgr->getVersionedConfigurationByName(configName,version)->getViewP();



	//get 'columns' of view
	parentEl = xmldoc.addTextElementToData("CurrentVersionColumnHeaders", "");
	std::vector<ViewColumnInfo> colInfo = cfgViewPtr->getColumnsInfo();
	for(int i=0;i<(int)colInfo.size();++i)	//column headers and types
	{
		__MOUT__ << "\t\tCol " << i << ": " << colInfo[i].getType()  << "() " <<
				colInfo[i].getName() << " "
				<< colInfo[i].getStorageName() << " " << colInfo[i].getDataType() << std::endl;

		xmldoc.addTextElementToParent("ColumnHeader", colInfo[i].getName(), parentEl);
		xmldoc.addTextElementToParent("ColumnType", colInfo[i].getType(), parentEl);
		xmldoc.addTextElementToParent("ColumnDataType", colInfo[i].getDataType(), parentEl);
	}

	parentEl = xmldoc.addTextElementToData("CurrentVersionRows", "");

	for(int r=0;r<(int)cfgViewPtr->getNumberOfRows();++r)
	{
		//__MOUT__ << "\t\tRow " << r << ": "  << std::endl;

		sprintf(tmpIntStr,"%d",r);
		DOMElement* tmpParentEl = xmldoc.addTextElementToParent("Row", tmpIntStr, parentEl);

		for(int c=0;c<(int)cfgViewPtr->getNumberOfColumns();++c)
			if(colInfo[c].getDataType() == "NUMBER")
			{
				int num;
				cfgViewPtr->getValue(num,r,c);
				__MOUT__ << "\t " << num << std::endl;

				sprintf(tmpIntStr,"%d",num);
				xmldoc.addTextElementToParent("Entry", tmpIntStr, tmpParentEl);
			}
			else
			{
				std::string val;
				cfgViewPtr->getValue(val,r,c);
				__MOUT__ << "\t " << val << std::endl;

				xmldoc.addTextElementToParent("Entry", val, tmpParentEl);
				xmldoc.addTextElementToData("ty", val);
			}
	}
}


//========================================================================================================================
//handleCreateConfigurationXML
//
//
//save the detail of specific Sub-System Configuration specified
//	by configName and version

//starting from dataOffset
//
//	if starting version is -1 start from mock-up
void ConfigurationGUISupervisor::handleCreateConfigurationXML(HttpXmlDocument &xmldoc,
		ConfigurationManagerRW *cfgMgr, const std::string &configName, ConfigurationVersion version,
		bool makeTemporary, const std::string &data, const int &dataOffset)
try
{
	__MOUT__ << "handleCreateConfigurationXML: " << configName << " version: " << version
			<< " dataOffset: " << dataOffset << std::endl;

	__MOUT__ << "data: " << data << std::endl;

	//create temporary version from starting version
	if(!version.isInvalid()) //if not using mock-up, make sure starting version is loaded
		cfgMgr->getVersionedConfigurationByName(configName,version);
	ConfigurationVersion temporaryVersion = cfgMgr->getConfigurationByName(configName)->createTemporaryView(version);

	__MOUT__ << "\t\ttemporaryVersion: " << temporaryVersion << std::endl;

	cfgMgr->getConfigurationByName(configName)->getTemporaryView(temporaryVersion)->fillFromCSV(data,dataOffset);

	if(makeTemporary)
		__MOUT__ << "\t\t**************************** Save as temporary sub-config version" << std::endl;
	else
		__MOUT__ << "\t\t**************************** Save as new sub-config version" << std::endl;

	ConfigurationVersion newAssignedVersion =
			cfgMgr->saveNewConfiguration(configName,temporaryVersion,makeTemporary);

	xmldoc.addTextElementToData("savedAlias", configName);
	xmldoc.addTextElementToData("savedVersion", newAssignedVersion.toString());

	__MOUT__ << "\t\t newAssignedVersion: " << newAssignedVersion << std::endl;
}
catch(std::runtime_error &e)
{
	__MOUT__ << "Error detected!\n\n " << e.what() << std::endl;
	xmldoc.addTextElementToData("Error", "Error saving new view! " + std::string(e.what()));
}
catch(...)
{
	__MOUT__ << "Error detected!\n\n "<< std::endl;
	xmldoc.addTextElementToData("Error", "Error saving new view! ");
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
ConfigurationManagerRW* ConfigurationGUISupervisor::refreshUserSession(std::string username,
		uint64_t activeSessionIndex, ConfigurationVersion &backboneVersion)
{
	std::stringstream ssMapKey;
	ssMapKey << username << ":" << activeSessionIndex;
	std::string mapKey = ssMapKey.str();
	__MOUT__ << "Config Session: " << mapKey << " ... out of size: " << userConfigurationManagers_.size() << std::endl;

	time_t now = time(0);

	//create new config mgr if not one for active session index
	if(userConfigurationManagers_.find(mapKey) == userConfigurationManagers_.end())
	{
		userConfigurationManagers_[mapKey] = new ConfigurationManagerRW(username);

		//update configuration info for each new configuration manager
		//	IMPORTANTLY this also fills all configuration manager pointers with instances,
		//	so we are not dealing with changing pointers later on
		userConfigurationManagers_[mapKey]->getAllConfigurationInfo(true);	//load empty instance of everything important
	}
	else if(userLastUseTime_.find(mapKey) == userLastUseTime_.end())
	{
		__MOUT_ERR__ << "Fatal error managing userLastUseTime_!" << std::endl;
		throw std::runtime_error("Fatal error managing userLastUseTime_!");
	}
	else if(now - userLastUseTime_[mapKey] >
	CONFIGURATION_MANAGER_REFRESH_THRESHOLD) //check if should refresh all config info
	{
		__MOUT_INFO__ << "Refreshing all configuration info." << std::endl;
		userConfigurationManagers_[mapKey]->getAllConfigurationInfo(true);
	}

	//load backbone configurations always based on backboneVersion
	//if backboneVersion is -1, then latest
	backboneVersion = 0;// userConfigurationManagers_[mapKey]->loadConfigurationBackbone(backboneVersion);

	//update active sessionIndex last use time
	userLastUseTime_[mapKey] = now;

	//check for stale sessions and remove them (so config user maps do not grow forever)
	for (std::map<std::string, time_t> ::iterator it=userLastUseTime_.begin(); it!=userLastUseTime_.end(); ++it)
		if(now - it->second > CONFIGURATION_MANAGER_EXPIRATION_TIME) // expired!
		{
			__MOUT__ << now << ":" << it->second << " = " << now - it->second << std::endl;
			delete userConfigurationManagers_[it->first]; //call destructor
			if(!(userConfigurationManagers_.erase(it->first)))	//erase by key
			{
				__MOUT_ERR__ << "Fatal error erasing configuration manager by key!" << std::endl;
				throw std::runtime_error("Fatal error erasing configuration manager by key!");
			}
			userLastUseTime_.erase(it);								//erase by iterator

			it=userLastUseTime_.begin(); //fail safe.. reset it, to avoid trying to understand what happens with the next iterator
		}

	return userConfigurationManagers_[mapKey];
}

//========================================================================================================================
//	handleCreateConfigurationGroupXML
//
//		Save a new ConfigurationGroup:
//			Search for existing ConfigurationGroupKeys for this ConfigurationGroup
//			Append a "bumped" system key to alias
//			Save based on list of configuration name/ConfigurationVersion
//
//		configList parameter is comma separated configuration name and version
//
void ConfigurationGUISupervisor::handleCreateConfigurationGroupXML	(HttpXmlDocument &xmldoc,
		ConfigurationManagerRW *cfgMgr, const std::string &groupName,
		const std::string &configList)
{
	std::map<std::string /*name*/, ConfigurationVersion /*version*/> groupMembers;
	std::string name, version;
	auto c = configList.find(',',0);
	auto i = c; i = 0; //auto used to get proper index/length type
	while(c < configList.length())
	{
		//add the configuration and version pair to the map
		name = configList.substr(i,c-i);
		i = c+1;
		c = configList.find(',',i);
		if(c == std::string::npos) //missing version list entry?!
		{
			xmldoc.addTextElementToData("Error", "Incomplete Configuration-Version pair!");
			return;
		}

		version = configList.substr(i,c-i);
		i = c+1;
		c = configList.find(',',i);

		__MOUT__ << "name: " << name << std::endl;
		__MOUT__ << "version: " << version << std::endl;

		groupMembers[name] = ConfigurationVersion(version.c_str());
	}

	ConfigurationGroupKey newKey;
	try
	{
		newKey = cfgMgr->saveNewConfigurationGroup(groupName,groupMembers);
	}
	catch(std::runtime_error &e)
	{
		__MOUT_ERR__ << "Failed to create config group: " << groupName << std::endl;
		__MOUT_ERR__ << "\n\n" << e.what() << std::endl;
		xmldoc.addTextElementToData("Error", "Failed to create configuration group: " + groupName +
				". (Error: " + e.what() + ")");
	}
	catch(...)
	{
		__MOUT_ERR__ << "Failed to create config group: " << groupName << std::endl;
		xmldoc.addTextElementToData("Error", "Failed to create configuration group: " + groupName);
	}

	xmldoc.addTextElementToData("NewGroupName",groupName);
	xmldoc.addTextElementToData("NewGroupKey",newKey.toString());
}


//========================================================================================================================
//	handleConfigurationGroupsXML
//
//		return this information
//		<configuration alias=xxx key=xxx>
//			<koc alias=xxx key=xxx />
//			<koc alias=xxx key=xxx />
//			...
//		</configuration>
//		<configuration alias=xxx key=xxx>...</configuration>
//		...
//
//Note: this is the new way with artdaq_db
void ConfigurationGUISupervisor::handleConfigurationGroupsXML(HttpXmlDocument &xmldoc, ConfigurationManagerRW *cfgMgr)
{
	DOMElement* parentEl;

	ConfigurationInterface* theInterface = cfgMgr->getConfigurationInterface();
	auto configGroups = theInterface->getAllConfigurationGroupNames();
	__MOUT__ << "Global config size: " << configGroups.size() << std::endl;

	ConfigurationGroupKey groupKey;
	std::string groupName;

	for(auto &groupString:configGroups)
	{
		ConfigurationGroupKey::getGroupNameAndKey(groupString,groupName,groupKey);

		__MOUT__ << "Config group " << groupString << " := " << groupName <<
				"(" << groupKey << ")" << std::endl;

		xmldoc.addTextElementToData("ConfigurationGroupName", groupName);
		xmldoc.addTextElementToData("ConfigurationGroupKey", groupKey.toString());
		parentEl = xmldoc.addTextElementToData("SystemConfigurationKOCs", "");

		std::map<std::string /*name*/, ConfigurationVersion /*version*/> gcMap;
		try
		{
			gcMap = theInterface->getConfigurationGroupMembers(groupString);
		}
		catch(...)
		{
			xmldoc.addTextElementToData("Error","Configuration group \"" + groupString +
					"\" has been corrupted!");
			continue;
		}

		for(auto &cv:gcMap)
		{
			__MOUT__ << "\tMember config " << cv.first << ":" << cv.second << std::endl;
			xmldoc.addTextElementToParent("MemberName", cv.first, parentEl);
			xmldoc.addTextElementToParent("MemberVersion", cv.second.toString(), parentEl);
		}
	}

}

//========================================================================================================================
//	handleConfigurationsXML
//
//		return this information
//		<subconfiguration alias=xxx>
//			<version key=xxx />
//			<version key=xxx />
//			...
//		</subconfiguration>
//		<subconfiguration alias=xxx>...</subconfiguration>
//		...
//
void ConfigurationGUISupervisor::handleConfigurationsXML(HttpXmlDocument &xmldoc, ConfigurationManagerRW *cfgMgr)
{
	DOMElement* parentEl;
	std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
	std::map<std::string, ConfigurationInfo>::const_iterator it = allCfgInfo.begin();

	while(it != allCfgInfo.end())
	{
		//for each subconfiguration alias
		//get existing version keys

		__MOUT__ << "Alias: " << it->first << " - #ofVersions: " << it->second.versions_.size() << std::endl;

		//add system subconfiguration alias
		xmldoc.addTextElementToData("SystemSubConfigurationAlias", it->first);
		parentEl = xmldoc.addTextElementToData("SystemSubConfigurationVersions", "");

		//get version key for the current system subconfiguration key
		for (auto &version:it->second.versions_)
			xmldoc.addTextElementToParent("VersionKey", version.toString(), parentEl);

		++it;
	}
}

//========================================================================================================================
//	testXDAQContext
//testXDAQContext just a test bed for navigating the new config tree
void ConfigurationGUISupervisor::testXDAQContext()
{

	//behave like a user
	//start with top level xdaq context
	//	then add and delete rows proof-of-concept
	//export xml xdaq config file

	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//behave like a new user

	ConfigurationManagerRW cfgMgrInst("ExampleUser");

	ConfigurationManagerRW *cfgMgr = &cfgMgrInst;

	std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo(true);
	__MOUT__ << "allCfgInfo.size() = " << allCfgInfo.size() << std::endl;
	for(auto& mapIt : allCfgInfo)
	{
		__MOUT__ << "Config Alias: " << mapIt.first << std::endl;
		__MOUT__ << "\t\tExisting Versions: " << mapIt.second.versions_.size() << std::endl;

		//get version key for the current system subconfiguration key
		for (auto &v:mapIt.second.versions_)
		{
			__MOUT__ << "\t\t" << v << std::endl;
		}
	}
	cfgMgr->testXDAQContext();




	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////


}






