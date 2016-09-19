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

#define __MF_SUBJECT__ "CfgGUI"
#define __MF_HDR__		__COUT_HDR_FL__
#define __MOUT_ERR__  	mf::LogError	(__MF_SUBJECT__) << __MF_HDR__
#define __MOUT_WARN__  	mf::LogWarning	(__MF_SUBJECT__) << __MF_HDR__
#define __MOUT_INFO__  	mf::LogInfo		(__MF_SUBJECT__) << __COUT_HDR__
#define __MOUT__  		std::cout << __MF_HDR__//mf::LogDebug	(__MF_SUBJECT__) << __MF_HDR__



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

	__MOUT__ << "To prove the concept...";


	return;

	testXDAQContext(); //test new config

	return;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//behave like a new user

	std::string userConfigurationManagerIndex = "1";
	userConfigurationManagers_["1"] = new ConfigurationManagerRW();
	userConfigurationManagers_["2"] = new ConfigurationManagerRW();
	ConfigurationManagerRW *cfgMgr = userConfigurationManagers_[userConfigurationManagerIndex];

	std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
	__MOUT__ << "All config info loaded." << std::endl;
	std::map<std::string, ConfigurationKey>	aliasMap = cfgMgr->__GET_CONFIG__(ConfigurationAliases)->getAliasesMap();


	__MOUT__ << "aliasMap size: " << aliasMap.size() << std::endl;
	__MOUT__ << "getAllConfigurationInfo size: " << allCfgInfo.size() << std::endl;


	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//for each configuration alias and key
	//get KOC version numbers (this is the version "conditioned" by the alias-key pair)

	std::set<std::string> listOfKocs;
	std::map<std::string, ConfigurationKey>::const_iterator it = aliasMap.begin();
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
		std::string specSystemAlias = "Physics";
		std::string KOCAlias = "FSSRDACsConfiguration";
		ConfigurationVersion newVersion(3);

		std::map<std::string, ConfigurationKey> aliasMap = cfgMgr->__GET_CONFIG__(ConfigurationAliases)->getAliasesMap();

		std::map<std::string, ConfigurationKey>::const_iterator it = aliasMap.find(specSystemAlias);
		if(it != aliasMap.end())
		{
			__MOUT__ << "Alias: " << it->first << " - Key: " << it->second.key() << std::endl;

			__MOUT__ << "Alias exists: " << specSystemAlias << std::endl;
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


			cfgMgr->setKOCVersionForSpecificConfiguration(allCfgInfo,temporaryVersion,specSystemAlias,KOCAlias,newVersion);

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
			__MOUT__ << "Alias doesnt exist: " << specSystemAlias << std::endl;




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
	//getSystemConfigurations
	//getSubSystemConfigurations
	//getSpecificSystemConfiguration
	//getSpecificSubSystemConfiguration
	//saveSpecificSubSystemConfiguration
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

		if(!theRemoteWebUsers_.xmlLoginGateway(
				cgi,out,&xmldoc,theSupervisorsConfiguration_,
				&userPermissions,  			//acquire user's access level (optionally null pointer)
				"0",						//report user's ip address, if known
				!automaticCommands,			//true/false refresh cookie code
				USER_PERMISSIONS_THRESHOLD, //set access level requirement to pass gateway
				checkLock,					//true/false enable check that system is unlocked or this user has the lock
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


	//HttpXmlDocument xmldoc(cookieCode);

	//acquire user's configuration manager based on username & activeSessionIndex
	__MOUT__ << std::endl;
	std::string  backboneVersionStr = CgiDataUtilities::getData(cgi,"backboneVersion");		  	//from GET
	ConfigurationVersion backboneVersion(
			(backboneVersionStr == "")?-1:atoi(backboneVersionStr.c_str())); //default to latest
	__MOUT__ << "ConfigurationManagerRW backboneVersion Version req \t\t" << backboneVersionStr << std::endl;
	ConfigurationManagerRW* cfgMgr = refreshUserSession(userName, activeSessionIndex, backboneVersion);
	__MOUT__ << "ConfigurationManagerRW backboneVersion Version Loaded \t\t" << backboneVersion << std::endl;

	char tmpIntStr[100];
	DOMElement* parentEl;

	//return this information always
	//<backboneVersion=xxx />
	xmldoc.addTextElementToData("backboneVersion", backboneVersion.toString()); //add to response

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

		if(0) //old way, not with artdaq_db handling global configurations
		{
			std::map<std::string, ConfigurationKey> aliasMap = cfgMgr->__GET_CONFIG__(ConfigurationAliases)->getAliasesMap();
			std::map<std::string, ConfigurationKey>::const_iterator it = aliasMap.begin();

			std::set<std::string> listOfKocs;

			while (1 &&
					it != aliasMap.end())
			{
				//for each configuration alias and key
				//get KOC version numbers

				//__MOUT__ << "Alias: " << it->first << " - Key: " << it->second.key() << std::endl;

				//add system configuration alias and key
				xmldoc.addTextElementToData("SystemConfigurationAlias", it->first);
				sprintf(tmpIntStr,"%u",it->second.key());
				xmldoc.addTextElementToData("SystemConfigurationKey", tmpIntStr);
				parentEl = xmldoc.addTextElementToData("SystemConfigurationKOCs", "");

				//get KOCs alias and version for the current system configuration key
				assert(cfgMgr->__GET_CONFIG__(Configurations));
				{
					listOfKocs = cfgMgr->__GET_CONFIG__(Configurations)->getListOfKocs(it->second);
					//__MOUT__ << "\tKocs size: " << listOfKocs.size() << std::endl;

					for (std::set<std::string>::iterator sit=listOfKocs.begin(); sit!=listOfKocs.end(); ++sit)
					{
						ConfigurationVersion cv = cfgMgr->__GET_CONFIG__(Configurations)->getConditionVersion(it->second,*sit);

						//__MOUT__ << "\tKoc: " << *sit << " Version: " << cv << std::endl;

						xmldoc.addTextElementToParent("KOC_alias", *sit, parentEl);
						xmldoc.addTextElementToParent("KOC_version", cv.toString(), parentEl);
					}
				}
				++it;
			}
		}

		//new way with artdaq_db

		{
			ConfigurationInterface* theInterface = cfgMgr->getConfigurationInterface();
			auto gcfgs = theInterface->findAllGlobalConfigurations();
			__MOUT__ << "Global config size: " << gcfgs.size() << std::endl;

			for(auto &g:gcfgs)
			{
				__MOUT__ << "Global config " << g << std::endl;

				xmldoc.addTextElementToData("SystemConfigurationAlias", g);
				xmldoc.addTextElementToData("SystemConfigurationKey", "0");
				parentEl = xmldoc.addTextElementToData("SystemConfigurationKOCs", "");


				cfgMgr->loadGlobalConfiguration(g);//theInterface->loadGlobalConfiguration(g);
				auto gcMap = cfgMgr->getActiveVersions();
				for(auto &cv:gcMap)
				{
					__MOUT__ << "\tMember config " << cv.first << ":" << cv.second << std::endl;
					xmldoc.addTextElementToParent("KOC_alias", cv.first, parentEl);
					xmldoc.addTextElementToParent("KOC_version", cv.second.toString(), parentEl);
				}
			}
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
		__MOUT__ << std::endl;
		std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
		__MOUT__ << std::endl;
		std::map<std::string, ConfigurationInfo>::const_iterator it = allCfgInfo.begin();
		__MOUT__ << std::endl;

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

		std::string alias = CgiDataUtilities::getData(cgi,"alias"); //from GET

		__MOUT__ << "getSpecificSystemConfiguration: " << alias << std::endl;

		fillSpecificSystemXML(xmldoc,cfgMgr,alias,ConfigurationVersion(ConfigurationVersion::DEFAULT)); //FIXME: to allow for non-default version
	}
	else if(Command == "getSpecificSubSystemConfiguration")
	{
		//give the detail of specific Sub-System Configuration specified
		//	by subAlias and version

		//if no version selected, default to latest version
		//if no versions exists, default to mock-up

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

		std::string		subAlias = CgiDataUtilities::getData(cgi,"subAlias"); 			//from GET
		std::string  	versionStr = CgiDataUtilities::getData(cgi,"version");		  	//from GET
		int				version = (versionStr == "")?-2:atoi(versionStr.c_str());
		int				dataOffset = atoi(CgiDataUtilities::getData(cgi,"dataOffset").c_str());	//from GET
		int				chunkSize = atoi(CgiDataUtilities::getData(cgi,"chunkSize").c_str());	//from GET

		__MOUT__ << "getSpecificSubSystemConfiguration: " << subAlias << " version: " << version
				<< " chunkSize: " << chunkSize << " dataOffset: " << dataOffset << std::endl;

		fillSpecificSubSystemXML(xmldoc,cfgMgr,subAlias,ConfigurationVersion(version));
	}
	else if(Command == "saveSpecificSubSystemConfiguration")
	{
		__MOUT__ << "saveSpecificSubSystemConfiguration" << std::endl;
		//TODO
		// CHECK MUST HAVE LOCK!!

		//save the detail of specific Sub-System Configuration specified
		//	by subAlias and version

		//starting from dataOffset
		//save first CHUNK_SIZE rows
		std::string 	subAlias = CgiDataUtilities::getData(cgi,"subAlias"); 			//from GET
		int		version = atoi(CgiDataUtilities::getData(cgi,"version").c_str());	//from GET
		int		dataOffset = atoi(CgiDataUtilities::getData(cgi,"dataOffset").c_str());	//from GET
		int		chunkSize = atoi(CgiDataUtilities::getData(cgi,"chunkSize").c_str());	//from GET

		std::string	data = CgiDataUtilities::postData(cgi,"data"); //from POST
		//data format: commas and semi-colons indicate new row
		//r0c0,r0c1,...,r0cN,;r1c0,...


		__MOUT__ << "getSpecificSubSystemConfiguration: " << subAlias << " version: " << version
				<< " chunkSize: " << chunkSize << " dataOffset: " << dataOffset << std::endl;

		__MOUT__ << "data: " << data << std::endl;

		//FIXME CHECK MUST HAVE LOCK!!
		// CHECK MUST HAVE LOCK!!
		// CHECK MUST HAVE LOCK!!
		// CHECK MUST HAVE LOCK!!
		// CHECK MUST HAVE LOCK!!

		saveSpecificSubSystemVersion(xmldoc,cfgMgr,subAlias,ConfigurationVersion(version),
				data,dataOffset,chunkSize);


		//		//verify alias and version exists
		//		std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
		//		std::map<std::string, ConfigurationInfo>::const_iterator it = allCfgInfo.find(subAlias);
		//
		//
		//		__MOUT__ << "getSpecificSubSystemConfiguration: " << subAlias << " version: " << version
		//				<< " chunkSize: " << chunkSize << " dataOffset: " << dataOffset << std::endl;
		//
		//		__MOUT__ << "data: " << data << std::endl;
		//
		//		//FIXME CHECK MUST HAVE LOCK!!
		//		// CHECK MUST HAVE LOCK!!
		//		// CHECK MUST HAVE LOCK!!
		//		// CHECK MUST HAVE LOCK!!
		//		// CHECK MUST HAVE LOCK!!
		//		if(it == allCfgInfo.end())
		//		{
		//			__MOUT__ << "SubSystemConfiguration not found" << std::endl;
		//
		//			xmldoc.addTextElementToData("Error", "SubSystemConfiguration not found");
		//		}
		//		else if(it->second.versions_.find(version) == it->second.versions_.end())
		//		{
		//			__MOUT__ << "Version not found" << std::endl;
		//
		//			xmldoc.addTextElementToData("Error", "Version not found");
		//		}
		//		else
		//		{
		//			//load current version
		//			bool isInConfiguration = (allCfgInfo[subAlias].configurationPtr_->isStored(version));
		//			__MOUT__ << "Version " << version << " is loaded: " <<
		//					(isInConfiguration?"YES":"NO") << std::endl;
		//
		//			if(!isInConfiguration) //load configuration view
		//				cfgMgr->getVersionedConfigurationByName(subAlias, version);
		//			else
		//				allCfgInfo[subAlias].configurationPtr_->setActiveView(version);
		//
		//			isInConfiguration = (allCfgInfo[subAlias].configurationPtr_->isStored(version));
		//			__MOUT__ << "Version " << version << " is loaded: " <<
		//					(isInConfiguration?"YES":"NO") << std::endl;
		//
		//			if(!isInConfiguration)
		//			{
		//				__MOUT__ << "Version could not be loaded" << std::endl;
		//
		//				xmldoc.addTextElementToData("Error", "Version could not be loaded");
		//			}
		//			else
		//			{
		//				int temporaryVersion = allCfgInfo[subAlias].configurationPtr_->createTemporaryView(version);
		//
		//				__MOUT__ << "\t\ttemporaryVersion: " << temporaryVersion << std::endl;
		//				ConfigurationView* cfgViewPtr = allCfgInfo[subAlias].configurationPtr_->getTemporaryView(temporaryVersion);
		//				std::vector<ViewColumnInfo> colInfo = cfgViewPtr->getColumnsInfo();
		//
		//				//while there are row entries in the data.. replace
		//				// data range from [dataOffset, dataOffset+chunkSize-1]
		//				// ... note if less rows, this means rows were deleted
		//				// ... if more, then rows were added.
		//				int r = dataOffset;
		//				int c = 0;
		//
		//				//int cellNum;
		//				//string cellStr;
		//
		//				int i = 0; //use to parse data std::string
		//				int j = data.find(',',i); //find next cell delimiter
		//				int k = data.find(';',i); //find next row delimiter
		//
		//				while(k != (int)(std::string::npos))
		//				{
		//					if(r >= (int)cfgViewPtr->getNumberOfRows())
		//					{
		//						cfgViewPtr->addRow();
		//						__MOUT__ << "Row added" << std::endl;
		//					}
		//
		//					while(j < k && j != (int)(std::string::npos))
		//					{
		//						//__MOUT__ << r << "|" << c << "][" << i << "|" << k << "][" << std::endl;
		//						//__MOUT__ << data.substr(i,j-i) << "|" << std::endl;
		//						if(colInfo[c].getViewType() == "NUMBER")
		//						{
		//							//__MOUT__ << atoi(data.substr(i,j-i).c_str()) << "|" << std::endl;
		//							cfgViewPtr->setValue(atoi(data.substr(i,j-i).c_str()),r,c);
		//							//cfgViewPtr->getValue(cellNum,r,c);
		//							//__MOUT__ << cellNum << " " << std::endl;
		//						}
		//						else
		//						{
		//							//__MOUT__ << data.substr(i,j-i) << "|" << std::endl;
		//							cfgViewPtr->setValue(data.substr(i,j-i),r,c);
		//							//cfgViewPtr->getValue(cellStr,r,c);
		//							//__MOUT__ << cellStr << " " << std::endl;
		//						}
		//						i=j+1;
		//						j = data.find(',',i); //find next cell delimiter
		//						++c;
		//					}
		//					++r;
		//					c = 0;
		//
		//
		//					i = k+1;
		//					j = data.find(',',i); //find next cell delimiter
		//					k = data.find(';',i); //find new row delimiter
		//				}
		//
		//				//delete excess rows
		//				while(r < (int)cfgViewPtr->getNumberOfRows())
		//				{
		//					cfgViewPtr->deleteRow(r);
		//					__MOUT__ << "Row deleted" << std::endl;
		//				}
		//
		//
		//				__MOUT__ << "\t\t**************************** Save as new sub-config version" << std::endl;
		//
		//				int newAssignedVersion = cfgMgr->saveNewConfiguration(subAlias,temporaryVersion);//cfgMgr->saveNewConfiguration(allCfgInfo[subAlias].configurationPtr_,temporaryVersion);
		//
		//				xmldoc.addTextElementToData("savedAlias", subAlias);
		//				sprintf(tmpIntStr,"%d",newAssignedVersion);
		//				xmldoc.addTextElementToData("savedVersion", tmpIntStr);
		//
		//				__MOUT__ << "\t\t newAssignedVersion: " << newAssignedVersion << std::endl;
		//
		//			}
		//		}
	}
	else if(Command == "changeKocVersionForSpecificConfig")
	{
		__MOUT__ << "changeKocVersionForSpecificConfig" << std::endl;

		//need 	std::string specSystemAlias = "Physics";
		//std::string KOCAlias = "FSSRDACsConfiguration";
		//int newVersion = 3;

		std::string 	specSystemAlias = CgiDataUtilities::getData(cgi,"cfgAlias"); 	//from GET
		std::string 	KOCAlias = CgiDataUtilities::getData(cgi,"subAlias"); 			//from GET
		int				newVersion = CgiDataUtilities::getDataAsInt(cgi,"version");		//from GET

		//TODO
		std::string chosenSubConfig;
		std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();

		{

			std::map<std::string, ConfigurationKey> aliasMap = cfgMgr->__GET_CONFIG__(ConfigurationAliases)->getAliasesMap();

			std::map<std::string, ConfigurationKey>::const_iterator it = aliasMap.find(specSystemAlias);
			if(it != aliasMap.end())
			{
				__MOUT__ << "Alias: " << it->first << " - Key: " << it->second.key() << std::endl;

				__MOUT__ << "Alias exists: " << specSystemAlias << std::endl;
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
						temporaryVersion,specSystemAlias,KOCAlias,ConfigurationVersion(newVersion));

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
				__MOUT__ << "Alias doesnt exist: " << specSystemAlias << std::endl;

			//reply with resulting sub system config
			fillSpecificSystemXML(xmldoc,cfgMgr,specSystemAlias,ConfigurationVersion(ConfigurationVersion::DEFAULT)); //FIXME.. should not always be DEFAULT!! should come from request

		}

	}
	else if(Command == "getTreeView")
	{
		//parameters
		//	globalConfigName (possibly version?)
		//	starting node path
		//	depth from starting node path

		//return xml
		//	<tree>
		//	<node="...">
		//		<node="...">
		//			<node="...">
		//				<value="...">
		//			</node>
		//			<node="...">
		//				<value="...">
		//			</node>
		//		</node>
		//		<node="...">
		//			<value="..">
		//		</node>
		//		...
		//	</node>
		//	</tree>

		std::string 	globalConfig 	= CgiDataUtilities::getData(cgi,"globalConfig");
		std::string 	startPath 		= CgiDataUtilities::postData(cgi,"startPath");
		int				depth	 		= CgiDataUtilities::getDataAsInt(cgi,"depth");

		__MOUT__ << "globalConfig: " << globalConfig << std::endl;
		__MOUT__ << "startPath: " << startPath << std::endl;
		__MOUT__ << "depth: " << depth << std::endl;
		fillTreeView(xmldoc,cfgMgr,globalConfig,startPath,depth);
	}
	else if(Command == "activateGlobalConfig")
	{
		std::string 	globalConfig 	= CgiDataUtilities::getData(cgi,"globalConfig");
		__MOUT__ << "Activating config: " << globalConfig << std::endl;
		if(globalConfig == "")
			globalConfig = cfgMgr->getActiveGlobalConfiguration();
		else
			globalConfig = cfgMgr->setActiveGlobalConfiguration(globalConfig);
		xmldoc.addTextElementToData("activeConfig", globalConfig);
	}
	else
		__MOUT__ << "Command request not recognized." << std::endl;



	//return xml doc holding server response
	xmldoc.outputXmlDocument((std::ostringstream*) out, true);
}

//========================================================================================================================
//fillTreeView
//	returns xml tree from path for given depth
void ConfigurationGUISupervisor::fillTreeView(HttpXmlDocument &xmldoc, ConfigurationManagerRW *cfgMgr, const std::string &globalConfig,
		const std::string &startPath, int depth)
{
	//return xml
	//	<globalConfig="globalConfig"/>
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
	xmldoc.addTextElementToData("globalConfig", globalConfig);

	try {
		cfgMgr->loadGlobalConfiguration(globalConfig);

		DOMElement* parentEl = xmldoc.addTextElementToData("tree", startPath);

		if(depth == 0) return; //already returned root node in itself

		std::map<std::string,ConfigurationTree> rootMap;
		if(startPath == "/") //then consider the configurationManager the root node
			rootMap = cfgMgr->getChildren();
		else
			rootMap = cfgMgr->getNode(startPath).getChildren();

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
	if(t.isValueNode())
	{
		parentEl = xmldoc.addTextElementToParent("node", t.getValueName(), parentEl);
		xmldoc.addTextElementToParent("value", t.getValueAsString(), parentEl);
	}
	else
	{
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
//fillSpecificSystemXML
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
void ConfigurationGUISupervisor::fillSpecificSystemXML(HttpXmlDocument &xmldoc,
		ConfigurationManagerRW *cfgMgr, const std::string &alias, ConfigurationVersion backboneVersion)
{
	char tmpIntStr[100];
	DOMElement* parentEl;

	std::map<std::string, ConfigurationKey> aliasMap = cfgMgr->__GET_CONFIG__(ConfigurationAliases)->getAliasesMap();

	std::map<std::string, ConfigurationKey>::const_iterator it = aliasMap.find(alias);
	if(it != aliasMap.end())
	{
		xmldoc.addTextElementToData("SystemConfigurationAlias", it->first);

		std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
		std::set<ConfigurationVersion> versions;

		std::set<std::string> listOfKocs;

		DOMElement* parentElKoc;

		//get all KOC alias and version numbers

		//__MOUT__ << "Alias: " << it->first << " - Key: " << it->second.key() << std::endl;

		//add system configuration alias and key
		xmldoc.addTextElementToData("SystemConfigurationAlias", it->first);
		sprintf(tmpIntStr,"%u",it->second.key());
		xmldoc.addTextElementToData("SystemConfigurationKey", tmpIntStr);
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

				xmldoc.addTextElementToParent("KOC_alias", *sit, parentEl);
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

				std::map<std::string, ConfigurationKey> aliasMap = cfgMgr->__GET_CONFIG__(ConfigurationAliases)->getAliasesMap();

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

//========================================================================================================================
//fillSpecificSubSystemXML
//
//	if isTemporaryVersion(), default to latest version
//	if INVALID or version does not exists, default to mock-up
void ConfigurationGUISupervisor::fillSpecificSubSystemXML(HttpXmlDocument &xmldoc,
		ConfigurationManagerRW *cfgMgr, const std::string &subAlias, ConfigurationVersion version)
{
	char tmpIntStr[100];
	DOMElement* parentEl;

	//verify alias and version exists
	std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
	std::map<std::string, ConfigurationInfo>::const_iterator it = allCfgInfo.find(subAlias);

	if(it == allCfgInfo.end())
	{
		__MOUT__ << "SubSystemConfiguration not found: " << subAlias << std::endl;
		//should never happen
		return;
	}

	if(version.isTemporaryVersion() == -2 &&  //take latest version
			it->second.versions_.size())
	{
		version = *(it->second.versions_.rbegin());
		__MOUT__ << "Using latest version: " << version << std::endl;
	}
	else if(it->second.versions_.find(version) == it->second.versions_.end())
	{
		__MOUT__ << "Version not found, so using mockup." << std::endl;
		version = ConfigurationVersion(); //use INVALID
	}

	//table name
	xmldoc.addTextElementToData("SubSystemConfigurationAlias", it->first);
	//table version
	xmldoc.addTextElementToData("SubSystemConfigurationVersion", version.toString());
	//existing table versions
	parentEl = xmldoc.addTextElementToData("SubSystemConfigurationVersions", "");
	for (auto &v:it->second.versions_)
		xmldoc.addTextElementToParent("VersionKey", v.toString(), parentEl);

	//table columns and then rows (from config view)
	{
		ConfigurationView* cfgViewPtr;
		if(version != ConfigurationVersion::INVALID)
		{
			//load current version
			bool isInConfiguration = (allCfgInfo[subAlias].configurationPtr_->isStored(version));
			__MOUT__ << "Version " << version << " is loaded: " <<
					(isInConfiguration?"YES":"NO") << std::endl;

			if(!isInConfiguration) //load configuration view
				cfgMgr->getVersionedConfigurationByName(subAlias, version);
			else
				allCfgInfo[subAlias].configurationPtr_->setActiveView(version);

			isInConfiguration = (allCfgInfo[subAlias].configurationPtr_->isStored(version));
			__MOUT__ << "Version " << version << " is loaded: " <<
					(isInConfiguration?"YES":"NO") << std::endl;

			if(!isInConfiguration)
			{
				__MOUT_ERR__ << "Version could not be loaded" << std::endl;
				return;
			}
			__MOUT__ << "\t\t******** view " <<
					allCfgInfo[subAlias].configurationPtr_->getViewVersion() << std::endl;

			cfgViewPtr = allCfgInfo[subAlias].configurationPtr_->getViewP();
		}
		else //use mockup version
		{
			__MOUT__ << "\t\t******** view mockup" << std::endl;
			cfgViewPtr = allCfgInfo[subAlias].configurationPtr_->getMockupViewP();
		}


		//get 'columns' of sub config view


		parentEl = xmldoc.addTextElementToData("CurrentVersionColumnHeaders", "");
		std::vector<ViewColumnInfo> colInfo = cfgViewPtr->getColumnsInfo();
		for(int i=0;i<(int)colInfo.size();++i)	//column headers and types
		{
			__MOUT__ << "\t\tCol " << i << ": " << colInfo[i].getType()  << "() " <<
					colInfo[i].getName() << " "
					<< colInfo[i].getStorageName() << " " << colInfo[i].getDataType() << std::endl;

			xmldoc.addTextElementToParent("ColumnHeader", colInfo[i].getName(), parentEl);
			xmldoc.addTextElementToParent("ColumnType", colInfo[i].getDataType(), parentEl);
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
					//__MOUT__ << "\t " << num << std::endl;

					sprintf(tmpIntStr,"%d",num);
					xmldoc.addTextElementToParent("Entry", tmpIntStr, tmpParentEl);
				}
				else
				{
					std::string val;
					cfgViewPtr->getValue(val,r,c);
					//__MOUT__ << "\t " << val << std::endl;

					xmldoc.addTextElementToParent("Entry", val, tmpParentEl);
				}
		}
	}
}


//========================================================================================================================
//saveSpecificSubSystemVersion
//
//	if starting version is -1 start from mock-up
void ConfigurationGUISupervisor::saveSpecificSubSystemVersion(HttpXmlDocument &xmldoc,
		ConfigurationManagerRW *cfgMgr, const std::string &subAlias, ConfigurationVersion version,
		const std::string &data, const int &dataOffset, const int &chunkSize)
{
	char tmpIntStr[100];

	//verify alias and version exists
	std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
	std::map<std::string, ConfigurationInfo>::const_iterator it = allCfgInfo.find(subAlias);


	__MOUT__ << "getSpecificSubSystemConfiguration: " << subAlias << " version: " << version
			<< " chunkSize: " << chunkSize << " dataOffset: " << dataOffset << std::endl;

	__MOUT__ << "data: " << data << std::endl;

	if(it == allCfgInfo.end())
	{
		__MOUT_ERR__ << "SubSystemConfiguration not found" << std::endl;

		xmldoc.addTextElementToData("Error", "SubSystemConfiguration not found");
		return;
	}
	else if(!version.isInvalid() && //-1 mock-up view "always" exists
			it->second.versions_.find(version) == it->second.versions_.end())
	{
		__MOUT_ERR__ << "Version not found" << std::endl;

		xmldoc.addTextElementToData("Error", "Version not found");
		return;
	}


	{
		//if starting version is not mockup, make sure it is loaded
		if(!version.isInvalid())
		{
			//load current version
			bool isInConfiguration = (allCfgInfo[subAlias].configurationPtr_->isStored(version));
			__MOUT__ << "Version " << version << " is loaded: " <<
					(isInConfiguration?"YES":"NO") << std::endl;

			if(!isInConfiguration) //load configuration view
				cfgMgr->getVersionedConfigurationByName(subAlias, version);

			isInConfiguration = (allCfgInfo[subAlias].configurationPtr_->isStored(version));
			__MOUT__ << "Version " << version << " is loaded: " <<
					(isInConfiguration?"YES":"NO") << std::endl;

			if(!isInConfiguration)
			{
				__MOUT_ERR__ << "Version could not be loaded" << std::endl;
				xmldoc.addTextElementToData("Error", "Version could not be loaded");
				return;
			}
			__MOUT__ << "\t\t******** view " <<
					allCfgInfo[subAlias].configurationPtr_->getViewVersion() << std::endl;

		}


		//create temporary version from starting version
		ConfigurationVersion temporaryVersion = allCfgInfo[subAlias].configurationPtr_->createTemporaryView(version);

		__MOUT__ << "\t\ttemporaryVersion: " << temporaryVersion << std::endl;

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
				__MOUT__ << "Row added" << std::endl;
			}

			while(j < k && j != (int)(std::string::npos))
			{
				//__MOUT__ << r << "|" << c << "][" << i << "|" << k << "][" << std::endl;
				//__MOUT__ << data.substr(i,j-i) << "|" << std::endl;
				if(colInfo[c].getDataType() == "NUMBER")
				{
					//__MOUT__ << atoi(data.substr(i,j-i).c_str()) << "|" << std::endl;
					cfgViewPtr->setValue(atoi(data.substr(i,j-i).c_str()),r,c);
					//cfgViewPtr->getValue(cellNum,r,c);
					//__MOUT__ << cellNum << " " << std::endl;
				}
				else
				{
					//__MOUT__ << data.substr(i,j-i) << "|" << std::endl;
					cfgViewPtr->setValue(data.substr(i,j-i),r,c);
					//cfgViewPtr->getValue(cellStr,r,c);
					//__MOUT__ << cellStr << " " << std::endl;
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
			__MOUT__ << "Row deleted" << std::endl;
		}


		__MOUT__ << "\t\t**************************** Save as new sub-config version" << std::endl;

		ConfigurationVersion newAssignedVersion = cfgMgr->saveNewConfiguration(subAlias,temporaryVersion);//cfgMgr->saveNewConfiguration(allCfgInfo[subAlias].configurationPtr_,temporaryVersion);

		xmldoc.addTextElementToData("savedAlias", subAlias);
		xmldoc.addTextElementToData("savedVersion", newAssignedVersion.toString());

		__MOUT__ << "\t\t newAssignedVersion: " << newAssignedVersion << std::endl;

	}
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
	__MOUT__ << mapKey << " ... current size: " << userConfigurationManagers_.size() << std::endl;
	//create new config mgr if not one for active session index
	if(userConfigurationManagers_.find(mapKey) == userConfigurationManagers_.end())
	{
		userConfigurationManagers_[mapKey] = new ConfigurationManagerRW();

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
			__MOUT__ << now << ":" << it->second << " = " << now - it->second << std::endl;
			delete userConfigurationManagers_[it->first]; //call destructor
			assert(userConfigurationManagers_.erase(it->first));	//erase by key
			userLastUseTime_.erase(it);								//erase by iterator

			it=userLastUseTime_.begin(); //fail safe.. reset it, to avoid trying to understand what happens with the next iterator
		}

	return userConfigurationManagers_[mapKey];
}



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

	ConfigurationManagerRW cfgMgrInst;

	ConfigurationManagerRW *cfgMgr = &cfgMgrInst;

	std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
	__MOUT__ << "allCfgInfo.size() = " << allCfgInfo.size() << std::endl;
	for(auto& mapIt : allCfgInfo)
	{
		__MOUT__ << "Config Alias: " << mapIt.first << std::endl;
		//NOTE: MessageFacility has strange bug.. where it clips of the message and adds a P
		//		__MOUT__ << "Config Aliasve: " <<
		//					mapIt.first << std::endl;
		//		std::stringstream ss;
		//		ss << "Config Aliassss: " << mapIt.first << std::endl;
		//		__MOUT__ << ss.str();
		//		__MOUT__ << ss.str();
		//		__MOUT__ << ss.str();
		//		__MOUT__ << "Config Aliassss: " << mapIt.first << std::endl;
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






