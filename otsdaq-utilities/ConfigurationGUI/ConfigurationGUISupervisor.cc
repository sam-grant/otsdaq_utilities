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

#define __MF_SUBJECT__ "CfgGUI"
#define __MF_HDR__		__COUT_HDR_FL__
#define __MOUT_ERR__  	mf::LogError	(__MF_SUBJECT__) << __MF_HDR__
#define __MOUT_WARN__  	mf::LogWarning	(__MF_SUBJECT__) << __MF_HDR__
#define __MOUT_INFO__  	mf::LogInfo		(__MF_SUBJECT__) << __COUT_HDR__
#define __MOUT__  		mf::LogDebug	(__MF_SUBJECT__) << __MF_HDR__



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


	//how do we know which version numbers of a KOC exist already?
		//getAllConfigurationInfo fills the version list for each KOC found in "Configurations"

	//FIXME TODO
	//LEFT OFF - Proof of Concept
	//	Choose a sub-config
	//	Make a copy of a view
	//	Edit copy
	//	Save as new sub-config version



	std::cout << __COUT_HDR_FL__ << "comment/uncomment here for debugging Configuration!" << std::endl;

	__MOUT__ << "To prove the concept...";
	return;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//behave like a new user

	std::string userConfigurationManagerIndex = "1";
	userConfigurationManagers_["1"] = new ConfigurationManagerWithWriteAccess();
	userConfigurationManagers_["2"] = new ConfigurationManagerWithWriteAccess();
	ConfigurationManagerWithWriteAccess *cfgMgr = userConfigurationManagers_[userConfigurationManagerIndex];

	std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
	std::cout << __COUT_HDR_FL__ << "All config info loaded." << std::endl;
	std::map<std::string, ConfigurationKey>	aliasMap = cfgMgr->getConfiguration<ConfigurationAliases>()->getAliasesMap();


	std::cout << __COUT_HDR_FL__ << "aliasMap size: " << aliasMap.size() << std::endl;
	std::cout << __COUT_HDR_FL__ << "getAllConfigurationInfo size: " << allCfgInfo.size() << std::endl;


	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//for each configuration alias and key
		//get KOC version numbers (this is the version "conditioned" by the alias-key pair)

	std::set<std::string> listOfKocs;
	std::map<std::string, ConfigurationKey>::const_iterator it = aliasMap.begin();
	while (it != aliasMap.end())
	{

		std::cout << __COUT_HDR_FL__ << "Alias: " << it->first << " - Key: " << it->second.key() << std::endl;

		listOfKocs = cfgMgr->getConfiguration<Configurations>()->getListOfKocs(it->second.key());
		std::cout << __COUT_HDR_FL__ << "\tKocs size: " << listOfKocs.size() << std::endl;

		for (auto& koc : listOfKocs)
		{
			unsigned int conditionVersion = cfgMgr->getConfiguration<Configurations>()->getConditionVersion(it->second.key(),koc);

			std::cout << __COUT_HDR_FL__ << "\tKoc: " << koc << " Version: " << conditionVersion << std::endl;
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
		std::cout << __COUT_HDR_FL__ << "KOC Alias: " << mapIt->first << std::endl;
		std::cout << __COUT_HDR_FL__ << "\t\tExisting Versions: " << mapIt->second.versions_.size() << std::endl;

		//get version key for the current system subconfiguration key
		for (std::set<int>::iterator vit=mapIt->second.versions_.begin(); vit!=mapIt->second.versions_.end(); ++vit)
		{
			std::cout << __COUT_HDR_FL__ << "\t\t" << *vit << std::endl;
		}
		++mapIt;
	}
	//return;


	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//Choose a sub config
	mf::LogDebug("cfgGUI");
	std::cout << __COUT_HDR_FL__ << "\t\t**************************** Choose a sub config" << std::endl;
	std::string chosenSubConfig = "FSSRDACsConfiguration"; //must be less than allCfgInfo.size()

	{
		int versionToCopy = 0; //-1 is empty, //must be less than allCfgInfo[chosenSubConfig].versions_.size()

		//check if is an existing version
		bool isInDatabase = allCfgInfo[chosenSubConfig].versions_.find(versionToCopy) != allCfgInfo[chosenSubConfig].versions_.end();
		std::cout << __COUT_HDR_FL__ << "Version " << versionToCopy << " is in database: " <<
				(isInDatabase?"YES":"NO") << std::endl;

		//check if version is already loaded
		bool isInConfiguration = (allCfgInfo[chosenSubConfig].configurationPtr_->isStored(versionToCopy));
		std::cout << __COUT_HDR_FL__ << "Version " << versionToCopy << " is loaded: " <<
				(isInConfiguration?"YES":"NO") << std::endl;

		//load configuration view and set as active view
		if(!isInConfiguration)
			cfgMgr->getVersionedConfigurationByName(chosenSubConfig, versionToCopy);
		else
			allCfgInfo[chosenSubConfig].configurationPtr_->setActiveView(versionToCopy);

		//verify version is loaded now
		isInConfiguration = (allCfgInfo[chosenSubConfig].configurationPtr_->isStored(versionToCopy));
		std::cout << __COUT_HDR_FL__ << "Version " << versionToCopy << " is loaded: " <<
				(isInConfiguration?"YES":"NO") << std::endl;
	}

	//return;


	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//view this version
	//general base class
	{
		//get 'columns' of sub config

		std::cout << __COUT_HDR_FL__ << "\t\t******** view version " <<
				allCfgInfo[chosenSubConfig].configurationPtr_->getViewVersion() << std::endl;
		ConfigurationView* cfgViewPtr = allCfgInfo[chosenSubConfig].configurationPtr_->getViewP();

		std::vector<ViewColumnInfo> colInfo = cfgViewPtr->getColumnsInfo();

		std::cout << __COUT_HDR_FL__ << "\t\tNumber of Cols " << colInfo.size() << std::endl;
		std::cout << __COUT_HDR_FL__ << "\t\tNumber of Rows " << cfgViewPtr->getNumberOfRows() << std::endl;
	}

	//return;


	std::cout << __COUT_HDR_FL__ << "\t\t**************************** Choose a different sub config" << std::endl;

	{
		int versionToCopy = 2; //-1 is empty, //must be less than allCfgInfo[chosenSubConfig].versions_.size()

		//check if is an existing version
		bool isInDatabase = allCfgInfo[chosenSubConfig].versions_.find(versionToCopy) != allCfgInfo[chosenSubConfig].versions_.end();
		std::cout << __COUT_HDR_FL__ << "Version " << versionToCopy << " is in database: " <<
				(isInDatabase?"YES":"NO") << std::endl;

		//check if version is already loaded
		bool isInConfiguration = (allCfgInfo[chosenSubConfig].configurationPtr_->isStored(versionToCopy));
		std::cout << __COUT_HDR_FL__ << "Version " << versionToCopy << " is loaded: " <<
				(isInConfiguration?"YES":"NO") << std::endl;

		//load configuration view and set as active view
		if(!isInConfiguration)
			cfgMgr->getVersionedConfigurationByName(chosenSubConfig, versionToCopy);
		else
			allCfgInfo[chosenSubConfig].configurationPtr_->setActiveView(versionToCopy);

		//verify version is loaded now
		isInConfiguration = (allCfgInfo[chosenSubConfig].configurationPtr_->isStored(versionToCopy));
		std::cout << __COUT_HDR_FL__ << "Version " << versionToCopy << " is loaded: " <<
				(isInConfiguration?"YES":"NO") << std::endl;

	}

	{
		//get 'columns' of sub config

		std::cout << __COUT_HDR_FL__ << "\t\t******** view version " <<
				allCfgInfo[chosenSubConfig].configurationPtr_->getViewVersion() << std::endl;
		ConfigurationView* cfgViewPtr = allCfgInfo[chosenSubConfig].configurationPtr_->getViewP();

		std::vector<ViewColumnInfo> colInfo = cfgViewPtr->getColumnsInfo();

		std::cout << __COUT_HDR_FL__ << "\t\tNumber of Cols " << colInfo.size() << std::endl;
		std::cout << __COUT_HDR_FL__ << "\t\tNumber of Rows " << cfgViewPtr->getNumberOfRows() << std::endl;
	}

	//return;

	std::cout << __COUT_HDR_FL__ << "\t\t**************************** Choose a different sub config" << std::endl;

	{
		int versionToCopy = 0; //-1 is empty, //must be less than allCfgInfo[chosenSubConfig].versions_.size()
		//check if is an existing version
		bool isInDatabase = allCfgInfo[chosenSubConfig].versions_.find(versionToCopy) != allCfgInfo[chosenSubConfig].versions_.end();
		std::cout << __COUT_HDR_FL__ << "Version " << versionToCopy << " is in database: " <<
				(isInDatabase?"YES":"NO") << std::endl;

		//check if version is already loaded
		bool isInConfiguration = (allCfgInfo[chosenSubConfig].configurationPtr_->isStored(versionToCopy));
		std::cout << __COUT_HDR_FL__ << "Version " << versionToCopy << " is loaded: " <<
				(isInConfiguration?"YES":"NO") << std::endl;

		//load configuration view and set as active view
		if(!isInConfiguration)
			cfgMgr->getVersionedConfigurationByName(chosenSubConfig, versionToCopy);
		else
			allCfgInfo[chosenSubConfig].configurationPtr_->setActiveView(versionToCopy);

		//verify version is loaded now
		isInConfiguration = (allCfgInfo[chosenSubConfig].configurationPtr_->isStored(versionToCopy));
		std::cout << __COUT_HDR_FL__ << "Version " << versionToCopy << " is loaded: " <<
				(isInConfiguration?"YES":"NO") << std::endl;
	}

	{
		//get 'columns' of sub config

		std::cout << __COUT_HDR_FL__ << "\t\t******** view version " <<
				allCfgInfo[chosenSubConfig].configurationPtr_->getViewVersion() << std::endl;
		ConfigurationView* cfgViewPtr = allCfgInfo[chosenSubConfig].configurationPtr_->getViewP();

		std::vector<ViewColumnInfo> colInfo = cfgViewPtr->getColumnsInfo();

		std::cout << __COUT_HDR_FL__ << "\t\tNumber of Cols " << colInfo.size() << std::endl;
		std::cout << __COUT_HDR_FL__ << "\t\tNumber of Rows " << cfgViewPtr->getNumberOfRows() << std::endl;
	}

	//return;

	int versionToCopy = 0;
	//	Make a copy of a view
	std::cout << __COUT_HDR_FL__ << "\t\t**************************** Make a copy of a view" << std::endl;
	int temporaryVersion = allCfgInfo[chosenSubConfig].configurationPtr_->createTemporaryView(versionToCopy);
	std::cout << __COUT_HDR_FL__ << "\t\ttemporaryVersion: " << temporaryVersion << std::endl;


	//	Edit copy
	std::cout << __COUT_HDR_FL__ << "\t\t**************************** Edit copy" << std::endl;

	//view temp version
	//general base class
	{
		//get 'columns' of sub config

		std::cout << __COUT_HDR_FL__ << "\t\t******** Before change" << std::endl;
		ConfigurationView* cfgViewPtr = allCfgInfo[chosenSubConfig].configurationPtr_->getTemporaryView(temporaryVersion);

		std::vector<ViewColumnInfo> colInfo = cfgViewPtr->getColumnsInfo();

		std::stringstream ss;
		cfgViewPtr->print(ss);
		std::cout << __COUT_HDR_FL__ << ss.str() << std::endl;


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
		std::cout << __COUT_HDR_FL__ << "\t\t******** After change" << std::endl;



		std::stringstream ss2;
		cfgViewPtr->print(ss2);
		std::cout << __COUT_HDR_FL__ << ss2.str() << std::endl;


		// add a new col for example
		//		int cret = cfgViewPtr->addColumn("TestCol","TEST_COL","TIMESTAMP WITH TIMEZONE");
		//		std::cout << __COUT_HDR_FL__ << "\t\t******** After change col: " << cret << std::endl;
		//		colInfo = cfgViewPtr->getColumnsInfo();
		//		std::cout << __COUT_HDR_FL__ << "\t\tNumber of Cols " << colInfo.size() << std::endl;
		//		std::cout << __COUT_HDR_FL__ << "\t\tNumber of Rows " << cfgViewPtr->getNumberOfRows() << std::endl;
		//
		//		for(int r=0;r<(int)cfgViewPtr->getNumberOfRows();++r)
		//		{
		//			std::cout << __COUT_HDR_FL__ << "\t\tRow " << r << ": "  << std::endl;
		//
		//			for(int c=0;c<(int)cfgViewPtr->getNumberOfColumns();++c)
		//				if(colInfo[c].getViewType() == "NUMBER")
		//				{
		//					int num;
		//					cfgViewPtr->getValue(num,r,c);
		//					std::cout << __COUT_HDR_FL__ << "\t " << num << std::endl;
		//				}
		//				else
		//				{
		//				std::string val;
		//					cfgViewPtr->getValue(val,r,c);
		//					std::cout << __COUT_HDR_FL__ << "\t " << val << std::endl;
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
	//			std::cout << __COUT_HDR_FL__ << "\t\tDac Name: " << rdit->first << " - Addr: " << rdit->second.first << ", Val: " << rdit->second.second << std::endl;
	//			rocDacs.setDAC(rdit->first,7,1);
	//
	//			rdit++;
	//		}
	//	}

	//	Save as new sub-config version
	std::cout << __COUT_HDR_FL__ << "\t\t**************************** Save as new sub-config version" << std::endl;

	int newAssignedVersion = 0;//cfgMgr->saveNewConfiguration(allCfgInfo[chosenSubConfig].configurationPtr_,temporaryVersion);
	//int newAssignedVersion = cfgMgr->saveNewConfiguration(allCfgInfo[chosenSubConfig].configurationPtr_,temporaryVersion);

	std::cout << __COUT_HDR_FL__ << "\t\t newAssignedVersion: " << newAssignedVersion << std::endl;

	//return;



	//proof of concept.. change a KOC version (this makes a new backbone version?!)
	std::cout << __COUT_HDR_FL__ << "\t\t**************************** Edit a KOC for a System Alias" << std::endl;

	std::stringstream ss;
	cfgMgr->getConfiguration<ConfigurationAliases>()->print(ss);
	cfgMgr->getConfiguration<DefaultConfigurations>()->print(ss);
	cfgMgr->getConfiguration<Configurations>()->print(ss);
	cfgMgr->getConfiguration<VersionAliases>()->print(ss);
	std::cout << __COUT_HDR_FL__ << ss.str() << std::endl;

	{
		std::string specSystemAlias = "Physics";
		std::string KOCAlias = "FSSRDACsConfiguration";
		int newVersion = 3;

		std::map<std::string, ConfigurationKey> aliasMap = cfgMgr->getConfiguration<ConfigurationAliases>()->getAliasesMap();

		std::map<std::string, ConfigurationKey>::const_iterator it = aliasMap.find(specSystemAlias);
		if(it != aliasMap.end())
		{
			std::cout << __COUT_HDR_FL__ << "Alias: " << it->first << " - Key: " << it->second.key() << std::endl;

			std::cout << __COUT_HDR_FL__ << "Alias exists: " << specSystemAlias << std::endl;
			std::cout << __COUT_HDR_FL__ << "Sub system alias: " << KOCAlias << std::endl;
			std::cout << __COUT_HDR_FL__ << "Changing to new version: " << newVersion << std::endl;

			std::set<std::string> listOfKocs;
			listOfKocs = cfgMgr->getConfiguration<Configurations>()->getListOfKocs(it->second.key());
			for (std::set<std::string>::iterator sit=listOfKocs.begin(); sit!=listOfKocs.end(); ++sit)
			{
				unsigned int cv = cfgMgr->getConfiguration<Configurations>()->getConditionVersion(it->second.key(),*sit);

				std::cout << __COUT_HDR_FL__ << "\tKoc: " << *sit << " Version: " << cv << std::endl;

				std::set<int> versions = allCfgInfo.find(*sit)->second.versions_;
				std::cout << __COUT_HDR_FL__ << "\t\tAll versions: " << std::endl;
				for (std::set<int>::iterator vit=versions.begin(); vit!=versions.end(); ++vit)
					std::cout << __COUT_HDR_FL__ << " " << *vit << std::endl;
				
			}


			//Make copy for all backbone members to temporary versions!
			std::cout << __COUT_HDR_FL__ << "\t\t**************************** Make temporary backbone" << std::endl;

			chosenSubConfig = "ConfigurationAliases";
			int temporaryVersion;
			int versionToCopy = cfgMgr->getConfiguration<ConfigurationAliases>()->getViewVersion();
			//cfgMgr->__GET_CONFIG__(DefaultConfigurations)->print();
			///////////////////////cfgMgr->getConfiguration<Configurations>()->print();
			//cfgMgr->__GET_CONFIG__(VersionAliases)

			std::cout << __COUT_HDR_FL__ << "\t\ttemporaryVersion versionToCopy: " << versionToCopy << std::endl;

			assert(allCfgInfo.find(chosenSubConfig) != allCfgInfo.end());

			bool isInDatabase = allCfgInfo[chosenSubConfig].versions_.find(versionToCopy) != allCfgInfo[chosenSubConfig].versions_.end();
			std::cout << __COUT_HDR_FL__ << "Version " << versionToCopy << " is in database: " <<
					(isInDatabase?"YES":"NO") << "     ";

			//temporaryVersion = allCfgInfo[chosenSubConfig].configurationPtr_->createTemporaryView(versionToCopy);
			temporaryVersion = cfgMgr->createTemporaryBackboneView(versionToCopy);
			std::cout << __COUT_HDR_FL__ << "\t\ttemporaryVersion Backbone: " << temporaryVersion << std::endl;
			std::cout << __COUT_HDR_FL__ << "\t\t(Note: it is not the) active version Backbone: " <<
					cfgMgr->getConfiguration<ConfigurationAliases>()->getViewVersion()  << std::endl;


			//edit the backbone however you want
			std::cout << __COUT_HDR_FL__ << "\t\t**************************** Make changes to backbone" << std::endl;

			ConfigurationView* cfgViewPtr = allCfgInfo[chosenSubConfig].configurationPtr_->getTemporaryView(temporaryVersion);


			std::cout << __COUT_HDR_FL__ << "\t\t******** Before change" << std::endl;
			std::stringstream ss;
			cfgViewPtr->print(ss);
			std::cout << __COUT_HDR_FL__ << ss.str() << std::endl;


			cfgMgr->setKOCVersionForSpecificConfiguration(temporaryVersion,specSystemAlias,KOCAlias,newVersion);

			std::cout << __COUT_HDR_FL__ << "\t\t******** After change" << std::endl;
			ss.clear();
			cfgViewPtr->print(ss);
			std::cout << __COUT_HDR_FL__ << ss.str() << std::endl;

			//FSSRDACsConfiguration


		}
		else
			std::cout << __COUT_HDR_FL__ << "Alias doesnt exist: " << specSystemAlias << std::endl;




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

		std::cout << __COUT_HDR_FL__ << "Alias: " << it->first << " - Key: " << it->second.key() << std::endl;

		//cfgMgr->loadConfiguration(cfgMgr->theConfigurations_,&(it->second));
		//cfgMgr->loadConfiguration(cfgMgr->theDefaultConfigurations_,&(it->second));
		//cfgMgr->loadConfiguration(cfgMgr->theVersionAliases_,&(it->second));
	    //cfgMgr->setupConfigurationGUI(&(it->second));	//load

		if(cfgMgr->getConfigurations())	// get Kocs
		{
			listOfKocs = cfgMgr->getConfigurations()->getListOfKocs(it->second.key());
			std::cout << __COUT_HDR_FL__ << "\tKocs size: " << listOfKocs.size() << std::endl;


			for (std::set<std::string>::iterator sit=listOfKocs.begin(); sit!=listOfKocs.end(); ++sit)
			{
				unsigned int cv = cfgMgr->getConfigurations()->getConditionVersion(it->second.key(),*sit);

				std::cout << __COUT_HDR_FL__ << "\tKoc: " << *sit << " Version: " << cv << std::endl;

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

			   std::cout << __COUT_HDR_FL__ << "\tFssr Name: " << dit->first << " - Row: " << dit->second << std::endl;



				// get FSSR Configs
			   	if(cfgMgr->getFSSRDACsConfiguration())
			   	{
			   		ROCDACs	rocDacs = cfgMgr->getFSSRDACsConfiguration()->getROCDACs(dit->first);

			   		DACList	dacs  = rocDacs.getDACList(); //map of std::string to pair (int, int)


			   		DACList::const_iterator rdit = dacs.begin();
			   		while(rdit != dacs.end())
			   		{
						   std::cout << __COUT_HDR_FL__ << "\t\tDac Name: " << rdit->first << " - Addr: " << rdit->second.first << ", Val: " << rdit->second.second << std::endl;
							rocDacs.setDAC(rdit->first,7,1);

						   rdit++;

			   		}


			   		dacs  = rocDacs.getDACList(); //map of std::string to pair (int, int)
					rdit = dacs.begin();
					while(rdit != dacs.end())
					{
						   std::cout << __COUT_HDR_FL__ << "\t\tDac Name: " << rdit->first << " - Addr: " << rdit->second.first << ", Val: " << rdit->second.second << std::endl;
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
	std::cout << __COUT_HDR_FL__ << "done getAliasList" << std::endl;


	//clear config managers
	for (std::map<std::string, ConfigurationManagerWithWriteAccess *> ::iterator it=userConfigurationManagers_.begin(); it!=userConfigurationManagers_.end(); ++it)
	{
		std::cout << __COUT_HDR_FL__ << it->first << std::endl;
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
	for (std::map<std::string, ConfigurationManagerWithWriteAccess *> ::iterator it=userConfigurationManagers_.begin(); it!=userConfigurationManagers_.end(); ++it)
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

	std::cout << __COUT_HDR_FL__ << "Command " << Command << " files: " << cgi.getFiles().size() << std::endl;

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
		std::cout << __COUT_HDR_FL__ << "Invalid Cookie Code" << std::endl;
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
		std::cout << __COUT_HDR_FL__ << "User " << username << " is locked out. " << userWithLock << " has lock." << std::endl;
		return;
	}
	//**** end LOCK GATEWAY CODE ***//

	if(userPermissions < USER_PERMISSIONS_THRESHOLD)
	{
		*out << RemoteWebUsers::REQ_NO_PERMISSION_RESPONSE;
		std::cout << __COUT_HDR_FL__ << "User " << username << " has insufficient permissions: " << userPermissions << "." << std::endl;
		return;
	}

	HttpXmlDocument xmldoc(cookieCode);

	//acquire user's configuration manager based on username & activeSessionIndex

	std::string  backboneVersionStr = cgi("backboneVersion");		  	//from GET
	int		backboneVersion = (backboneVersionStr == "")?-1:atoi(backboneVersionStr.c_str()); //default to latest
	std::cout << __COUT_HDR_FL__ << "ConfigurationManagerWithWriteAccess backboneVersion Version req \t\t" << backboneVersionStr << std::endl;
	ConfigurationManagerWithWriteAccess* cfgMgr = refreshUserSession(username, activeSessionIndex, backboneVersion);
	std::cout << __COUT_HDR_FL__ << "ConfigurationManagerWithWriteAccess backboneVersion Version Loaded \t\t" << backboneVersion << std::endl;

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

			//std::cout << __COUT_HDR_FL__ << "Alias: " << it->first << " - Key: " << it->second.key() << std::endl;

			//add system configuration alias and key
			xmldoc.addTextElementToData("SystemConfigurationAlias", it->first);
			sprintf(tmpIntStr,"%u",it->second.key());
			xmldoc.addTextElementToData("SystemConfigurationKey", tmpIntStr);
			parentEl = xmldoc.addTextElementToData("SystemConfigurationKOCs", "");

			//get KOCs alias and version for the current system configuration key
			assert(cfgMgr->getConfiguration<Configurations>());
			{
				listOfKocs = cfgMgr->getConfiguration<Configurations>()->getListOfKocs(it->second.key());
				//std::cout << __COUT_HDR_FL__ << "\tKocs size: " << listOfKocs.size() << std::endl;

				for (std::set<std::string>::iterator sit=listOfKocs.begin(); sit!=listOfKocs.end(); ++sit)
				{
					unsigned int cv = cfgMgr->getConfiguration<Configurations>()->getConditionVersion(it->second.key(),*sit);

					//std::cout << __COUT_HDR_FL__ << "\tKoc: " << *sit << " Version: " << cv << std::endl;

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

			//std::cout << __COUT_HDR_FL__ << "Alias: " << it->first << " - Key: " << it->second.key() << std::endl;

			//add system subconfiguration alias
			xmldoc.addTextElementToData("SystemSubConfigurationAlias", it->first);
			parentEl = xmldoc.addTextElementToData("SystemSubConfigurationVersions", "");

			//get version key for the current system subconfiguration key
			for (std::set<int>::iterator vit=it->second.versions_.begin(); vit!=it->second.versions_.end(); ++vit)
			{
				//std::cout << __COUT_HDR_FL__ << "\t\t" << *vit << std::endl;

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

		std::cout << __COUT_HDR_FL__ << "getSpecificSystemConfiguration: " << alias << std::endl;

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

			//std::cout << __COUT_HDR_FL__ << "Alias: " << it->first << " - Key: " << it->second.key() << std::endl;

			//add system configuration alias and key
			xmldoc.addTextElementToData("SystemConfigurationAlias", it->first);
			sprintf(tmpIntStr,"%u",it->second.key());
			xmldoc.addTextElementToData("SystemConfigurationKey", tmpIntStr);
			parentEl = xmldoc.addTextElementToData("SystemConfigurationKOCs", "");

			//get KOCs alias and version for the current system configuration key
			assert(cfgMgr->getConfiguration<Configurations>());
			{
				listOfKocs = cfgMgr->getConfiguration<Configurations>()->getListOfKocs(it->second.key());
				//std::cout << __COUT_HDR_FL__ << "\tKocs size: " << listOfKocs.size() << std::endl;

				for (std::set<std::string>::iterator sit=listOfKocs.begin(); sit!=listOfKocs.end(); ++sit)
				{
					//current version
					unsigned int cv = cfgMgr->getConfiguration<Configurations>()->getConditionVersion(it->second.key(),*sit);

					//all existing versions
					versions = allCfgInfo.find(*sit)->second.versions_;

					//std::cout << __COUT_HDR_FL__ << "\tKoc: " << *sit << " Version: " << cv << std::endl;

					xmldoc.addTextElementToParent("KOC_alias", *sit, parentEl);
					sprintf(tmpIntStr,"%u",cv);
					parentElKoc = xmldoc.addTextElementToParent("KOC_currentVersion", tmpIntStr, parentEl);
					for (std::set<int>::iterator vit=versions.begin(); vit!=versions.end(); ++vit)
					{
						//std::cout << __COUT_HDR_FL__ << "\t\t" << *vit << std::endl;
						if(*vit == (int)cv) continue;
						sprintf(tmpIntStr,"%d",*vit);
						xmldoc.addTextElementToParent("KOC_existingVersion", tmpIntStr, parentElKoc);
					}
				}
			}


			//check for other existing backbone versions - they all (3 backbone pieces) better match!!
			//	(so we can just check one of them to get the list of historical versions: Configurations)
			//  (The 3 backbone pieces are Configurations -- ConfigurationAliases -- DefaultConfigurations

			versions = allCfgInfo.find("Configurations")->second.versions_;

			for (std::set<int>::iterator vit=versions.begin(); vit!=versions.end(); ++vit)
			{
				std::cout << __COUT_HDR_FL__ << "Configurations Version \t\t" << *vit << std::endl;
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

		std::cout << __COUT_HDR_FL__ << "getSpecificSubSystemConfiguration: " << subAlias << " version: " << version
				<< " chunkSize: " << chunkSize << " dataOffset: " << dataOffset << "     ";

		//verify alias and version exists
		std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
		std::map<std::string, ConfigurationInfo>::const_iterator it = allCfgInfo.find(subAlias);

		if(it == allCfgInfo.end())
		{
			std::cout << __COUT_HDR_FL__ << "SubSystemConfiguration not found" << std::endl;
		}
		else if(it->second.versions_.find(version) == it->second.versions_.end())
		{
			std::cout << __COUT_HDR_FL__ << "Version not found" << std::endl;
		}
		else
		{
			//load current version
			bool isInConfiguration = (allCfgInfo[subAlias].configurationPtr_->isStored(version));
			std::cout << __COUT_HDR_FL__ << "Version " << version << " is loaded: " <<
					(isInConfiguration?"YES":"NO") << "     ";

			if(!isInConfiguration) //load configuration view
				cfgMgr->getVersionedConfigurationByName(subAlias, version);
			else
				allCfgInfo[subAlias].configurationPtr_->setActiveView(version);

			isInConfiguration = (allCfgInfo[subAlias].configurationPtr_->isStored(version));
			std::cout << __COUT_HDR_FL__ << "Version " << version << " is loaded: " <<
					(isInConfiguration?"YES":"NO") << "     ";

			if(!isInConfiguration)
			{
				std::cout << __COUT_HDR_FL__ << "Version could not be loaded" << std::endl;
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

					std::cout << __COUT_HDR_FL__ << "\t\t******** view " <<
							allCfgInfo[subAlias].configurationPtr_->getViewVersion() << "     ";
					ConfigurationView* cfgViewPtr = allCfgInfo[subAlias].configurationPtr_->getViewP();

					parentEl = xmldoc.addTextElementToData("CurrentVersionColumnHeaders", "");
					std::vector<ViewColumnInfo> colInfo = cfgViewPtr->getColumnsInfo();
					for(int i=0;i<(int)colInfo.size();++i)	//column headers and types
					{
						std::cout << __COUT_HDR_FL__ << "\t\tCol " << i << ": " << colInfo[i].getName() << " "
								<< colInfo[i].getViewName() << " " << colInfo[i].getViewType() << "     ";

						xmldoc.addTextElementToParent("ColumnHeader", colInfo[i].getName(), parentEl);
						xmldoc.addTextElementToParent("ColumnType", colInfo[i].getViewType(), parentEl);
					}

					parentEl = xmldoc.addTextElementToData("CurrentVersionRows", "");

					for(int r=0;r<(int)cfgViewPtr->getNumberOfRows();++r)
					{
						//std::cout << __COUT_HDR_FL__ << "\t\tRow " << r << ": "  << std::endl;

						sprintf(tmpIntStr,"%d",r);
						DOMElement* tmpParentEl = xmldoc.addTextElementToParent("Row", tmpIntStr, parentEl);

						for(int c=0;c<(int)cfgViewPtr->getNumberOfColumns();++c)
							if(colInfo[c].getViewType() == "NUMBER")
							{
								int num;
								cfgViewPtr->getValue(num,r,c);
								//std::cout << __COUT_HDR_FL__ << "\t " << num << std::endl;

								sprintf(tmpIntStr,"%d",num);
								xmldoc.addTextElementToParent("Entry", tmpIntStr, tmpParentEl);
							}
							else
							{
								std::string val;
								cfgViewPtr->getValue(val,r,c);
								//std::cout << __COUT_HDR_FL__ << "\t " << val << std::endl;

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
		std::cout << __COUT_HDR_FL__ << "saveSpecificSubSystemConfiguration" << std::endl;
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


		std::cout << __COUT_HDR_FL__ << "getSpecificSubSystemConfiguration: " << subAlias << " version: " << version
				<< " chunkSize: " << chunkSize << " dataOffset: " << dataOffset << "     ";

		std::cout << __COUT_HDR_FL__ << "data: " << data << std::endl;

		//FIXME CHECK MUST HAVE LOCK!!
		// CHECK MUST HAVE LOCK!!
		// CHECK MUST HAVE LOCK!!
		// CHECK MUST HAVE LOCK!!
		// CHECK MUST HAVE LOCK!!
		if(it == allCfgInfo.end())
		{
			std::cout << __COUT_HDR_FL__ << "SubSystemConfiguration not found" << std::endl;

			xmldoc.addTextElementToData("Error", "SubSystemConfiguration not found");
		}
		else if(it->second.versions_.find(version) == it->second.versions_.end())
		{
			std::cout << __COUT_HDR_FL__ << "Version not found" << std::endl;

			xmldoc.addTextElementToData("Error", "Version not found");
		}
		else
		{
			//load current version
			bool isInConfiguration = (allCfgInfo[subAlias].configurationPtr_->isStored(version));
			std::cout << __COUT_HDR_FL__ << "Version " << version << " is loaded: " <<
					(isInConfiguration?"YES":"NO") << "     ";

			if(!isInConfiguration) //load configuration view
				cfgMgr->getVersionedConfigurationByName(subAlias, version);
			else
				allCfgInfo[subAlias].configurationPtr_->setActiveView(version);

			isInConfiguration = (allCfgInfo[subAlias].configurationPtr_->isStored(version));
			std::cout << __COUT_HDR_FL__ << "Version " << version << " is loaded: " <<
					(isInConfiguration?"YES":"NO") << "     ";

			if(!isInConfiguration)
			{
				std::cout << __COUT_HDR_FL__ << "Version could not be loaded" << std::endl;

				xmldoc.addTextElementToData("Error", "Version could not be loaded");
			}
			else
			{
				int temporaryVersion = allCfgInfo[subAlias].configurationPtr_->createTemporaryView(version);

				std::cout << __COUT_HDR_FL__ << "\t\ttemporaryVersion: " << temporaryVersion << std::endl;
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
						std::cout << __COUT_HDR_FL__ << "Row added" << std::endl;
					}

					while(j < k && j != (int)(std::string::npos))
					{
						//std::cout << __COUT_HDR_FL__ << r << "|" << c << "][" << i << "|" << k << "][" << std::endl;
						//std::cout << __COUT_HDR_FL__ << data.substr(i,j-i) << "|" << std::endl;
						if(colInfo[c].getViewType() == "NUMBER")
						{
							//std::cout << __COUT_HDR_FL__ << atoi(data.substr(i,j-i).c_str()) << "|" << std::endl;
							cfgViewPtr->setValue(atoi(data.substr(i,j-i).c_str()),r,c);
							//cfgViewPtr->getValue(cellNum,r,c);
							//std::cout << __COUT_HDR_FL__ << cellNum << " " << std::endl;
						}
						else
						{
							//std::cout << __COUT_HDR_FL__ << data.substr(i,j-i) << "|" << std::endl;
							cfgViewPtr->setValue(data.substr(i,j-i),r,c);
							//cfgViewPtr->getValue(cellStr,r,c);
							//std::cout << __COUT_HDR_FL__ << cellStr << " " << std::endl;
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
					std::cout << __COUT_HDR_FL__ << "Row deleted" << std::endl;
				}


				std::cout << __COUT_HDR_FL__ << "\t\t**************************** Save as new sub-config version" << std::endl;

				int newAssignedVersion = saveNewConfiguration(cfgMgr,subAlias,temporaryVersion);//cfgMgr->saveNewConfiguration(allCfgInfo[subAlias].configurationPtr_,temporaryVersion);

				xmldoc.addTextElementToData("savedAlias", subAlias);
				sprintf(tmpIntStr,"%d",newAssignedVersion);
				xmldoc.addTextElementToData("savedVersion", tmpIntStr);

				std::cout << __COUT_HDR_FL__ << "\t\t newAssignedVersion: " << newAssignedVersion << std::endl;

			}
		}
	}
	else if(Command == "changeKocVersionForSpecificConfig")
	{
		std::cout << __COUT_HDR_FL__ << "changeKocVersionForSpecificConfig" << std::endl;
		//TODO
	}
	else
		std::cout << __COUT_HDR_FL__ << "Command request not recognized." << std::endl;



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
ConfigurationManagerWithWriteAccess* ConfigurationGUISupervisor::refreshUserSession(std::string username, uint64_t activeSessionIndex, int &backboneVersion)
{
	std::stringstream ssMapKey;
	ssMapKey << username << ":" << activeSessionIndex;
	std::string mapKey = ssMapKey.str();
	std::cout << __COUT_HDR_FL__ << mapKey << " ... current size: " << userConfigurationManagers_.size() << std::endl;
	//create new config mgr if not one for active session index
	if(userConfigurationManagers_.find(mapKey) == userConfigurationManagers_.end())
	{
		userConfigurationManagers_[mapKey] = new ConfigurationManagerWithWriteAccess();

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
			std::cout << __COUT_HDR_FL__ << now << ":" << it->second << " = " << now - it->second << std::endl;
			delete userConfigurationManagers_[it->first]; //call destructor
			assert(userConfigurationManagers_.erase(it->first));	//erase by key
			userLastUseTime_.erase(it);								//erase by iterator

			it=userLastUseTime_.begin(); //fail safe.. reset it, to avoid trying to understand what happens with the next iterator
		}

	return userConfigurationManagers_[mapKey];
}

int ConfigurationGUISupervisor::saveNewConfiguration(ConfigurationManagerWithWriteAccess *cfgMgr,
		std::string configurationName, int temporaryVersion)
{
	return cfgMgr->getConfigurationInterface()->saveNewVersion(
			cfgMgr->getConfigurationByName(configurationName), temporaryVersion);
}


