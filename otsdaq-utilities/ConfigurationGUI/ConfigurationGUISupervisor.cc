#include "otsdaq-utilities/ConfigurationGUI/ConfigurationGUISupervisor.h"
#include "otsdaq-core/ConfigurationPluginDataFormats/IterateConfiguration.h"
#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/Macros/CoutMacros.h"
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"

#if MESSAGEFACILITY_HEX_VERSION > 0x20100
#include <boost/stacktrace.hpp>
#endif

#include "otsdaq-core/ConfigurationPluginDataFormats/XDAQContextConfiguration.h" //for context relaunch

#include <xdaq/NamespaceURI.h>

#include <iostream>
#include <map>
#include <utility>



using namespace ots;

#undef 	__MF_SUBJECT__
#define __MF_SUBJECT__ "CfgGUI"

#define CONFIG_INFO_PATH	std::string(getenv("CONFIGURATION_INFO_PATH")) + "/"
#define CONFIG_INFO_EXT		std::string("Info.xml")

/*! the XDAQ_INSTANTIATOR_IMPL(ns1::ns2::...) macro needs to be put into the implementation file (.cc) of the XDAQ application */
xdaq::Application * ConfigurationGUISupervisor::instantiate(xdaq::ApplicationStub * stub )
{
	return new ConfigurationGUISupervisor(stub);
}


//========================================================================================================================
//new user gets a config mgr assigned
//user can fill any of the tables (fill from version or init empty), which becomes the active view for that table
ConfigurationGUISupervisor::ConfigurationGUISupervisor(xdaq::ApplicationStub* stub)
: 	CoreSupervisorBase	(stub)
{
	__SUP_COUT__ << "Constructor started." << __E__;

	INIT_MF("ConfigurationGUI");

	init();
	__SUP_COUT__ << "Constructor complete." << __E__;
}

//========================================================================================================================
ConfigurationGUISupervisor::~ConfigurationGUISupervisor(void)
{
	destroy();
}

//========================================================================================================================
void ConfigurationGUISupervisor::init(void)
{
	__SUP_COUT__ << "Initializing..." << std::endl;

	__SUP_COUT__ << "Activating saved context, which may prepare for normal mode..." << std::endl;
	try
	{
		testXDAQContext(); //test context group activation
	}
	catch(...)
	{
		__COUT_WARN__ << "Failed test context group activation. otsdaq, in Normal mode, will not launch when this test fails. " <<
				"Check the active context group from within Wizard Mode." << __E__;
	}
}


//========================================================================================================================
void ConfigurationGUISupervisor::destroy(void)
{
	//called by destructor
	for (std::map<std::string, ConfigurationManagerRW* > ::iterator it=userConfigurationManagers_.begin(); it!=userConfigurationManagers_.end(); ++it)
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
void ConfigurationGUISupervisor::defaultPage(xgi::Input*  in, xgi::Output*  out )
{
	cgicc::Cgicc cgiIn(in);
	std::string configName = CgiDataUtilities::getData(cgiIn,"configWindowName"); //from GET
	if(configName == "tableEditor")
		*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/ConfigurationTableEditor.html?urn=" <<
		this->getApplicationDescriptor()->getLocalId() <<"'></frameset></html>";
	if(configName == "iterate")
		*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/Iterate.html?urn=" <<
		this->getApplicationDescriptor()->getLocalId() <<"'></frameset></html>";
	else
		*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/ConfigurationGUI.html?urn=" <<
		this->getApplicationDescriptor()->getLocalId() <<"'></frameset></html>";
}


//========================================================================================================================
//When overriding, setup default property values here
// called by CoreSupervisorBase constructor
void ConfigurationGUISupervisor::setSupervisorPropertyDefaults(void)
{
	CorePropertySupervisorBase::setSupervisorProperty(CorePropertySupervisorBase::SUPERVISOR_PROPERTIES.UserPermissionsThreshold,
			"*=10 | deleteTreeNodeRecords=255 | saveConfigurationInfo=255 | deleteConfigurationInfo=255"); //experienced users to edit, admins to delete
	CorePropertySupervisorBase::setSupervisorProperty(CorePropertySupervisorBase::SUPERVISOR_PROPERTIES.RequireUserLockRequestTypes,
			"*");//all
}

//========================================================================================================================
//forceSupervisorPropertyValues
//		override to force supervisor property values (and ignore user settings)
void ConfigurationGUISupervisor::forceSupervisorPropertyValues()
{
	CorePropertySupervisorBase::setSupervisorProperty(CorePropertySupervisorBase::SUPERVISOR_PROPERTIES.AutomatedRequestTypes,
			""); //none
	CorePropertySupervisorBase::setSupervisorProperty(CorePropertySupervisorBase::SUPERVISOR_PROPERTIES.CheckUserLockRequestTypes,
			"*");//all
}

//========================================================================================================================
void ConfigurationGUISupervisor::request(const std::string& requestType, cgicc::Cgicc& cgiIn,
		HttpXmlDocument& xmlOut, const WebUsers::RequestUserInfo& userInfo)
try
{
	//Commands
	//	saveConfigurationInfo
	//	deleteConfigurationInfo
	//	launchOTS
	// 	launchWiz
	// 	flattenToSystemAliases
	// 	versionTracking
	//	getColumnTypes
	//	getGroupAliases
	//	setGroupAliasInActiveBackbone
	//	setVersionAliasInActiveBackbone
	//	setAliasOfGroupMembers
	//	getVersionAliases
	//	getConfigurationGroups
	//	getConfigurationGroupType
	//	getConfigurations
	//	getContextMemberNames
	//	getBackboneMemberNames
	//	getIterateMemberNames
	//	getSpecificConfigurationGroup
	//	saveNewConfigurationGroup
	//	getSpecificConfiguration
	//	saveSpecificConfiguration
	//	clearConfigurationTemporaryVersions
	//	clearConfigurationCachedVersions
	//
	//		---- associated with JavaScript Config API
	//	getTreeView
	//	getTreeNodeCommonFields
	//	getUniqueFieldValuesForRecords
	//	getTreeNodeFieldValues
	//	setTreeNodeFieldValues
	//	addTreeNodeRecords
	//	deleteTreeNodeRecords
	//		---- end associated with JavaScript Config API
	//
	//	activateConfigGroup
	//	getActiveConfigGroups
	//  copyViewToCurrentColumns
	//	saveTreeNodeEdit
	//	getAffectedActiveGroups
	// 	getLinkToChoices
	//	getLastConfigGroups
	//	mergeGroups
	//
	//		---- associated with JavaScript Iterate App
	//	savePlanCommandSequence
	//		---- end associated with JavaScript Iterate App



	//acquire user's configuration manager based on username&  activeSessionIndex
	std::string 	refresh = CgiDataUtilities::getData(cgiIn,"refresh"); 	//from GET
	//__SUP_COUT__ << "refresh: " << refresh << std::endl;

	//refresh to reload from info files and db (maintains temporary views!)
	ConfigurationManagerRW* cfgMgr = refreshUserSession(userInfo.username_,
			userInfo.activeUserSessionIndex_,
			(refresh == "1") );

	if(requestType == "saveConfigurationInfo")
	{
		std::string configName       = CgiDataUtilities::getData (cgiIn,"configName"); //from GET
		std::string columnCSV        = CgiDataUtilities::postData(cgiIn,"columnCSV"); //from POST
		std::string allowOverwrite   = CgiDataUtilities::getData (cgiIn,"allowOverwrite"); //from GET
		std::string tableDescription = CgiDataUtilities::postData(cgiIn,"tableDescription"); //from POST
		std::string columnChoicesCSV = CgiDataUtilities::postData(cgiIn,"columnChoicesCSV"); //from POST

		//columnCSV = CgiDataUtilities::decodeURIComponent(columnCSV);
		//tableDescription = CgiDataUtilities::decodeURIComponent(tableDescription);

		__SUP_COUT__ << "configName: " << configName << std::endl;
		__SUP_COUT__ << "columnCSV: " << columnCSV << std::endl;
		__SUP_COUT__ << "tableDescription: " << tableDescription << std::endl;
		__SUP_COUT__ << "columnChoicesCSV: " << columnChoicesCSV << std::endl;
		__SUP_COUT__ << "allowOverwrite: " << allowOverwrite << std::endl;

		if(!allSupervisorInfo_.isWizardMode())
		{
			__SUP_SS__ << "Improper permissions for saving configuration info." << std::endl;
			xmlOut.addTextElementToData("Error", ss.str());
		}
		else
			handleSaveConfigurationInfoXML(xmlOut,cfgMgr,configName,columnCSV,tableDescription,
					columnChoicesCSV,allowOverwrite=="1");
	}
	else if(requestType == "deleteConfigurationInfo")
	{
		std::string configName = CgiDataUtilities::getData(cgiIn,"configName"); //from GET
		__SUP_COUT__ << "configName: " << configName << std::endl;
		handleDeleteConfigurationInfoXML(xmlOut,cfgMgr,configName);
	}
	else if(requestType == "gatewayLaunchOTS" || requestType == "gatewayLaunchWiz" ||//requestType == "launchOTS" || requestType == "launchWiz" ||
			requestType == "flattenToSystemAliases")
	{
		//NOTE: similar to Supervisor version but does not keep active sessions
		__SUP_COUT_WARN__ << requestType << " command received! " << std::endl;
		__MOUT_WARN__ << requestType << " command received! " << std::endl;

		//now launch
		__SUP_COUT_INFO__ << "Launching... " << std::endl;

		__SUP_COUT__ << "Extracting target context hostnames... " << std::endl;
		std::vector<std::string> hostnames;
		try
		{
			cfgMgr->init(); //completely reset to re-align with any changes

			const XDAQContextConfiguration* contextConfiguration = cfgMgr->__GET_CONFIG__(XDAQContextConfiguration);

			auto contexts = contextConfiguration->getContexts();
			unsigned int i,j;
			for(const auto& context: contexts)
			{
				if(!context.status_) continue;

				//find last slash
				j=0; //default to whole string
				for(i=0;i<context.address_.size();++i)
					if(context.address_[i] == '/')
						j = i+1;
				hostnames.push_back(context.address_.substr(j));
				__SUP_COUT__ << "hostname = " << hostnames.back() << std::endl;
			}
		}
		catch(...)
		{
			__SUP_SS__ << "\nTransition to Configuring interrupted! " <<
					"The Configuration Manager could not be initialized." << std::endl;

			__SUP_COUT_ERR__ << "\n" << ss.str();
			return;
		}

		for(const auto& hostname: hostnames)
		{
			std::string fn = (std::string(getenv("SERVICE_DATA_PATH")) +
					"/StartOTS_action_" + hostname + ".cmd");
			FILE* fp = fopen(fn.c_str(),"w");
			if(fp)
			{
				if(requestType == "gatewayLaunchOTS")
					fprintf(fp,"LAUNCH_OTS");
				else if(requestType == "gatewayLaunchWiz")
					fprintf(fp,"LAUNCH_WIZ");
				else if(requestType == "flattenToSystemAliases")
				{
					fprintf(fp,"FLATTEN_TO_SYSTEM_ALIASES");
					fclose(fp);
					break; // only do at one host
				}

				fclose(fp);
			}
			else
				__SUP_COUT_ERR__ << "Unable to open command file: " << fn << std::endl;
		}

	}
//	else if(requestType == "launchOTS")
//	{
//		__SUP_COUT_WARN__ << "launchOTS command received! Launching... " << std::endl;
//
//		FILE* fp = fopen((std::string(getenv("SERVICE_DATA_PATH")) +
//				"/StartOTS_action.cmd").c_str(),"w");
//		if(fp)
//		{
//			fprintf(fp,"LAUNCH_OTS");
//			fclose(fp);
//		}
//		else
//			__SUP_COUT_ERR__ << "Unable to open command file: " << (std::string(getenv("SERVICE_DATA_PATH")) +
//					"/StartOTS_action.cmd") << std::endl;
//	}
//	else if(requestType == "launchWiz")
//	{
//		__SUP_COUT_WARN__ << "launchWiz command received! Launching... " << std::endl;
//
//		FILE* fp = fopen((std::string(getenv("SERVICE_DATA_PATH")) +
//				"/StartOTS_action.cmd").c_str(),"w");
//		if(fp)
//		{
//			fprintf(fp,"LAUNCH_WIZ");
//			fclose(fp);
//		}
//		else
//			__SUP_COUT_ERR__ << "Unable to open command file: " << (std::string(getenv("SERVICE_DATA_PATH")) +
//					"/StartOTS_action.cmd") << std::endl;
//	}
//	else if(requestType == "flattenToSystemAliases")
//	{
//		__SUP_COUT_WARN__ << "flattenToSystemAliases command received! Launching... " << std::endl;
//
//		FILE* fp = fopen((std::string(getenv("SERVICE_DATA_PATH")) +
//				"/StartOTS_action.cmd").c_str(),"w");
//		if(fp)
//		{
//			fprintf(fp,"FLATTEN_TO_SYSTEM_ALIASES");
//			fclose(fp);
//		}
//		else
//			__SUP_COUT_ERR__ << "Unable to open command file: " << (std::string(getenv("SERVICE_DATA_PATH")) +
//					"/StartOTS_action.cmd") << std::endl;
//	}
	else if(requestType == "versionTracking")
	{
		std::string type = CgiDataUtilities::getData(cgiIn,"Type"); //from GET
		__SUP_COUT__ << "type: " << type << std::endl;

		if(type == "Get")
			xmlOut.addTextElementToData("versionTrackingStatus",
					ConfigurationInterface::isVersionTrackingEnabled()?"ON":"OFF");
		else if(type == "ON")
		{
			ConfigurationInterface::setVersionTrackingEnabled(true);
			xmlOut.addTextElementToData("versionTrackingStatus",
					ConfigurationInterface::isVersionTrackingEnabled()?"ON":"OFF");
		}
		else if(type == "OFF")
		{
			ConfigurationInterface::setVersionTrackingEnabled(false);
			xmlOut.addTextElementToData("versionTrackingStatus",
					ConfigurationInterface::isVersionTrackingEnabled()?"ON":"OFF");
		}
	}
	else if(requestType == "getColumnTypes")
	{
		//return the possible column types and their defaults
		std::vector<std::string> allTypes = ViewColumnInfo::getAllTypesForGUI();
		std::vector<std::string> allDataTypes = ViewColumnInfo::getAllDataTypesForGUI();
		std::map<std::pair<std::string,std::string>,std::string> allDefaults =
				ViewColumnInfo::getAllDefaultsForGUI();

		for(const auto& type:allTypes)
			xmlOut.addTextElementToData("columnTypeForGUI",
					type);
		for(const auto& dataType:allDataTypes)
			xmlOut.addTextElementToData("columnDataTypeForGUI",
					dataType);

		for(const auto& colDefault:allDefaults)
		{
			xmlOut.addTextElementToData("columnDefaultDataType",
					colDefault.first.first);
			xmlOut.addTextElementToData("columnDefaultTypeFilter",
					colDefault.first.second);
			xmlOut.addTextElementToData("columnDefaultValue",
					colDefault.second);
		}
	}
	else if(requestType == "getGroupAliases")
	{
		//Since this is called from setting up System View in the config GUI
		//	give option for reloading "persistent" active configurations
		bool reloadActive = 1 == CgiDataUtilities::getDataAsInt(cgiIn,"reloadActiveGroups"); //from GET

		__SUP_COUT__ << "reloadActive: " << reloadActive << std::endl;
		bool wasError = false;
		if(reloadActive)
		{
			try
			{
				cfgMgr->clearAllCachedVersions();
				cfgMgr->restoreActiveConfigurationGroups(true);
			}
			catch(std::runtime_error& e)
			{
				__SUP_SS__ << ("Error loading active groups!\n\n" + std::string(e.what())) << std::endl;
				__SUP_COUT_ERR__ << "\n" << ss.str();
				xmlOut.addTextElementToData("Error", ss.str());
				wasError = true;
			}
			catch(...)
			{
				__SUP_SS__ << ("Error loading active groups!\n\n") << std::endl;
				__SUP_COUT_ERR__ << "\n" << ss.str();
				xmlOut.addTextElementToData("Error", ss.str());
				wasError = true;
			}

		}


		handleGroupAliasesXML(xmlOut,cfgMgr);
	}
	else if(requestType == "setGroupAliasInActiveBackbone")
	{
		std::string groupAlias = CgiDataUtilities::getData(cgiIn,"groupAlias"); //from GET
		std::string groupName = CgiDataUtilities::getData(cgiIn,"groupName"); //from GET
		std::string groupKey = CgiDataUtilities::getData(cgiIn,"groupKey"); //from GET

		__SUP_COUT__ << "groupAlias: " << groupAlias << std::endl;
		__SUP_COUT__ << "groupName: " << groupName << std::endl;
		__SUP_COUT__ << "groupKey: " << groupKey << std::endl;

		handleSetGroupAliasInBackboneXML(xmlOut,cfgMgr,groupAlias,groupName,
				ConfigurationGroupKey(groupKey),userInfo.username_);
	}
	else if(requestType == "setVersionAliasInActiveBackbone")
	{
		std::string versionAlias = CgiDataUtilities::getData(cgiIn,"versionAlias"); //from GET
		std::string configName = CgiDataUtilities::getData(cgiIn,"configName"); //from GET
		std::string version = CgiDataUtilities::getData(cgiIn,"version"); //from GET

		__SUP_COUT__ << "versionAlias: " << versionAlias << std::endl;
		__SUP_COUT__ << "configName: " << configName << std::endl;
		__SUP_COUT__ << "version: " << version << std::endl;

		handleSetVersionAliasInBackboneXML(xmlOut,cfgMgr,versionAlias,
				configName,
				ConfigurationVersion(version),userInfo.username_);
	}
	else if(requestType == "setAliasOfGroupMembers")
	{
		std::string versionAlias = CgiDataUtilities::getData(cgiIn,"versionAlias"); //from GET
		std::string groupName = CgiDataUtilities::getData(cgiIn,"groupName"); //from GET
		std::string groupKey = CgiDataUtilities::getData(cgiIn,"groupKey"); //from GET

		__SUP_COUT__ << "versionAlias: " << versionAlias << std::endl;
		__SUP_COUT__ << "groupName: " << groupName << std::endl;
		__SUP_COUT__ << "groupKey: " << groupKey << std::endl;

		handleAliasGroupMembersInBackboneXML(xmlOut,cfgMgr,versionAlias,
				groupName,
				ConfigurationGroupKey(groupKey),userInfo.username_);
	}
	else if(requestType == "getVersionAliases")
	{
		handleVersionAliasesXML(xmlOut,cfgMgr);
	}
	else if(requestType == "getConfigurationGroups")
	{
		bool doNotReturnMembers = CgiDataUtilities::getDataAsInt(cgiIn,"doNotReturnMembers") == 1?true:false; //from GET

		__SUP_COUT__ << "doNotReturnMembers: " << doNotReturnMembers << std::endl;
		handleConfigurationGroupsXML(xmlOut,cfgMgr,!doNotReturnMembers);
	}
	else if(requestType == "getConfigurationGroupType")
	{
		std::string configList 		= CgiDataUtilities::postData(cgiIn,"configList"); 	//from POST
		__SUP_COUT__ << "configList: " << configList << std::endl;

		handleGetConfigurationGroupTypeXML(xmlOut,cfgMgr,configList);
	}
	else if(requestType == "getConfigurations")
	{
		std::string allowIllegalColumns = CgiDataUtilities::getData(cgiIn,"allowIllegalColumns"); //from GET

		__SUP_COUT__ << "allowIllegalColumns: " << allowIllegalColumns << std::endl;

		handleConfigurationsXML(xmlOut,cfgMgr, allowIllegalColumns == "1");
	}
	else if(requestType == "getContextMemberNames")
	{
		std::set<std::string> members = cfgMgr->getContextMemberNames();

		for(auto& member:members)
			xmlOut.addTextElementToData("ContextMember", member);
	}
	else if(requestType == "getBackboneMemberNames")
	{
		std::set<std::string> members = cfgMgr->getBackboneMemberNames();

		for(auto& member:members)
			xmlOut.addTextElementToData("BackboneMember", member);
	}
	else if(requestType == "getIterateMemberNames")
	{
		std::set<std::string> members = cfgMgr->getIterateMemberNames();

		for(auto& member:members)
			xmlOut.addTextElementToData("IterateMember", member);
	}
	else if(requestType == "getSpecificConfigurationGroup")
	{
		std::string 	groupName = CgiDataUtilities::getData	(cgiIn,"groupName"); 	//from GET
		std::string 	groupKey = CgiDataUtilities::getData	(cgiIn,"groupKey"); 		//from GET

		__SUP_COUT__ << "groupName: " << groupName << std::endl;
		__SUP_COUT__ << "groupKey: " << groupKey << std::endl;

		handleGetConfigurationGroupXML(xmlOut,cfgMgr,groupName,ConfigurationGroupKey(groupKey));
	}
	else if(requestType == "saveNewConfigurationGroup")
	{
		std::string groupName 			= CgiDataUtilities::getData 	(cgiIn,"groupName"); 		//from GET
		bool		ignoreWarnings 		= CgiDataUtilities::getDataAsInt(cgiIn,"ignoreWarnings");	//from GET
		bool		allowDuplicates 	= CgiDataUtilities::getDataAsInt(cgiIn,"allowDuplicates");	//from GET
		bool		lookForEquivalent	= CgiDataUtilities::getDataAsInt(cgiIn,"lookForEquivalent");	//from GET
		std::string configList 			= CgiDataUtilities::postData	(cgiIn,"configList"); 	//from POST
		std::string	comment 			= CgiDataUtilities::getData		(cgiIn,"groupComment");	//from GET

		__SUP_COUT__ << "saveNewConfigurationGroup: " << groupName << std::endl;
		__SUP_COUT__ << "configList: " << configList << std::endl;
		__SUP_COUT__ << "ignoreWarnings: " << ignoreWarnings << std::endl;
		__SUP_COUT__ << "allowDuplicates: " << allowDuplicates << std::endl;
		__SUP_COUT__ << "lookForEquivalent: " << lookForEquivalent << std::endl;
		__SUP_COUT__ << "comment: " << comment << std::endl;

		handleCreateConfigurationGroupXML(xmlOut,cfgMgr,groupName,configList,
				allowDuplicates,ignoreWarnings,comment,lookForEquivalent);
	}
	else if(requestType == "getSpecificConfiguration")
	{
		std::string		configName = CgiDataUtilities::getData		(cgiIn,"configName"); //from GET
		std::string  	versionStr = CgiDataUtilities::getData		(cgiIn,"version");	//from GET
		int				dataOffset = CgiDataUtilities::getDataAsInt	(cgiIn,"dataOffset");	//from GET
		int				chunkSize  = CgiDataUtilities::getDataAsInt	(cgiIn,"chunkSize");	//from GET

		std::string 	allowIllegalColumns = CgiDataUtilities::getData(cgiIn,"allowIllegalColumns"); //from GET
		__SUP_COUT__ << "allowIllegalColumns: " << (allowIllegalColumns=="1") << std::endl;

		__SUP_COUT__ << "getSpecificConfiguration: " << configName << " versionStr: " << versionStr
				<< " chunkSize: " << chunkSize << " dataOffset: " << dataOffset << std::endl;

		ConfigurationVersion version;
		const std::map<std::string, ConfigurationInfo>& allCfgInfo = cfgMgr->getAllConfigurationInfo();
		std::string versionAlias;

		if(allCfgInfo.find(configName) != allCfgInfo.end())
		{
			if(versionStr == "" && //take latest version if no version specified
					allCfgInfo.at(configName).versions_.size())
				version =* (allCfgInfo.at(configName).versions_.rbegin());
			else if(versionStr.find(ConfigurationManager::ALIAS_VERSION_PREAMBLE) == 0)
			{
				//convert alias to version
				std::map<std::string /*table*/, std::map<
					std::string /*alias*/,ConfigurationVersion> > versionAliases =
						cfgMgr->getVersionAliases();

				versionAlias = versionStr.substr(ConfigurationManager::ALIAS_VERSION_PREAMBLE.size());
	//			if(versionAlias == ConfigurationManager::SCRATCH_VERSION_ALIAS) //NOT NEEDED IF SCRATCH IS ALWAYS ALIAS
	//			{
	//				version = ConfigurationVersion::SCRATCH;
	//				__SUP_COUT__ << "version alias translated to: " << version << std::endl;
	//			}
	//			else
				if(versionAliases.find(configName) != versionAliases.end() &&
						versionAliases[configName].find(versionStr.substr(
								ConfigurationManager::ALIAS_VERSION_PREAMBLE.size())) !=
										versionAliases[configName].end())
				{
					version = versionAliases[configName][versionStr.substr(
							ConfigurationManager::ALIAS_VERSION_PREAMBLE.size())];
					__SUP_COUT__ << "version alias translated to: " << version << std::endl;
				}
				else
					__SUP_COUT_WARN__ << "version alias '" << versionStr.substr(
							ConfigurationManager::ALIAS_VERSION_PREAMBLE.size()) <<
							"'was not found in active version aliases!" << std::endl;
			}
			else					//else take specified version
				version = atoi(versionStr.c_str());
		}

		__SUP_COUT__ << "version: " << version << std::endl;

		handleGetConfigurationXML(xmlOut,cfgMgr,configName,ConfigurationVersion(version),
				(allowIllegalColumns=="1"));
		//append author column default value
		xmlOut.addTextElementToData("DefaultRowValue", userInfo.username_);
	}
	else if(requestType == "saveSpecificConfiguration")
	{
		std::string 	configName 	= CgiDataUtilities::getData	    (cgiIn,"configName"); 	//from GET
		int				version 	= CgiDataUtilities::getDataAsInt(cgiIn,"version");		//from GET
		int				dataOffset 	= CgiDataUtilities::getDataAsInt(cgiIn,"dataOffset");		//from GET
		bool			sourceTableAsIs	= CgiDataUtilities::getDataAsInt(cgiIn,"sourceTableAsIs");	//from GET
		bool			lookForEquivalent = CgiDataUtilities::getDataAsInt(cgiIn,"lookForEquivalent");	//from GET
		int				temporary 	= CgiDataUtilities::getDataAsInt(cgiIn,"temporary");		//from GET
		std::string		comment 	= CgiDataUtilities::getData		(cgiIn,"tableComment");	//from GET

		std::string	data = CgiDataUtilities::postData(cgiIn,"data"); //from POST
		//data format: commas and semi-colons indicate new row
		//r0c0,r0c1,...,r0cN,;r1c0,...

		__SUP_COUT__ << "configName: " << configName << " version: " << version
				<< " temporary: " << temporary << " dataOffset: " << dataOffset << std::endl;
		__SUP_COUT__ << "comment: " << comment << std::endl;
		__SUP_COUT__ << "data: " << data << std::endl;
		__SUP_COUT__ << "sourceTableAsIs: " << sourceTableAsIs << std::endl;
		__SUP_COUT__ << "lookForEquivalent: " << lookForEquivalent << std::endl;

		handleCreateConfigurationXML(xmlOut,cfgMgr,configName,ConfigurationVersion(version),
				temporary,data,dataOffset,userInfo.username_,comment,sourceTableAsIs,lookForEquivalent);
	}
	else if(requestType == "clearConfigurationTemporaryVersions")
	{
		std::string 	configName 	= CgiDataUtilities::getData	    (cgiIn,"configName"); 	//from GET
		__SUP_COUT__ << "configName: " << configName << std::endl;

		try { cfgMgr->eraseTemporaryVersion(configName);}
		catch(std::runtime_error& e)
		{
			__SUP_COUT__ << "Error detected!\n\n " << e.what() << std::endl;
			xmlOut.addTextElementToData("Error", "Error clearing temporary views!\n " +
					std::string(e.what()));
		}
		catch(...)
		{
			__SUP_COUT__ << "Error detected!\n\n "<< std::endl;
			xmlOut.addTextElementToData("Error", "Error clearing temporary views! ");
		}
	}
	else if(requestType == "clearConfigurationCachedVersions")
	{
		std::string 	configName 	= CgiDataUtilities::getData	    (cgiIn,"configName"); 	//from GET
		__SUP_COUT__ << "configName: " << configName << std::endl;

		try { cfgMgr->clearCachedVersions(configName);}
		catch(std::runtime_error& e)
		{
			__SUP_COUT__ << "Error detected!\n\n " << e.what() << std::endl;
			xmlOut.addTextElementToData("Error", "Error clearing cached views!\n " +
					std::string(e.what()));
		}
		catch(...)
		{
			__SUP_COUT__ << "Error detected!\n\n "<< std::endl;
			xmlOut.addTextElementToData("Error", "Error clearing cached views! ");
		}
	}
	else if(requestType == "getTreeView")
	{
		std::string 	configGroup 	= CgiDataUtilities::getData(cgiIn,"configGroup");
		std::string 	configGroupKey 	= CgiDataUtilities::getData(cgiIn,"configGroupKey");
		std::string 	startPath 		= CgiDataUtilities::postData(cgiIn,"startPath");
		std::string 	modifiedTables 	= CgiDataUtilities::postData(cgiIn,"modifiedTables");
		std::string 	filterList	 	= CgiDataUtilities::postData(cgiIn,"filterList");
		int				depth	 		= CgiDataUtilities::getDataAsInt(cgiIn,"depth");
		bool			hideStatusFalse	= CgiDataUtilities::getDataAsInt(cgiIn,"hideStatusFalse");

		__SUP_COUT__ << "configGroup: " << configGroup << std::endl;
		__SUP_COUT__ << "configGroupKey: " << configGroupKey << std::endl;
		__SUP_COUT__ << "startPath: " << startPath << std::endl;
		__SUP_COUT__ << "depth: " << depth << std::endl;
		__SUP_COUT__ << "hideStatusFalse: " << hideStatusFalse << std::endl;
		__SUP_COUT__ << "modifiedTables: " << modifiedTables << std::endl;
		__SUP_COUT__ << "filterList: " << filterList << std::endl;

		handleFillTreeViewXML(xmlOut,cfgMgr,configGroup,ConfigurationGroupKey(configGroupKey),
				startPath,depth,hideStatusFalse,modifiedTables,filterList);
	}
	else if(requestType == "getTreeNodeCommonFields")
	{
		std::string 	configGroup 	= CgiDataUtilities::getData(cgiIn,"configGroup");
		std::string 	configGroupKey 	= CgiDataUtilities::getData(cgiIn,"configGroupKey");
		std::string 	startPath 		= CgiDataUtilities::postData(cgiIn,"startPath");
		std::string 	modifiedTables 	= CgiDataUtilities::postData(cgiIn,"modifiedTables");
		std::string 	fieldList	 	= CgiDataUtilities::postData(cgiIn,"fieldList");
		std::string 	recordList	 	= CgiDataUtilities::postData(cgiIn,"recordList");
		int				depth	 		= CgiDataUtilities::getDataAsInt(cgiIn,"depth");

		__SUP_COUT__ << "configGroup: " << configGroup << std::endl;
		__SUP_COUT__ << "configGroupKey: " << configGroupKey << std::endl;
		__SUP_COUT__ << "startPath: " << startPath << std::endl;
		__SUP_COUT__ << "depth: " << depth << std::endl;
		__SUP_COUT__ << "fieldList: " << fieldList << std::endl;
		__SUP_COUT__ << "recordList: " << recordList << std::endl;
		__SUP_COUT__ << "modifiedTables: " << modifiedTables << std::endl;

		handleFillTreeNodeCommonFieldsXML(xmlOut,cfgMgr,configGroup,ConfigurationGroupKey(configGroupKey),
				startPath,depth,modifiedTables,recordList,fieldList);

	}
	else if(requestType == "getUniqueFieldValuesForRecords")
	{
		std::string 	configGroup 	= CgiDataUtilities::getData(cgiIn,"configGroup");
		std::string 	configGroupKey 	= CgiDataUtilities::getData(cgiIn,"configGroupKey");
		std::string 	startPath 		= CgiDataUtilities::postData(cgiIn,"startPath");
		std::string 	modifiedTables 	= CgiDataUtilities::postData(cgiIn,"modifiedTables");
		std::string 	fieldList	 	= CgiDataUtilities::postData(cgiIn,"fieldList");
		std::string 	recordList	 	= CgiDataUtilities::postData(cgiIn,"recordList");

		__SUP_COUT__ << "configGroup: " << configGroup << std::endl;
		__SUP_COUT__ << "configGroupKey: " << configGroupKey << std::endl;
		__SUP_COUT__ << "startPath: " << startPath << std::endl;
		__SUP_COUT__ << "fieldList: " << fieldList << std::endl;
		__SUP_COUT__ << "recordList: " << recordList << std::endl;
		__SUP_COUT__ << "modifiedTables: " << modifiedTables << std::endl;

		handleFillUniqueFieldValuesForRecordsXML(xmlOut,cfgMgr,configGroup,ConfigurationGroupKey(configGroupKey),
				startPath,modifiedTables,recordList,fieldList);

	}
	else if(requestType == "getTreeNodeFieldValues")
	{
		std::string 	configGroup 	= CgiDataUtilities::getData(cgiIn,"configGroup");
		std::string 	configGroupKey 	= CgiDataUtilities::getData(cgiIn,"configGroupKey");
		std::string 	startPath 		= CgiDataUtilities::postData(cgiIn,"startPath");
		std::string 	modifiedTables 	= CgiDataUtilities::postData(cgiIn,"modifiedTables");
		std::string 	fieldList	 	= CgiDataUtilities::postData(cgiIn,"fieldList");
		std::string 	recordList	 	= CgiDataUtilities::postData(cgiIn,"recordList");

		__SUP_COUT__ << "configGroup: " << configGroup << std::endl;
		__SUP_COUT__ << "configGroupKey: " << configGroupKey << std::endl;
		__SUP_COUT__ << "startPath: " << startPath << std::endl;
		__SUP_COUT__ << "fieldList: " << fieldList << std::endl;
		__SUP_COUT__ << "recordList: " << recordList << std::endl;
		__SUP_COUT__ << "modifiedTables: " << modifiedTables << std::endl;

		handleFillGetTreeNodeFieldValuesXML(xmlOut,cfgMgr,configGroup,ConfigurationGroupKey(configGroupKey),
				startPath,modifiedTables,recordList,fieldList);
	}
	else if(requestType == "setTreeNodeFieldValues")
	{
		std::string 	configGroup 	= CgiDataUtilities::getData(cgiIn,"configGroup");
		std::string 	configGroupKey 	= CgiDataUtilities::getData(cgiIn,"configGroupKey");
		std::string 	startPath 		= CgiDataUtilities::postData(cgiIn,"startPath");
		std::string 	modifiedTables 	= CgiDataUtilities::postData(cgiIn,"modifiedTables");
		std::string 	fieldList	 	= CgiDataUtilities::postData(cgiIn,"fieldList");
		std::string 	recordList	 	= CgiDataUtilities::postData(cgiIn,"recordList");
		std::string 	valueList	 	= CgiDataUtilities::postData(cgiIn,"valueList");

		__SUP_COUT__ << "configGroup: " << configGroup << std::endl;
		__SUP_COUT__ << "configGroupKey: " << configGroupKey << std::endl;
		__SUP_COUT__ << "startPath: " << startPath << std::endl;
		__SUP_COUT__ << "fieldList: " << fieldList << std::endl;
		__SUP_COUT__ << "valueList: " << valueList << std::endl;
		__SUP_COUT__ << "recordList: " << recordList << std::endl;
		__SUP_COUT__ << "modifiedTables: " << modifiedTables << std::endl;

		handleFillSetTreeNodeFieldValuesXML(xmlOut,cfgMgr,configGroup,ConfigurationGroupKey(configGroupKey),
				startPath,modifiedTables,recordList,fieldList,valueList,userInfo.username_);

	}
	else if(requestType == "addTreeNodeRecords")
	{
		std::string 	configGroup 	= CgiDataUtilities::getData(cgiIn,"configGroup");
		std::string 	configGroupKey 	= CgiDataUtilities::getData(cgiIn,"configGroupKey");
		std::string 	startPath 		= CgiDataUtilities::postData(cgiIn,"startPath");
		std::string 	modifiedTables 	= CgiDataUtilities::postData(cgiIn,"modifiedTables");
		std::string 	recordList	 	= CgiDataUtilities::postData(cgiIn,"recordList");

		__SUP_COUT__ << "configGroup: " << configGroup << std::endl;
		__SUP_COUT__ << "configGroupKey: " << configGroupKey << std::endl;
		__SUP_COUT__ << "startPath: " << startPath << std::endl;
		__SUP_COUT__ << "recordList: " << recordList << std::endl;
		__SUP_COUT__ << "modifiedTables: " << modifiedTables << std::endl;

		handleFillCreateTreeNodeRecordsXML(xmlOut,cfgMgr,configGroup,ConfigurationGroupKey(configGroupKey),
				startPath,modifiedTables,recordList,userInfo.username_);
	}
	else if(requestType == "deleteTreeNodeRecords")
	{
		std::string 	configGroup 	= CgiDataUtilities::getData(cgiIn,"configGroup");
		std::string 	configGroupKey 	= CgiDataUtilities::getData(cgiIn,"configGroupKey");
		std::string 	startPath 		= CgiDataUtilities::postData(cgiIn,"startPath");
		std::string 	modifiedTables 	= CgiDataUtilities::postData(cgiIn,"modifiedTables");
		std::string 	recordList	 	= CgiDataUtilities::postData(cgiIn,"recordList");

		__SUP_COUT__ << "configGroup: " << configGroup << std::endl;
		__SUP_COUT__ << "configGroupKey: " << configGroupKey << std::endl;
		__SUP_COUT__ << "startPath: " << startPath << std::endl;
		__SUP_COUT__ << "recordList: " << recordList << std::endl;
		__SUP_COUT__ << "modifiedTables: " << modifiedTables << std::endl;

		handleFillDeleteTreeNodeRecordsXML(xmlOut,cfgMgr,configGroup,ConfigurationGroupKey(configGroupKey),
				startPath,modifiedTables,recordList);
	}
	else if(requestType == "getAffectedActiveGroups")
	{
		std::string 	groupName 		= CgiDataUtilities::getData(cgiIn,"groupName");
		std::string 	groupKey 		= CgiDataUtilities::getData(cgiIn,"groupKey");
		std::string 	modifiedTables 	= CgiDataUtilities::postData(cgiIn,"modifiedTables");
		__SUP_COUT__ << "modifiedTables: " << modifiedTables << std::endl;
		__SUP_COUT__ << "groupName: " << groupName << std::endl;
		__SUP_COUT__ << "groupKey: " << groupKey << std::endl;

		handleGetAffectedGroupsXML(xmlOut,cfgMgr,groupName,ConfigurationGroupKey(groupKey),
				modifiedTables);
	}
	else if(requestType == "saveTreeNodeEdit")
	{
		std::string 	editNodeType 		= CgiDataUtilities::getData(cgiIn,"editNodeType");
		std::string 	targetTable 		= CgiDataUtilities::getData(cgiIn,"targetTable");
		std::string 	targetTableVersion 	= CgiDataUtilities::getData(cgiIn,"targetTableVersion");
		std::string 	targetUID 			= CgiDataUtilities::getData(cgiIn,"targetUID");
		std::string 	targetColumn 		= CgiDataUtilities::getData(cgiIn,"targetColumn");
		std::string 	newValue 			= CgiDataUtilities::postData(cgiIn,"newValue");

		__SUP_COUT__ << "editNodeType: " << editNodeType << std::endl;
		__SUP_COUT__ << "targetTable: " << targetTable << std::endl;
		__SUP_COUT__ << "targetTableVersion: " << targetTableVersion << std::endl;
		__SUP_COUT__ << "targetUID: " << targetUID << std::endl;
		__SUP_COUT__ << "targetColumn: " << targetColumn << std::endl;
		__SUP_COUT__ << "newValue: " << newValue << std::endl;

		handleSaveTreeNodeEditXML(xmlOut,cfgMgr,targetTable,ConfigurationVersion(targetTableVersion),
				editNodeType,
				CgiDataUtilities::decodeURIComponent(targetUID),
				CgiDataUtilities::decodeURIComponent(targetColumn),
				newValue,userInfo.username_);
	}
	else if(requestType == "getLinkToChoices")
	{
		std::string 	linkToTableName 	= CgiDataUtilities::getData(cgiIn,"linkToTableName");
		std::string 	linkToTableVersion 	= CgiDataUtilities::getData(cgiIn,"linkToTableVersion");
		std::string 	linkIdType 			= CgiDataUtilities::getData(cgiIn,"linkIdType");
		std::string 	linkIndex 			= CgiDataUtilities::getData(cgiIn,"linkIndex");
		std::string 	linkInitId 			= CgiDataUtilities::getData(cgiIn,"linkInitId");

		__SUP_COUT__ << "linkToTableName: " << linkToTableName << std::endl;
		__SUP_COUT__ << "linkToTableVersion: " << linkToTableVersion << std::endl;
		__SUP_COUT__ << "linkIdType: " << linkIdType << std::endl;
		__SUP_COUT__ << "linkIndex: " << linkIndex << std::endl;
		__SUP_COUT__ << "linkInitId: " << linkInitId << std::endl;

		handleGetLinkToChoicesXML(xmlOut,cfgMgr,linkToTableName,
				ConfigurationVersion(linkToTableVersion),linkIdType,linkIndex,linkInitId);
	}
	else if(requestType == "activateConfigGroup")
	{
		std::string 	groupName 		= CgiDataUtilities::getData(cgiIn,"groupName");
		std::string 	groupKey 		= CgiDataUtilities::getData(cgiIn,"groupKey");
		bool		 	ignoreWarnings 	= CgiDataUtilities::getDataAsInt(cgiIn,"ignoreWarnings");

		__SUP_COUT__ << "Activating config: " << groupName <<
				"(" << groupKey << ")" << std::endl;
		__SUP_COUT__ << "ignoreWarnings: " << ignoreWarnings << std::endl;

		//add flag for GUI handling
		xmlOut.addTextElementToData("AttemptedGroupActivation","1");
		xmlOut.addTextElementToData("AttemptedGroupActivationName",groupName);
		xmlOut.addTextElementToData("AttemptedGroupActivationKey",groupKey);

		std::string accumulatedTreeErrors;
		try
		{
			cfgMgr->activateConfigurationGroup(groupName, ConfigurationGroupKey(groupKey),
					ignoreWarnings?0:&accumulatedTreeErrors); //if ignore warning then pass null
		}
		catch(std::runtime_error& e)
		{
			//NOTE it is critical for flimsy error parsing in JS GUI to leave
			//	single quotes around the groupName and groupKey and have them be
			//	the first single quotes encountered in the error mesage!
			__SUP_COUT__ << "Error detected!\n\n " << e.what() << std::endl;
			xmlOut.addTextElementToData("Error", "Error activating config group '" +
					groupName +	"(" + groupKey + ")" + ".' Please see details below:\n\n" +
					std::string(e.what()));
			__SUP_COUT_ERR__ << "Errors detected so de-activating group: " <<
					groupName << " (" << groupKey << ")" << std::endl;
			try //just in case any lingering pieces, lets deactivate
			{ cfgMgr->destroyConfigurationGroup(groupName,true); }
			catch(...){}
		}
		catch(cet::exception& e)
		{
			//NOTE it is critical for flimsy error parsing in JS GUI to leave
			//	single quotes around the groupName and groupKey and have them be
			//	the first single quotes encountered in the error mesage!

			__SUP_COUT__ << "Error detected!\n\n " << e.what() << std::endl;
			xmlOut.addTextElementToData("Error", "Error activating config group '" +
					groupName +	"(" + groupKey + ")" + "!'\n\n" +
					std::string(e.what()));
			__SUP_COUT_ERR__ << "Errors detected so de-activating group: " <<
					groupName << " (" << groupKey << ")" << std::endl;
			try	//just in case any lingering pieces, lets deactivate
			{ cfgMgr->destroyConfigurationGroup(groupName,true); }
			catch(...){}
		}
		catch(...)
		{
			__SUP_COUT__ << "Error detected!" << std::endl;
			throw; //unexpected exception!
		}

		if(accumulatedTreeErrors != "")
			xmlOut.addTextElementToData("Error", "Warnings were found when activating group '" +
					groupName +	"(" + groupKey + ")" + "'! Please see details below:\n\n" +
					accumulatedTreeErrors);
	}
	else if(requestType == "getActiveConfigGroups"); //do nothing, since they are always returned
	else if(requestType == "copyViewToCurrentColumns")
	{
		std::string 	configName 		= CgiDataUtilities::getData(cgiIn,"configName"); //from GET
		std::string 	sourceVersion 	= CgiDataUtilities::getData(cgiIn,"sourceVersion");

		__SUP_COUT__ << "configName: " << configName << std::endl;
		__SUP_COUT__ << "sourceVersion: " << sourceVersion << std::endl;
		__SUP_COUT__ << "userInfo.username_: " << userInfo.username_ << std::endl;

		//copy source version to new temporary version
		ConfigurationVersion newTemporaryVersion;
		try
		{
			//force emptying of cache for this configuration
			newTemporaryVersion = cfgMgr->copyViewToCurrentColumns(configName,
					ConfigurationVersion(sourceVersion));
			//
			//			getConfigurationByName(configName)->reset();
			//
			//			//make sure source version is loaded
			//			//need to load with loose column rules!
			//			config = cfgMgr->getVersionedConfigurationByName(configName,
			//					ConfigurationVersion(sourceVersion), true);
			//
			//			//copy from source version to a new temporary version
			//			newTemporaryVersion = config->copyView(config->getView(),
			//							ConfigurationVersion(),userName);

			__SUP_COUT__ << "New temporary version = " << newTemporaryVersion << std::endl;
		}
		catch(std::runtime_error& e)
		{
			__SUP_COUT__ << "Error detected!\n\n " << e.what() << std::endl;
			xmlOut.addTextElementToData("Error", "Error copying view from '" +
					configName +	"_v" + sourceVersion + "'! " +
					std::string(e.what()));
		}
		catch(...)
		{
			__SUP_COUT__ << "Error detected!\n\n " << std::endl;
			xmlOut.addTextElementToData("Error", "Error copying view from '" +
					configName +	"_v" + sourceVersion + "'! ");
		}

		handleGetConfigurationXML(xmlOut,cfgMgr,configName,newTemporaryVersion);
	}
	else if(requestType == "getLastConfigGroups")
	{
		XDAQ_CONST_CALL xdaq::ApplicationDescriptor* gatewaySupervisor =
				allSupervisorInfo_.isWizardMode()?
						allSupervisorInfo_.getWizardDescriptor():
						allSupervisorInfo_.getGatewayDescriptor();

		std::string timeString;
		std::pair<std::string /*group name*/, ConfigurationGroupKey> theGroup =
				theRemoteWebUsers_.getLastConfigGroup(gatewaySupervisor,
						"Configured",timeString);
		xmlOut.addTextElementToData("LastConfiguredGroupName",theGroup.first);
		xmlOut.addTextElementToData("LastConfiguredGroupKey",theGroup.second.toString());
		xmlOut.addTextElementToData("LastConfiguredGroupTime",timeString);
		theGroup = theRemoteWebUsers_.getLastConfigGroup(gatewaySupervisor,
						"Started",timeString);
		xmlOut.addTextElementToData("LastStartedGroupName",theGroup.first);
		xmlOut.addTextElementToData("LastStartedGroupKey",theGroup.second.toString());
		xmlOut.addTextElementToData("LastStartedGroupTime",timeString);
	}
	else if(requestType == "savePlanCommandSequence")
	{
		std::string 	planName 		= CgiDataUtilities::getData(cgiIn,"planName"); //from GET
		std::string 	commands 		= CgiDataUtilities::postData(cgiIn,"commands"); //from POST
		std::string 	modifiedTables 	= CgiDataUtilities::postData(cgiIn,"modifiedTables");
		std::string 	groupName 		= CgiDataUtilities::getData(cgiIn,"groupName");
		std::string 	groupKey 		= CgiDataUtilities::getData(cgiIn,"groupKey");

		__SUP_COUT__ << "modifiedTables: " << modifiedTables << std::endl;
		__SUP_COUT__ << "planName: " << planName << std::endl;
		__SUP_COUT__ << "commands: " << commands << std::endl;
		__SUP_COUT__ << "groupName: " << groupName << std::endl;
		__SUP_COUT__ << "groupKey: " << groupKey << std::endl;

		handleSavePlanCommandSequenceXML(xmlOut,cfgMgr,groupName,ConfigurationGroupKey(groupKey),
				modifiedTables,userInfo.username_,planName,commands);
	}
	else if(requestType == "mergeGroups")
	{
		std::string 	groupANameContext 		= CgiDataUtilities::getData(cgiIn,"groupANameContext");
		std::string 	groupAKeyContext 		= CgiDataUtilities::getData(cgiIn,"groupAKeyContext");
		std::string 	groupBNameContext 		= CgiDataUtilities::getData(cgiIn,"groupBNameContext");
		std::string 	groupBKeyContext 		= CgiDataUtilities::getData(cgiIn,"groupBKeyContext");
		std::string 	groupANameConfig 		= CgiDataUtilities::getData(cgiIn,"groupANameConfig");
		std::string 	groupAKeyConfig 		= CgiDataUtilities::getData(cgiIn,"groupAKeyConfig");
		std::string 	groupBNameConfig 		= CgiDataUtilities::getData(cgiIn,"groupBNameConfig");
		std::string 	groupBKeyConfig 		= CgiDataUtilities::getData(cgiIn,"groupBKeyConfig");
		std::string 	mergeApproach	= CgiDataUtilities::getData(cgiIn,"mergeApproach");

		__SUP_COUTV__(groupANameContext);
		__SUP_COUTV__(groupAKeyContext);
		__SUP_COUTV__(groupBNameContext);
		__SUP_COUTV__(groupBKeyContext);
		__SUP_COUTV__(groupANameConfig);
		__SUP_COUTV__(groupAKeyConfig);
		__SUP_COUTV__(groupBNameConfig);
		__SUP_COUTV__(groupBKeyConfig);
		__SUP_COUTV__(mergeApproach);

		handleMergeGroupsXML(xmlOut,cfgMgr,
				groupANameContext,ConfigurationGroupKey(groupAKeyContext),
				groupBNameContext,ConfigurationGroupKey(groupBKeyContext),
				groupANameConfig,ConfigurationGroupKey(groupAKeyConfig),
				groupBNameConfig,ConfigurationGroupKey(groupBKeyConfig),
				userInfo.username_,mergeApproach);
	}
	else
	{
		__SUP_SS__ << "requestType '" << requestType << "' request not recognized." << std::endl;
		__SUP_COUT__ << "\n" << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
	}

	//__SUP_COUT__ << "Wrapping up..." << std::endl;

	//always add active config groups to xml response
	std::map<std::string /*type*/,
		std::pair<
			std::string /*groupName*/,
			ConfigurationGroupKey> > activeGroupMap =
					cfgMgr->getActiveConfigurationGroups();

	for(auto& type:activeGroupMap)
	{
		xmlOut.addTextElementToData(type.first + "-ActiveGroupName",
				type.second.first);
		xmlOut.addTextElementToData(type.first + "-ActiveGroupKey",
				type.second.second.toString());
		//__SUP_COUT__ << "ActiveGroup " << type.first << " " << type.second.first << "(" << type.second.second << ")" << __E__;
	}

	//always add version tracking bool
	xmlOut.addTextElementToData("versionTracking",
			ConfigurationInterface::isVersionTrackingEnabled()?"ON":"OFF");
//
//	__SUP_COUT__ << __E__;
//	xmlOut.outputXmlDocument(0,true,true);
//	__SUP_COUT__ << __E__;

} //end ::request()
catch(const std::runtime_error& e)
{
	__SS__ << "A fatal error occurred while handling the request '" <<
			requestType << ".' Error: " <<
			e.what() << __E__;
	__COUT_ERR__ << "\n" << ss.str();
	xmlOut.addTextElementToData("Error", ss.str());

	try
	{
		//always add version tracking bool
		xmlOut.addTextElementToData("versionTracking",
				ConfigurationInterface::isVersionTrackingEnabled()?"ON":"OFF");
	}
	catch(...)
	{
		__COUT_ERR__ << "Error getting version tracking status!" << __E__;
	}
}
catch(...)
{
	__SS__ << "A fatal error occurred while handling the request '" <<
			requestType << ".'" << __E__;
	__COUT_ERR__ << "\n" << ss.str();
	xmlOut.addTextElementToData("Error", ss.str());

	try
	{
		//always add version tracking bool
		xmlOut.addTextElementToData("versionTracking",
				ConfigurationInterface::isVersionTrackingEnabled()?"ON":"OFF");
	}
	catch(...)
	{
		__COUT_ERR__ << "Error getting version tracking status!" << __E__;
	}
}


//========================================================================================================================
//handleGetAffectedGroupsXML
//	checks which of the active groups are affected
//		by the tables changing in the modified tables list.
//
//	returns for each group affected:
//		the group name/key affected
//			and modified member map
void ConfigurationGUISupervisor::handleGetAffectedGroupsXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr,
		const std::string& rootGroupName, const ConfigurationGroupKey& rootGroupKey,
		const std::string& modifiedTables)
try
{

	//determine type of rootGroup
	//replace the matching type in considered groups
	//for each considered configuration group
	//
	//	check if there is a modified table that is also a member of that group
	//	if so,
	//		make xml entry pair


	std::map<std::string, std::pair<std::string, ConfigurationGroupKey>> consideredGroups =
			cfgMgr->getActiveConfigurationGroups();


	//determine the type of configuration group
	try
	{
		std::map<std::string /*name*/, ConfigurationVersion /*version*/> rootGroupMemberMap;

		cfgMgr->loadConfigurationGroup(rootGroupName,rootGroupKey,
						0,&rootGroupMemberMap,0,0,0,0,0, //defaults
						true); //doNotLoadMember

		const std::string& groupType = cfgMgr->getTypeNameOfGroup(rootGroupMemberMap);

		consideredGroups[groupType] = std::pair<std::string, ConfigurationGroupKey>(
				rootGroupName,rootGroupKey);
	}
	catch(const std::runtime_error& e)
	{
		//if actual group name was attempted re-throw
		if(rootGroupName.size())
		{

			__SUP_SS__ << "Failed to determine type of configuration group for " << rootGroupName << "(" <<
					rootGroupKey << ")! " << e.what() << std::endl;
			__SUP_COUT_ERR__ << "\n" << ss.str();
			throw std::runtime_error(ss.str());
		}

		//else assume it was the intention to just consider the active groups
		__SUP_COUT__ << "Did not modify considered active groups due to empty root group name - assuming this was intentional." << std::endl;
	}
	catch(...)
	{
		//if actual group name was attempted re-throw
		if(rootGroupName.size())
		{
			__SUP_COUT_ERR__ << "Failed to determine type of configuration group for " << rootGroupName << "(" <<
					rootGroupKey << ")!" << std::endl;
			throw;
		}

		//else assume it was the intention to just consider the active groups
		__SUP_COUT__ << "Did not modify considered active groups due to empty root group name - assuming this was intentional." << std::endl;
	}


	std::map<std::string /*name*/, ConfigurationVersion /*version*/> modifiedTablesMap;
	std::map<std::string /*name*/, ConfigurationVersion /*version*/>::iterator modifiedTablesMapIt;
	{
		std::istringstream f(modifiedTables);
		std::string table,version;
		while (getline(f, table, ','))
		{
			getline(f, version, ',');
			modifiedTablesMap.insert(
					std::pair<std::string /*name*/,
					ConfigurationVersion /*version*/>(
							table, ConfigurationVersion(version)));
		}
		__SUP_COUT__ << modifiedTables << std::endl;
		for(auto& pair:modifiedTablesMap)
			__SUP_COUT__ << "modified table " <<
			pair.first << ":" <<
			pair.second << std::endl;
	}

	bool affected;
	DOMElement* parentEl;
	std::string groupComment;
	for(auto group : consideredGroups)
	{
		if(group.second.second.isInvalid()) continue; //skip invalid

		__SUP_COUT__ << "Considering " << group.first << " group " <<
				group.second.first << " (" << group.second.second << ")" << std::endl;

		affected = false;

		std::map<std::string /*name*/, ConfigurationVersion /*version*/> memberMap;
		cfgMgr->loadConfigurationGroup(group.second.first,group.second.second,
				0,&memberMap,0,0,&groupComment,0,0, //mostly defaults
				true /*doNotLoadMember*/);

		__SUP_COUT__ << "groupComment = " << groupComment << std::endl;

		for(auto& table: memberMap)
		{
			if((modifiedTablesMapIt = modifiedTablesMap.find(table.first)) !=
					modifiedTablesMap.end() && //check if version is different for member table
					table.second != (*modifiedTablesMapIt).second)
			{
				__SUP_COUT__ << "Affected by " <<
						(*modifiedTablesMapIt).first << ":" <<
						(*modifiedTablesMapIt).second << std::endl;
				affected = true;
				memberMap[table.first] = (*modifiedTablesMapIt).second;
			}
		}

		if(affected)
		{

			parentEl = xmlOut.addTextElementToData("AffectedActiveGroup", "");
			xmlOut.addTextElementToParent("GroupName", group.second.first, parentEl);
			xmlOut.addTextElementToParent("GroupKey",
					group.second.second.toString(), parentEl);
			xmlOut.addTextElementToParent("GroupComment", groupComment, parentEl);


			for(auto& table: memberMap)
			{
				xmlOut.addTextElementToParent("MemberName",
						table.first, parentEl);
				xmlOut.addTextElementToParent("MemberVersion",
						table.second.toString(), parentEl);
			}
		}
	}
}
catch(std::runtime_error& e)
{
	__SUP_COUT__ << "Error detected!\n\n " << e.what() << std::endl;
	xmlOut.addTextElementToData("Error", "Error getting affected groups! " + std::string(e.what()));
}
catch(...)
{
	__SUP_COUT__ << "Error detected!\n\n "<< std::endl;
	xmlOut.addTextElementToData("Error", "Error getting affected groups! ");
}


//========================================================================================================================
//setupActiveTables
//	setup active tables based on input group and modified tables
//
//	if groupName == "" || groupKey is invalid
//		then do for active groups
//	if valid, then replace appropriate active group with specified group
//	Also replace active versions of modified tables with the specified version
void ConfigurationGUISupervisor::setupActiveTablesXML(
		HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr,
		const std::string& groupName, const ConfigurationGroupKey& groupKey,
		const std::string& modifiedTables,
		bool refreshAll, bool doGetGroupInfo,
		std::map<std::string /*name*/, ConfigurationVersion /*version*/>* returnMemberMap,
		bool outputActiveTables,
		std::string* accumulatedErrors)
try
{
	if(accumulatedErrors)* accumulatedErrors = "";

	xmlOut.addTextElementToData("configGroup", groupName);
	xmlOut.addTextElementToData("configGroupKey", groupKey.toString());

	bool usingActiveGroups = (groupName == "" || groupKey.isInvalid());

	//reload all tables so that partially loaded tables are not allowed
	if(usingActiveGroups || refreshAll)
		cfgMgr->getAllConfigurationInfo(true,accumulatedErrors); //do refresh

	const std::map<std::string, ConfigurationInfo>& allCfgInfo = cfgMgr->getAllConfigurationInfo(false);

	std::map<std::string /*name*/, ConfigurationVersion /*version*/> modifiedTablesMap;
	std::map<std::string /*name*/, ConfigurationVersion /*version*/>::iterator modifiedTablesMapIt;

	if(usingActiveGroups)
	{
		//no need to load a target group
		__SUP_COUT__ << "Using active groups." << std::endl;
	}
	else
	{
		std::string groupComment, groupAuthor, configGroupCreationTime;

		//only same member map if object pointer was passed
		cfgMgr->loadConfigurationGroup(groupName,groupKey,
								0,returnMemberMap,0,0,
								doGetGroupInfo?&groupComment:0,
								doGetGroupInfo?&groupAuthor:0,
								doGetGroupInfo?&configGroupCreationTime:0);

		if(doGetGroupInfo)
		{
			xmlOut.addTextElementToData("configGroupComment", groupComment);
			xmlOut.addTextElementToData("configGroupAuthor", groupAuthor);
			xmlOut.addTextElementToData("configGroupCreationTime", configGroupCreationTime);
		}
	}

	//extract modified tables
	{
		std::istringstream f(modifiedTables);
		std::string table,version;
		while (getline(f, table, ','))
		{
			getline(f, version, ',');
			modifiedTablesMap.insert(
					std::pair<std::string /*name*/,
					ConfigurationVersion /*version*/>(
							table, ConfigurationVersion(version)));
		}
		//__SUP_COUT__ << modifiedTables << std::endl;
		for(auto& pair:modifiedTablesMap)
			__SUP_COUT__ << "modified table " <<
				pair.first << ":" <<
				pair.second << std::endl;
	}

	//add all active configuration pairs to xmlOut
	std::map<std::string, ConfigurationVersion> allActivePairs = cfgMgr->getActiveVersions();
	xmlOut.addTextElementToData("DefaultNoLink", ViewColumnInfo::DATATYPE_LINK_DEFAULT);
	for(auto& activePair: allActivePairs)
	{
		if(outputActiveTables)
			xmlOut.addTextElementToData("ActiveTableName", activePair.first);

		//check if name is in modifiedTables
		//if so, activate the temporary version
		if((modifiedTablesMapIt = modifiedTablesMap.find(activePair.first)) !=
				modifiedTablesMap.end())
		{
			__SUP_COUT__ << "Found modified table " <<
					(*modifiedTablesMapIt).first << ": trying... " <<
					(*modifiedTablesMapIt).second << std::endl;

			try
			{
				allCfgInfo.at(activePair.first).configurationPtr_->setActiveView(
						(*modifiedTablesMapIt).second);
			}
			catch(...)
			{
				__SUP_SS__ << "Modified table version v" << (*modifiedTablesMapIt).second <<
						" failed. Reverting to v" <<
						allCfgInfo.at(activePair.first).configurationPtr_->getView().getVersion() <<
						"." <<
						std::endl;
				__SUP_COUT_WARN__ << "Warning detected!\n\n " << ss.str() << std::endl;
				xmlOut.addTextElementToData("Warning", "Error setting up active tables!\n\n" +
						std::string(ss.str()));
			}
		}

		if(outputActiveTables)
		{
			xmlOut.addTextElementToData("ActiveTableVersion",
					allCfgInfo.at(activePair.first).configurationPtr_->getView().getVersion().toString());
			xmlOut.addTextElementToData("ActiveTableComment",
					allCfgInfo.at(activePair.first).configurationPtr_->getView().getComment());
		}

		//__SUP_COUT__ << "Active table = " <<
		//		activePair.first << "-v" <<
		//		allCfgInfo.at(activePair.first).configurationPtr_->getView().getVersion() << std::endl;
	}

}
catch(std::runtime_error& e)
{
	__SUP_SS__ << ("Error setting up active tables!\n\n" + std::string(e.what())) << std::endl;
	__SUP_COUT_ERR__ << "\n" << ss.str();
	xmlOut.addTextElementToData("Error", ss.str());
}
catch(...)
{
	__SUP_SS__ << ("Error setting up active tables!\n\n") << std::endl;
	__SUP_COUT_ERR__ << "\n" << ss.str();
	xmlOut.addTextElementToData("Error", ss.str());
}


//========================================================================================================================
//handleFillCreateTreeNodeRecordsXML
//	Creates the records in the appropriate table
//		and creates a temporary version.
//	the modified-<modified tables> list is returned in xml
//
// if groupName == "" || groupKey is invalid
//	 then do for active groups
//
//parameters
//	configGroupName (full name with key)
//	starting node path
//	modifiedTables := CSV of table/version pairs
//	recordList := CSV list of records to create
//
void ConfigurationGUISupervisor::handleFillCreateTreeNodeRecordsXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr,
		const std::string& groupName, const ConfigurationGroupKey& groupKey,
		const std::string& startPath,
		const std::string& modifiedTables, const std::string& recordList,
		const std::string& author)
{
	//	setup active tables based on input group and modified tables
	setupActiveTablesXML(
			xmlOut,
			cfgMgr,
			groupName, groupKey,
			modifiedTables,
			true /* refresh all */, false /* getGroupInfo */,
			0 /* returnMemberMap */, false /* outputActiveTables */);

	try
	{

		ConfigurationTree targetNode =
				cfgMgr->getNode(startPath);
		ConfigurationBase* config = cfgMgr->getConfigurationByName(
									targetNode.getConfigurationName());

		__SUP_COUT__ << config->getConfigurationName() << std::endl;
		ConfigurationVersion temporaryVersion;

		// if current version is not temporary
		//		create temporary
		//	else re-modify temporary version
		//	edit temporary version directly
		//	then after all edits return active versions
		//

		bool firstSave = true;


		//save current version
		ConfigurationView backupView;


		//extract record list
		{
			std::istringstream f(recordList);
			std::string recordUID;
			unsigned int i;

			while (getline(f, recordUID, ',')) //for each record
			{
				recordUID = StringMacros::decodeURIComponent(recordUID);

				__SUP_COUT__ << "recordUID " <<
						recordUID << std::endl;

				if(firstSave) //handle version bookkeeping
				{
					if(!(temporaryVersion =
							targetNode.getConfigurationVersion()).isTemporaryVersion())
					{
						__SUP_COUT__ << "Start version " << temporaryVersion << std::endl;
						//create temporary version for editing
						temporaryVersion = config->createTemporaryView(temporaryVersion);
						cfgMgr->saveNewConfiguration(
								targetNode.getConfigurationName(),
								temporaryVersion, true); //proper bookkeeping for temporary version with the new version

						__SUP_COUT__ << "Created temporary version " << temporaryVersion << std::endl;
					}
					else //else table is already temporary version
						__SUP_COUT__ << "Using temporary version " << temporaryVersion << std::endl;

					firstSave = false;

					//copy original to backup before modifying
					backupView.copy(config->getView(),temporaryVersion,author);
				}

				//at this point have valid temporary version to edit

				//copy "table-newRow" type edit from handleSaveTreeNodeEditXML() functionality

				//add row
				unsigned int row = config->getViewP()->addRow(author,
						true /*incrementUniqueData*/); //increment all unique data fields to void conflict

				//if ViewColumnInfo::COL_NAME_STATUS exists, set it to true
				try
				{
					unsigned int col = config->getViewP()->getColStatus();
					config->getViewP()->setURIEncodedValue("1",row,col);
				}
				catch(...) {} //if not, ignore

				//set UID value
				config->getViewP()->setURIEncodedValue(recordUID,row,
						config->getViewP()->getColUID());

			}
		}

		if(!firstSave) //only test table if there was a change
		{
			try
			{
				config->getViewP()->init(); //verify new table (throws runtime_errors)
			}
			catch(...)
			{
				__SUP_COUT_INFO__ << "Reverting to original view." << __E__;
				__SUP_COUT__ << "Before:" << __E__;
				config->getViewP()->print();
				config->getViewP()->copy(backupView,temporaryVersion,author);
				__SUP_COUT__ << "After:" << __E__;
				config->getViewP()->print();

				throw; //rethrow
			}
		}

		handleFillModifiedTablesXML(xmlOut,cfgMgr);
	}
	catch(std::runtime_error& e)
	{
		__SUP_SS__ << ("Error creating new record(s)!\n\n" + std::string(e.what())) << std::endl;
		__SUP_COUT_ERR__ << "\n" << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
	}
	catch(...)
	{
		__SUP_SS__ << ("Error creating new record(s)!\n\n") << std::endl;
		__SUP_COUT_ERR__ << "\n" << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
	}
}

//========================================================================================================================
//handleFillModifiedTablesXML
//	fills <modified tables> as used by ConfigurationAPI
void ConfigurationGUISupervisor::handleFillModifiedTablesXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr)
try
{
	//return modified <modified tables>
	const std::map<std::string, ConfigurationInfo>& allCfgInfo = cfgMgr->getAllConfigurationInfo();
	std::map<std::string, ConfigurationVersion> allActivePairs = cfgMgr->getActiveVersions();
	for(auto& activePair: allActivePairs)
	{
		xmlOut.addTextElementToData("NewActiveTableName", activePair.first);
		xmlOut.addTextElementToData("NewActiveTableVersion",
				allCfgInfo.at(activePair.first).configurationPtr_->getView().getVersion().toString());
		xmlOut.addTextElementToData("NewActiveTableComment",
				allCfgInfo.at(activePair.first).configurationPtr_->getView().getComment());
	}
}
catch(std::runtime_error& e)
{
	__SUP_SS__ << ("Error!\n\n" + std::string(e.what())) << std::endl;
	__SUP_COUT_ERR__ << "\n" << ss.str();
	xmlOut.addTextElementToData("Error", ss.str());
}
catch(...)
{
	__SUP_SS__ << ("Error!\n\n") << std::endl;
	__SUP_COUT_ERR__ << "\n" << ss.str();
	xmlOut.addTextElementToData("Error", ss.str());
}

//========================================================================================================================
//handleFillDeleteTreeNodeRecordsXML
//	Deletes the records in the appropriate table
//		and creates a temporary version.
//	the modified-<modified tables> list is returned in xml
//
// if groupName == "" || groupKey is invalid
//	 then do for active groups
//
//parameters
//	configGroupName (full name with key)
//	starting node path
//	modifiedTables := CSV of table/version pairs
//	recordList := CSV list of records to create
//
void ConfigurationGUISupervisor::handleFillDeleteTreeNodeRecordsXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr,
		const std::string& groupName, const ConfigurationGroupKey& groupKey,
		const std::string& startPath,
		const std::string& modifiedTables, const std::string& recordList)
{
	//	setup active tables based on input group and modified tables
	setupActiveTablesXML(
			xmlOut,
			cfgMgr,
			groupName, groupKey,
			modifiedTables,
			true /* refresh all */, false /* getGroupInfo */,
			0 /* returnMemberMap */, false /* outputActiveTables */);

	try
	{

		ConfigurationTree targetNode =
				cfgMgr->getNode(startPath);
		ConfigurationBase* config = cfgMgr->getConfigurationByName(
				targetNode.getConfigurationName());

		__SUP_COUT__ << config->getConfigurationName() << std::endl;
		ConfigurationVersion temporaryVersion;

		// if current version is not temporary
		//		create temporary
		//	else re-modify temporary version
		//	edit temporary version directly
		//	then after all edits return active versions
		//

		bool firstSave = true;



		//extract record list
		{
			std::istringstream f(recordList);
			std::string recordUID;
			unsigned int i;

			while (getline(f, recordUID, ',')) //for each record
			{
				recordUID = StringMacros::decodeURIComponent(recordUID);

				__SUP_COUT__ << "recordUID " <<
						recordUID << std::endl;

				if(firstSave) //handle version bookkeeping
				{
					if(!(temporaryVersion =
							targetNode.getConfigurationVersion()).isTemporaryVersion())
					{
						__SUP_COUT__ << "Start version " << temporaryVersion << std::endl;
						//create temporary version for editing
						temporaryVersion = config->createTemporaryView(temporaryVersion);
						cfgMgr->saveNewConfiguration(
								targetNode.getConfigurationName(),
								temporaryVersion, true); //proper bookkeeping for temporary version with the new version

						__SUP_COUT__ << "Created temporary version " << temporaryVersion << std::endl;
					}
					else //else table is already temporary version
						__SUP_COUT__ << "Using temporary version " << temporaryVersion << std::endl;

					firstSave = false;
				}

				//at this point have valid temporary version to edit

				//copy "delete-uid" type edit from handleSaveTreeNodeEditXML() functionality
				unsigned int row = config->getViewP()->findRow(
						config->getViewP()->getColUID(),
						recordUID
					);
				config->getViewP()->deleteRow(row);
			}
		}

		if(!firstSave) //only test table if there was a change
			config->getViewP()->init(); //verify new table (throws runtime_errors)

		handleFillModifiedTablesXML(xmlOut,cfgMgr);
	}
	catch(std::runtime_error& e)
	{
		__SUP_SS__ << ("Error removing record(s)!\n\n" + std::string(e.what())) << std::endl;
		__SUP_COUT_ERR__ << "\n" << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
	}
	catch(...)
	{
		__SUP_SS__ << ("Error removing record(s)!\n\n") << std::endl;
		__SUP_COUT_ERR__ << "\n" << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
	}
}


//========================================================================================================================
//handleFillSetTreeNodeFieldValuesXML
//	writes for each record, the field/value pairs to the appropriate table
//		and creates a temporary version.
//	the modified-<modified tables> list is returned in xml
//
// if groupName == "" || groupKey is invalid
//	 then do for active groups
//
//parameters
//	configGroupName (full name with key)
//	starting node path
//	modifiedTables := CSV of table/version pairs
//	recordList := CSV list of records for which to write values for fields
//	fieldList := CSV of relative-to-record-path to fields to write to each record
//	valueList := CSV of values corresponding to fields
//
void ConfigurationGUISupervisor::handleFillSetTreeNodeFieldValuesXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr,
		const std::string& groupName, const ConfigurationGroupKey& groupKey,
		const std::string& startPath,
		const std::string& modifiedTables, const std::string& recordList,
		const std::string& fieldList, const std::string& valueList,
		const std::string& author)
{
	//	setup active tables based on input group and modified tables
	setupActiveTablesXML(
			xmlOut,
			cfgMgr,
			groupName, groupKey,
			modifiedTables,
			true /* refresh all */, false /* getGroupInfo */,
			0 /* returnMemberMap */, false /* outputActiveTables */);

	//for each field
	//	return field/value pair in xml

	try
	{
		std::vector<std::string /*relative-path*/> fieldPaths;
		//extract field list
		{
			std::istringstream f(fieldList);
			std::string fieldPath;
			while (getline(f, fieldPath, ','))
			{
				fieldPaths.push_back(
						StringMacros::decodeURIComponent(fieldPath));
			}
			__SUP_COUT__ << fieldList << std::endl;
			for(const auto& field:fieldPaths)
				__SUP_COUT__ << "fieldPath " <<
					field << std::endl;
		}

		std::vector<std::string /*relative-path*/> fieldValues;
		//extract value list
		{
			std::istringstream f(valueList);
			std::string fieldValue;
			while (getline(f, fieldValue, ','))
			{
				fieldValues.push_back(fieldValue); //setURIEncodedValue is expected
						//StringMacros::decodeURIComponent(fieldValue));
			}

			//if last value is "" then push empty value
			if(valueList.size() &&
					valueList[valueList.size()-1] == ',')
				fieldValues.push_back("");

			__SUP_COUT__ << valueList << std::endl;
			for(const auto& value:fieldValues)
				__SUP_COUT__ << "fieldValue " <<
					value << std::endl;
		}

		if(fieldPaths.size() != fieldValues.size())
			{__SUP_SS__; throw std::runtime_error(ss.str()+"Mismatch in fields and values array size!");}

		//extract record list
		{
			ConfigurationBase* config;
			ConfigurationVersion temporaryVersion;
			std::istringstream f(recordList);
			std::string recordUID;
			unsigned int i;

			while (getline(f, recordUID, ',')) //for each record
			{
				recordUID = StringMacros::decodeURIComponent(recordUID);

				//__SUP_COUT__ << "recordUID " <<	recordUID << std::endl;

				DOMElement* parentEl = xmlOut.addTextElementToData("fieldValues", recordUID);

				//for each field, set value
				for(i=0;i<fieldPaths.size();++i)
				{
					__SUP_COUT__ << "fieldPath " << fieldPaths[i] << std::endl;
					__SUP_COUT__ << "fieldValue " << fieldValues[i] << std::endl;

					//doNotThrowOnBrokenUIDLinks so that link UIDs can be edited like other fields
					ConfigurationTree targetNode =
							cfgMgr->getNode(startPath + "/" + recordUID + "/" + fieldPaths[i],
									true /*doNotThrowOnBrokenUIDLinks*/);

					//need table, uid, columnName to set a value

					//assume correct table version is loaded by setupActiveTablesXML()
					//config = cfgMgr->getConfigurationByName(
					//							targetNode.getConfigurationName());
					//
					//__SUP_COUT__ << "Active version is " << config->getViewVersion() << std::endl;

					//mimic handleSaveTreeNodeEditXML L 1750
					//					Actually call it! ..
					//					with a modifier?
					//					or
					//					handleSaveTreeNodeEditXML(xmlOut,
					//							cfgMgr,
					//							targetNode.getConfigurationName(),
					//							targetNode.getConfigurationVersion(),
					//							"value",
					//							targetNode.getUIDAsString(),
					//							targetNode.getValueName(), //col name
					//							fieldValues[i]
					//							);

					//or
					// 	(because problem is this would create a new temporary version each time)
					// if current version is not temporary
					//		create temporary
					//	else re-modify temporary version
					//	edit temporary version directly
					//	then after all edits return active versions
					//

					__SUP_COUT__ << "Getting table " <<
							targetNode.getFieldConfigurationName() << std::endl;

					//if link must get parent config name
					config = cfgMgr->getConfigurationByName(
							targetNode.getFieldConfigurationName()); //NOT getConfigurationName!
					if(!(temporaryVersion =
							config->getViewP()->getVersion()).isTemporaryVersion())
					{
						//create temporary version for editing
						temporaryVersion = config->createTemporaryView(
								config->getViewP()->getVersion());
						cfgMgr->saveNewConfiguration(
								config->getConfigurationName(),
								temporaryVersion, true); //proper bookkeeping for temporary version with the new version

						__SUP_COUT__ << "Created temporary version " <<
								config->getConfigurationName() << "-v" << temporaryVersion << std::endl;
					}
					//else //else table is already temporary version
					__SUP_COUT__ << "Using temporary version " <<
						config->getConfigurationName() << "-v" << temporaryVersion << std::endl;

					//copy "value" type edit from handleSaveTreeNodeEditXML() functionality
					config->getViewP()->setURIEncodedValue(
							fieldValues[i],
							targetNode.getFieldRow(),
							targetNode.getFieldColumn(),
							author);

					config->getViewP()->init(); //verify new table (throws runtime_errors)
				}
			}
		}

		handleFillModifiedTablesXML(xmlOut,cfgMgr);

	}
	catch(std::runtime_error& e)
	{
		__SUP_SS__ << ("Error setting field values!\n\n" + std::string(e.what())) << std::endl;
		__SUP_COUT_ERR__ << "\n" << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
	}
	catch(...)
	{
		__SUP_SS__ << ("Error setting field values!\n\n") << std::endl;
		__SUP_COUT_ERR__ << "\n" << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
	}
}

//========================================================================================================================
//handleFillGetTreeNodeFieldValuesXML
//	returns for each record, xml list of field/value pairs
//		field := relative-path
//
// if groupName == "" || groupKey is invalid
//	 then do for active groups
//
//parameters
//	configGroupName (full name with key)
//	starting node path
//	modifiedTables := CSV of table/version pairs
//	recordStr := CSV list of records for which to lookup values for fields
//	fieldList := CSV of relative-to-record-path to filter common fields
//
void ConfigurationGUISupervisor::handleFillGetTreeNodeFieldValuesXML(HttpXmlDocument& xmlOut, ConfigurationManagerRW* cfgMgr,
		const std::string& groupName, const ConfigurationGroupKey& groupKey,
		const std::string& startPath,
		const std::string& modifiedTables, const std::string& recordList,
		const std::string& fieldList)
{
	//	setup active tables based on input group and modified tables
	setupActiveTablesXML(
			xmlOut,
			cfgMgr,
			groupName, groupKey,
			modifiedTables);

	//for each field
	//	return field/value pair in xml

	try
	{
		std::vector<std::string /*relative-path*/> fieldPaths;
		//extract field list
		{
			std::istringstream f(fieldList);
			std::string fieldPath;
			while (getline(f, fieldPath, ','))
			{
				fieldPaths.push_back(
						StringMacros::decodeURIComponent(fieldPath));
			}
			__SUP_COUT__ << fieldList << std::endl;
			for(auto& field:fieldPaths)
				__SUP_COUT__ << "fieldPath " <<
				field << std::endl;
		}

		//extract record list
		{
			std::istringstream f(recordList);
			std::string recordUID;
			while (getline(f, recordUID, ',')) //for each record
			{
				recordUID = StringMacros::decodeURIComponent(recordUID);

				__SUP_COUT__ << "recordUID " <<
						recordUID << std::endl;

				DOMElement* parentEl = xmlOut.addTextElementToData("fieldValues", recordUID);

				//for each field, get value
				for(const auto& fieldPath:fieldPaths)
				{
					__SUP_COUT__ << "fieldPath " << fieldPath << std::endl;

					xmlOut.addTextElementToParent("FieldPath",
							fieldPath,
							parentEl);
					xmlOut.addTextElementToParent("FieldValue",
							cfgMgr->getNode(startPath + "/" + recordUID + "/" + fieldPath).getValueAsString(),
							parentEl);
				}
			}
		}
	}
	catch(std::runtime_error& e)
	{
		__SUP_SS__ << ("Error getting field values!\n\n" + std::string(e.what())) << std::endl;
		__SUP_COUT_ERR__ << "\n" << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
	}
	catch(...)
	{
		__SUP_SS__ << ("Error getting field values!\n\n") << std::endl;
		__SUP_COUT_ERR__ << "\n" << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
	}
}

//========================================================================================================================
//handleFillTreeNodeCommonFieldsXML
//	returns xml list of common fields among records
//		field := relative-path
//
// if groupName == "" || groupKey is invalid
//	 then do for active groups
//
//parameters
//	configGroupName (full name with key)
//	starting node path
//	depth from starting node path
//	modifiedTables := CSV of table/version pairs
//	recordList := CSV of records to search for fields
//	fieldList := CSV of relative-to-record-path to filter common fields
//		(accept or reject [use ! as first character to reject])
//		[use leading*  to ignore relative path - note that only leading and trailing wildcards work]
//
void ConfigurationGUISupervisor::handleFillTreeNodeCommonFieldsXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr,
		const std::string& groupName, const ConfigurationGroupKey& groupKey,
		const std::string& startPath, unsigned int depth,
		const std::string& modifiedTables, const std::string& recordList,
		const std::string& fieldList)
{
	//	setup active tables based on input group and modified tables
	setupActiveTablesXML(
			xmlOut,
			cfgMgr,
			groupName, groupKey,
			modifiedTables);


	try
	{
		DOMElement* parentEl = xmlOut.addTextElementToData("fields", startPath);

		if(depth == 0)
		{
			__SUP_SS__ << "Depth of search must be greater than 0." << __E__;
			__SUP_COUT__ << ss.str();
			throw std::runtime_error(ss.str()); //done if 0 depth, no fields
		}

		//do not allow traversing for common fields from root level
		//	the tree view should be used for such a purpose
		//if(startPath == "/")
		//	return;

		std::vector<ConfigurationTree::RecordField> retFieldList;


		{
			ConfigurationTree startNode = cfgMgr->getNode(startPath);
			if(startNode.isLinkNode() && startNode.isDisconnected())
			{
				__SUP_SS__ << "Start path was a disconnected link node!" << std::endl;
				__SUP_COUT_ERR__ << "\n" << ss.str();
				throw std::runtime_error(ss.str());
				return; //quietly ignore disconnected links at depth
				//note: at the root level they will be flagged for the user
			}

			std::vector<std::string /*relative-path*/> fieldAcceptList, fieldRejectList;
			if(fieldList != "")
			{
				//extract field filter list
				{
					std::istringstream f(fieldList);
					std::string fieldPath, decodedFieldPath;
					while (getline(f, fieldPath, ','))
					{
						decodedFieldPath = StringMacros::decodeURIComponent(fieldPath);

						if(decodedFieldPath[0] == '!') //reject field
							fieldRejectList.push_back(decodedFieldPath.substr(1));
						else
							fieldAcceptList.push_back(decodedFieldPath);
					}
					__SUP_COUT__ << fieldList << std::endl;
					for(auto& field:fieldAcceptList)
						__SUP_COUT__ << "fieldAcceptList " <<
							field << std::endl;
					for(auto& field:fieldRejectList)
						__SUP_COUT__ << "fieldRejectList " <<
							field << std::endl;
				}
			}

			std::vector<std::string /*relative-path*/> records;
			if(recordList == "*") //handle all records case
			{
				records.clear();
				records = startNode.getChildrenNames();
				__SUP_COUT__ << "Translating wildcard..." << __E__;
				for(auto& record:records)
					__SUP_COUT__ << "recordList " <<
					record << std::endl;
			}
			else if(recordList != "")
			{
				//extract record list
				{
					std::istringstream f(recordList);
					std::string recordStr;
					while (getline(f, recordStr, ','))
					{
						records.push_back(
								StringMacros::decodeURIComponent(recordStr));
					}
					__SUP_COUT__ << recordList << std::endl;
					for(auto& record:records)
						__SUP_COUT__ << "recordList " <<
							record << std::endl;
				}
			}

			retFieldList = startNode.getCommonFields(
					records,fieldAcceptList,fieldRejectList,depth);
		}

		DOMElement* parentTypeEl;
		for(const auto& fieldInfo:retFieldList)
		{
			xmlOut.addTextElementToParent("FieldTableName",
					fieldInfo.tableName_,
					parentEl);
			xmlOut.addTextElementToParent("FieldColumnName",
					fieldInfo.columnName_,
					parentEl);
			xmlOut.addTextElementToParent("FieldRelativePath",
					fieldInfo.relativePath_,
					parentEl);
			xmlOut.addTextElementToParent("FieldColumnType",
					fieldInfo.columnInfo_->getType(),
					parentEl);
			xmlOut.addTextElementToParent("FieldColumnDataType",
					fieldInfo.columnInfo_->getDataType(),
					parentEl);
			xmlOut.addTextElementToParent("FieldColumnDefaultValue",
					fieldInfo.columnInfo_->getDefaultValue(),
					parentEl);

			parentTypeEl = xmlOut.addTextElementToParent("FieldColumnDataChoices",
					"",	parentEl);

			//if there are associated data choices, send info
			auto dataChoices = fieldInfo.columnInfo_->getDataChoices();
			xmlOut.addTextElementToParent("FieldColumnDataChoice", //add default to list to mimic tree handling
					fieldInfo.columnInfo_->getDefaultValue(),
					parentTypeEl);
			for(const auto& dataChoice : dataChoices)
				xmlOut.addTextElementToParent("FieldColumnDataChoice",
						dataChoice,
						parentTypeEl);
		}
	}
	catch(std::runtime_error& e)
	{
		__SUP_SS__ << ("Error getting common fields!\n\n" + std::string(e.what())) << std::endl;
		__SUP_COUT_ERR__ << "\n" << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
	}
	catch(...)
	{
		__SUP_SS__ << ("Error getting common fields!\n\n") << std::endl;
		__SUP_COUT_ERR__ << "\n" << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
	}

}

//========================================================================================================================
//handleFillUniqueFieldValuesForRecordsXML
//	returns xml list of unique values for each fields among records
//		field := relative-path
//
//	return xml
//		<xml>
//			<field val=relative-path>
//				<unique_val val=uval0>
//				<unique_val val=uval1>
//				.. next unique value
//			</field>
//			... next field
//		</xml>
//
// if groupName == "" || groupKey is invalid
//	 then do for active groups
//
//parameters
//	configGroupName (full name with key)
//	starting node path
//	modifiedTables := CSV of table/version pairs
//	recordList := CSV of records to search for unique values
//	fieldList := CSV of fields relative-to-record-path for which to get list of unique values
//
void ConfigurationGUISupervisor::handleFillUniqueFieldValuesForRecordsXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr,
		const std::string& groupName, const ConfigurationGroupKey& groupKey,
		const std::string& startPath,
		const std::string& modifiedTables, const std::string& recordList,
		const std::string& fieldList)
{
	//	setup active tables based on input group and modified tables
	setupActiveTablesXML(
			xmlOut,
			cfgMgr,
			groupName, groupKey,
			modifiedTables);


	try
	{
		//do not allow traversing for common fields from root level
		//	the tree view should be used for such a purpose
		if(startPath == "/")
			return;

		std::vector<std::string /*relative-path*/> fieldsToGet;
		if(fieldList != "")
		{
			//extract field filter list
			{
				std::istringstream f(fieldList);
				std::string fieldPath;
				while (getline(f, fieldPath, ','))
				{
					fieldsToGet.push_back(
							StringMacros::decodeURIComponent(fieldPath));
				}
				__SUP_COUT__ << fieldList << std::endl;
				for(auto& field:fieldsToGet)
					__SUP_COUT__ << "fieldsToGet " <<
						field << std::endl;
			}
		}



		ConfigurationTree startNode = cfgMgr->getNode(startPath);
		if(startNode.isLinkNode() && startNode.isDisconnected())
		{
			__SUP_SS__ << "Start path was a disconnected link node!" << std::endl;
			__SUP_COUT_ERR__ << "\n" << ss.str();
			throw std::runtime_error(ss.str());
		}

		std::vector<std::string /*relative-path*/> records;
		if(recordList == "*") //handle all records case
		{
			records.clear();
			records = startNode.getChildrenNames();
			__SUP_COUT__ << "Translating wildcard..." << __E__;
			for(auto& record:records)
				__SUP_COUT__ << "recordList " <<
				record << std::endl;
		}
		else if(recordList != "")
		{
			//extract record list
			{
				std::istringstream f(recordList);
				std::string recordStr;
				while (getline(f, recordStr, ','))
				{
					records.push_back(
							StringMacros::decodeURIComponent(recordStr));
				}
				__SUP_COUT__ << recordList << std::endl;
				for(auto& record:records)
					__SUP_COUT__ << "recordList " <<
					record << std::endl;
			}
		}



		//loop through each field and get unique values among records
		for(auto& field:fieldsToGet)
		{
			__SUP_COUT__ << "fieldsToGet " <<
				field << std::endl;

			DOMElement* parentEl = xmlOut.addTextElementToData("field", field);


			//use set to force sorted unique values
			std::set<std::string /*unique-values*/> uniqueValues;

			uniqueValues = cfgMgr->getNode(startPath).getUniqueValuesForField(
					records,field);

			for(auto& uniqueValue:uniqueValues)
			{
				__SUP_COUT__ << "uniqueValue " <<
						uniqueValue << std::endl;

				xmlOut.addTextElementToParent("uniqueValue",
						uniqueValue,
						parentEl);
			}
		}
	}
	catch(std::runtime_error& e)
	{
		__SUP_SS__ << ("Error getting common fields!\n\n" + std::string(e.what())) << std::endl;
		__SUP_COUT_ERR__ << "\n" << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
	}
	catch(...)
	{
		__SUP_SS__ << ("Error getting common fields!\n\n") << std::endl;
		__SUP_COUT_ERR__ << "\n" << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
	}

}

//========================================================================================================================
//handleFillTreeViewXML
//	returns xml tree from path for given depth
//
// if groupName == "" || groupKey is invalid
//	 then return tree for active groups
//
//parameters
//	configGroupName (full name with key)
//	starting node path
//	depth from starting node path
//	modifiedTables := CSV of table/version pairs
//	filterList := relative-to-record-path=value(,value,...);path=value... filtering
//		records with relative path not meeting all filter criteria
//		- can accept multiple values per field (values separated by commas) (i.e. OR)
//		- fields/value pairs separated by ; for AND
//			- Note: limitation here is there is no OR among fields/value pairs (in future, could separate field/value pairs by : for OR)
//		e.g. "LinkToFETypeConfiguration=NIMPlus,TemplateUDP;FEInterfacePluginName=NIMPlusPlugin"
//
void ConfigurationGUISupervisor::handleFillTreeViewXML(HttpXmlDocument& xmlOut, ConfigurationManagerRW* cfgMgr,
		const std::string& groupName, const ConfigurationGroupKey& groupKey,
		const std::string& startPath, unsigned int depth, bool hideStatusFalse,
		const std::string& modifiedTables, const std::string& filterList)
{
	//return xml
	//	<groupName="groupName"/>
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



	//Think about using this in the future to clean up the code
	//	But may not work well since there is some special functionality used below
	//	like getting group comments, and not reloading everything except for at root level /
	// ....
	//	//	setup active tables based on input group and modified tables


	bool usingActiveGroups = (groupName == "" || groupKey.isInvalid());
	std::map<std::string /*name*/, ConfigurationVersion /*version*/> memberMap;

	std::string accumulatedErrors;
	setupActiveTablesXML(
			xmlOut,
			cfgMgr,
			groupName, groupKey,
			modifiedTables,
			(startPath == "/"), //refreshAll, if at root node, reload all tables so that partially loaded tables are not allowed
			(startPath == "/"), //get group info
			&memberMap,			//get group member map
			true,				//output active tables (default)
			&accumulatedErrors	//accumulate errors
			);
	if(accumulatedErrors != "")
		xmlOut.addTextElementToData("Warning",
				accumulatedErrors);

	__SUP_COUT__ << "Active tables are setup. Warning string: '" << accumulatedErrors << "'" << std::endl;

	try
	{
		DOMElement* parentEl = xmlOut.addTextElementToData("tree", startPath);

		if(depth == 0) return; //already returned root node in itself

		std::vector<std::pair<std::string,ConfigurationTree> > rootMap;

		if(startPath == "/")
		{
			//then consider the configurationManager the root node

			if(usingActiveGroups)
				rootMap = cfgMgr->getChildren();
			else
			{
				std::string accumulateTreeErrs;
				rootMap = cfgMgr->getChildren(&memberMap,&accumulateTreeErrs);
				__SUP_COUT__ << "accumulateTreeErrs = " << accumulateTreeErrs << std::endl;
				if(accumulateTreeErrs != "")
					xmlOut.addTextElementToData("TreeErrors",
							accumulateTreeErrs);
			}
		}
		else
		{
			ConfigurationTree startNode = cfgMgr->getNode(startPath,
					true /*doNotThrowOnBrokenUIDLinks*/);
			if(startNode.isLinkNode() && startNode.isDisconnected())
			{
				xmlOut.addTextElementToData("DisconnectedStartNode","1");
				return; //quietly ignore disconnected links at depth
				//note: at the root level they will be flagged for the user
			}

			std::map<std::string /*relative-path*/, std::string /*value*/> filterMap;
			StringMacros::getMapFromString(filterList,filterMap,
					std::set<char>({';'})/*pair delimiters*/,
					std::set<char>({'='})/*name/value delimiters*/);
			//			if(filterList != "")
			//			{
			//				//extract filter list
			//				{
			//					std::istringstream f(filterList);
			//					std::string filterPath,filterValue;
			//					while (getline(f, filterPath, '='))
			//					{
			//						getline(f, filterValue, ';');
			//						filterMap.insert(
			//								std::pair<std::string,std::string>(
			//										filterPath,
			//										filterValue));
			//					}
			//					__SUP_COUT__ << filterList << std::endl;
			//					for(auto& pair:filterMap)
			//						__SUP_COUT__ << "filterMap " <<
			//						pair.first << "=" <<
			//						pair.second << std::endl;
			//				}
			//			}
			__COUTV__(StringMacros::mapToString(filterMap));

			rootMap = cfgMgr->getNode(startPath).getChildren(filterMap);
		}

		for(auto& treePair:rootMap)
			recursiveTreeToXML(treePair.second,depth-1,xmlOut,parentEl,hideStatusFalse);
	}
	catch(std::runtime_error& e)
	{
		__SUP_SS__ << "Error detected generating XML tree!\n\n " << e.what() << std::endl;
		__SUP_COUT_ERR__ << "\n" << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
	}
	catch(...)
	{
		__SUP_SS__ << "Error detected generating XML tree!" << std::endl;
		__SUP_COUT_ERR__ << "\n" << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
	}
}

//==============================================================================
//recursiveToXml
//	output tree to XML from this node for desired depth
//	depth of 0 means output only this node's value
//	depth of 1 means include this node's children's values, etc..
//	depth of -1(unsigned int) effectively means output full tree
void ConfigurationGUISupervisor::recursiveTreeToXML(const ConfigurationTree& t, unsigned int depth, HttpXmlDocument& xmlOut,
		DOMElement* parentEl, bool hideStatusFalse)
{
	//__COUT__ << t.getValueAsString() << std::endl;

	if(t.isValueNode())
	{
		parentEl = xmlOut.addTextElementToParent("node", t.getValueName(), parentEl);
		xmlOut.addTextElementToParent("value", t.getValueAsString(), parentEl);
		parentEl = xmlOut.addTextElementToParent("valueType", t.getValueType(), parentEl);

		//fixed choice and bitmap both use fixed choices strings
		//	so output them to xml
		if(t.getValueType() == ViewColumnInfo::TYPE_FIXED_CHOICE_DATA ||
				t.getValueType() == ViewColumnInfo::TYPE_BITMAP_DATA)
		{
			__COUT__ << t.getValueType() << std::endl;

			std::vector<std::string> choices = t.getFixedChoices();
			for(const auto& choice:choices)
				xmlOut.addTextElementToParent("fixedChoice", choice, parentEl);
		}
	}
	else
	{
		if(t.isLinkNode())
		{
			//__COUT__ << t.getValueName() << std::endl;

			//Note: The order of xml fields is required by JavaScript, so do NOT change order.
			parentEl = xmlOut.addTextElementToParent("node", t.getValueName(), parentEl);

			if(t.isDisconnected())
			{
				__COUT__ << t.getValueName() << std::endl;

				//xmlOut.addTextElementToParent("value", t.getValueAsString(), parentEl);
				//xmlOut.addTextElementToParent("DisconnectedLink", t.getValueAsString(), parentEl);

				xmlOut.addTextElementToParent("valueType", t.getValueType(), parentEl);

				//add extra fields for disconnected link
				xmlOut.addTextElementToParent(
						(t.isGroupLinkNode()?"Group":"U") +	std::string("ID"),
						t.getDisconnectedLinkID(),
						parentEl);
				xmlOut.addTextElementToParent("LinkConfigurationName",
						t.getDisconnectedTableName(),
						parentEl);
				xmlOut.addTextElementToParent("LinkIndex",
						t.getChildLinkIndex(),
						parentEl);


				//add fixed choices (in case link has them)
				DOMElement* choicesParentEl = xmlOut.addTextElementToParent("fixedChoices", "",
						parentEl);
				//try
				//{

					std::vector<std::string> choices = t.getFixedChoices();
					__COUT__ << "choices.size() " << choices.size() << std::endl;

					for(const auto& choice:choices)
						xmlOut.addTextElementToParent("fixedChoice", choice, choicesParentEl);
				//}
				//catch(...)
				//{
				//	__COUT__ << "Ignoring unknown fixed choice error"
				//} //ignore no fixed choices for disconnected

				return;
			}

			//handle connected links

			xmlOut.addTextElementToParent(
					(t.isGroupLinkNode()?"Group":"U") +	std::string("ID"),
					t.getValueAsString(), parentEl);

			xmlOut.addTextElementToParent("LinkConfigurationName", t.getConfigurationName(),
					parentEl);

			xmlOut.addTextElementToParent("LinkIndex", t.getChildLinkIndex(),
					parentEl);

			//add fixed choices (in case link has them)
			{
				DOMElement* choicesParentEl = xmlOut.addTextElementToParent("fixedChoices", "", parentEl);
				std::vector<std::string> choices = t.getFixedChoices();

				//__COUT__ << "choices.size() " << choices.size() << std::endl;

				for(const auto& choice:choices)
					xmlOut.addTextElementToParent("fixedChoice", choice, choicesParentEl);
			}
		}
		else //uid node
		{
			bool returnNode = true; //default to shown

			if(hideStatusFalse) //only show if status evaluates to true
			{
				try //try to get Status child as boolean..
				{	//if Status bool doesn't exist exception will be thrown
					t.getNode(ViewColumnInfo::COL_NAME_STATUS).getValue(returnNode);
				} catch(...) {}
			}

			if(returnNode)
				parentEl = xmlOut.addTextElementToParent("node", t.getValueAsString(), parentEl);
			else
				return; //done.. no further depth needed for node that is not shown
		}

		//if depth>=1 toXml all children
		//child.toXml(depth-1)
		if(depth >= 1)
		{
			auto C = t.getChildren();
			for(auto& c:C)
				recursiveTreeToXML(c.second,depth-1,xmlOut,parentEl,hideStatusFalse);
		}
	}
}



//========================================================================================================================
//handleGetLinkToChoicesXML
//	return all possible choices for link
//		linkIdType = "UID" or "GroupID"
//
//	as xml:
//	<linkToChoice = xxx>
void ConfigurationGUISupervisor::handleGetLinkToChoicesXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr, const std::string& linkToTableName,
		const ConfigurationVersion& linkToTableVersion,
		const std::string& linkIdType, const std::string& linkIndex,
		const std::string& linkInitId)
try
{
	//get table
	//	if uid link
	//		return all uids
	//	if groupid link
	//		find target column
	//		create the set of values (unique values only)
	//			note: insert group unions individually (i.e. groups | separated)


	//get table and activate target version
	//	rename to re-use code template
	const std::string& configName = linkToTableName;
	const ConfigurationVersion& version = linkToTableVersion;
	ConfigurationBase* config = cfgMgr->getConfigurationByName(configName);
	try
	{
		config->setActiveView(version);
	}
	catch(...)
	{
		__SUP_COUT__ << "Failed to find stored version, so attempting to load version: " <<
				version << std::endl;
		cfgMgr->getVersionedConfigurationByName(
				configName, version);
	}

	if(version != config->getViewVersion())
	{
		__SUP_SS__ << "Target table version (" << version <<
				") is not the currently active version (" << config->getViewVersion()
				<< ". Try refreshing the tree." << std::endl;
		__SUP_COUT_WARN__ << ss.str();
		throw std::runtime_error(ss.str());
	}

	__SUP_COUT__ << "Active version is " << config->getViewVersion() << std::endl;

	if(linkIdType == "UID")
	{
		//give all UIDs
		unsigned int col = config->getView().getColUID();
		for(unsigned int row = 0; row < config->getView().getNumberOfRows(); ++row)
			xmlOut.addTextElementToData("linkToChoice",
					config->getView().getDataView()[row][col]);
	}
	else if(linkIdType == "GroupID")
	{
		//		find target column
		//		create the set of values (unique values only)
		//			note: insert group unions individually (i.e. groups | separated)

		std::set<std::string> setOfGroupIDs =
				config->getView().getSetOfGroupIDs(linkIndex);

		//build list of groupids
		//	always include initial link group id in choices
		//	(even if not in set of group ids)
		bool foundInitId = false;
		for(const auto& groupID : setOfGroupIDs)
		{
			if(!foundInitId &&
					linkInitId == groupID)
				foundInitId = true; //mark init id found

			xmlOut.addTextElementToData("linkToChoice",
					groupID);
		}
		//if init id was not found, add to list
		if(!foundInitId)
			xmlOut.addTextElementToData("linkToChoice",
					linkInitId);

		//give all UIDs
		unsigned int col = config->getView().getColUID();
		for(unsigned int row = 0; row < config->getView().getNumberOfRows(); ++row)
		{
			xmlOut.addTextElementToData("groupChoice",
					config->getView().getDataView()[row][col]);
			if(config->getView().isEntryInGroup(row,linkIndex,linkInitId))
				xmlOut.addTextElementToData("groupMember",
						config->getView().getDataView()[row][col]);
		}
	}
	else
	{
		__SUP_SS__ << "Unrecognized linkIdType '" << linkIdType
				<< ".'" << std::endl;
		throw std::runtime_error(ss.str());
	}


}
catch(std::runtime_error& e)
{
	__SUP_SS__ << "Error detected saving tree node!\n\n " << e.what() << std::endl;
	__SUP_COUT_ERR__ << "\n" << ss.str() << std::endl;
	xmlOut.addTextElementToData("Error", ss.str());
}
catch(...)
{
	__SUP_SS__ << "Error detected saving tree node!\n\n "<< std::endl;
	__SUP_COUT_ERR__ << "\n" << ss.str() << std::endl;
	xmlOut.addTextElementToData("Error", ss.str());
}


//========================================================================================================================
//handleMergeGroupsXML
void ConfigurationGUISupervisor::handleMergeGroupsXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr,
		const std::string& groupANameContext, const ConfigurationGroupKey& groupAKeyContext,
		const std::string& groupBNameContext, const ConfigurationGroupKey& groupBKeyContext,
		const std::string& groupANameConfig, const ConfigurationGroupKey& groupAKeyConfig,
		const std::string& groupBNameConfig, const ConfigurationGroupKey& groupBKeyConfig,
		const std::string& author, const std::string& mergeApproach)
try
{
	__SUP_COUT__ << "Merging context group pair " <<
			groupANameContext << " (" << groupAKeyContext << ") & " <<
			groupBNameContext << " (" << groupBKeyContext << ") and config group pair " <<
			groupANameConfig << " (" << groupAKeyConfig << ") & " <<
			groupBNameConfig << " (" << groupBKeyConfig << ") with approach '" <<
			mergeApproach << __E__;

	// Merges group A and group B
	//	with consideration for UID conflicts
	// Result is a new key of group A's name
	//
	//There 3 modes:
	//	Rename		-- All records from both groups are maintained, but conflicts from B are renamed.
	//					Must maintain a map of UIDs that are remapped to new name for groupB,
	//					because linkUID fields must be preserved.
	//	Replace		-- Any UID conflicts for a record are replaced by the record from group B.
	//	Skip		-- Any UID conflicts for a record are skipped so that group A record remains

	//check valid mode
	if(!(mergeApproach == "Rename" || mergeApproach == "Replace" || mergeApproach == "Skip"))
	{
		__SS__ << "Error! Invalid merge approach '" <<	mergeApproach << ".'" << __E__;
		__SS_THROW__;
	}

	std::map<std::string /*name*/, ConfigurationVersion /*version*/> memberMapAContext, memberMapBContext,
		memberMapAConfig, memberMapBConfig;


	//check if skipping group pairs
	bool skippingContextPair = false;
	bool skippingConfigPair = false;
	if(groupANameContext.size() == 0 || groupANameContext[0] == ' ' ||
			groupBNameContext.size() == 0 || groupBNameContext[0] == ' ')
	{
		skippingContextPair = true;
		__SUP_COUTV__(skippingContextPair);
	}
	if(groupANameConfig.size() == 0 || groupANameConfig[0] == ' ' ||
			groupBNameConfig.size() == 0 || groupBNameConfig[0] == ' ')
	{
		skippingConfigPair = true;
		__SUP_COUTV__(skippingConfigPair);
	}

	//get context group member maps
	if(!skippingContextPair)
	{
		cfgMgr->loadConfigurationGroup(groupANameContext,groupAKeyContext,
				false /*doActivate*/,&memberMapAContext,0 /*progressBar*/,0 /*accumulateErrors*/,
				0 /*groupComment*/, 0 /*groupAuthor*/, 0 /*groupCreationTime*/,
				false /*doNotLoadMember*/, 0 /*groupTypeString*/
				);
		__SUP_COUTV__(StringMacros::mapToString(memberMapAContext));

		cfgMgr->loadConfigurationGroup(groupBNameContext,groupBKeyContext,
				false /*doActivate*/,&memberMapBContext,0 /*progressBar*/,0 /*accumulateErrors*/,
				0 /*groupComment*/, 0 /*groupAuthor*/, 0 /*groupCreationTime*/,
				false /*doNotLoadMember*/, 0 /*groupTypeString*/
				);

		__SUP_COUTV__(StringMacros::mapToString(memberMapBContext));
	}

	//get config group member maps
	if(!skippingConfigPair)
	{
		cfgMgr->loadConfigurationGroup(groupANameConfig,groupAKeyConfig,
				false /*doActivate*/,&memberMapAConfig,0 /*progressBar*/,0 /*accumulateErrors*/,
				0 /*groupComment*/, 0 /*groupAuthor*/, 0 /*groupCreationTime*/,
				false /*doNotLoadMember*/, 0 /*groupTypeString*/
				);
		__SUP_COUTV__(StringMacros::mapToString(memberMapAConfig));

		cfgMgr->loadConfigurationGroup(groupBNameConfig,groupBKeyConfig,
				false /*doActivate*/,&memberMapBConfig,0 /*progressBar*/,0 /*accumulateErrors*/,
				0 /*groupComment*/, 0 /*groupAuthor*/, 0 /*groupCreationTime*/,
				false /*doNotLoadMember*/, 0 /*groupTypeString*/
				);

		__SUP_COUTV__(StringMacros::mapToString(memberMapBConfig));
	}

	//for each member of B
	//	if not found in A member map, add it
	//	if found in both member maps, and versions are different, load both tables and merge

	std::map< std::pair<std::string /*original table*/,std::string /*original uidB*/>,
		std::string /*converted uidB*/> uidConversionMap;
	std::map< std::pair<std::string /*original table*/,std::pair<std::string /*group linkid*/,std::string /*original gidB*/>>,
		std::string /*converted gidB*/> groupidConversionMap;

	//first loop create record conversion map, second loop implement merge (using conversion map if Rename)
	for(unsigned int i=0;i<2;++i)
	{
		if(i==0 && mergeApproach != "Rename") continue; //only need to construct uidConversionMap for rename approach

		//loop for context and config pair types
		for(unsigned int j=0;j<2;++j)
		{
			if(j == 0 && skippingContextPair) //context
			{
				__COUT__ << "Skipping context pair..." << __E__;
				continue;
			}
			else if(j == 1 && skippingConfigPair)
			{
				__COUT__ << "Skipping config pair..." << __E__;
				continue;
			}

			std::map<std::string /*name*/, ConfigurationVersion /*version*/>& memberMapAref =
					j == 0?memberMapAContext:memberMapAConfig;

			std::map<std::string /*name*/, ConfigurationVersion /*version*/>& memberMapBref =
					j == 0?memberMapBContext:memberMapBConfig;

			if(j == 0) //context
				__COUT__ << "Context pair..." << __E__;
			else
				__COUT__ << "Config pair..." << __E__;

			__COUT__ << "Starting member map B scan." << __E__;
			for(const auto bkey : memberMapBref)
			{
				__SUP_COUTV__(bkey.first);

				if(memberMapAref.find(bkey.first) == memberMapAref.end())
				{
					//not found, so add to A member map
					memberMapAref[bkey.first] = bkey.second;
				}
				else if(memberMapAref[bkey.first] != bkey.second)
				{
					//found table version confict
					__SUP_COUTV__(memberMapAref[bkey.first]);
					__SUP_COUTV__(bkey.second);

					//load both tables, and merge
					ConfigurationBase* config = cfgMgr->getConfigurationByName(bkey.first);

					__SUP_COUT__ << "Got configuration." << __E__;

					ConfigurationVersion newVersion = config->mergeViews(
							cfgMgr->getVersionedConfigurationByName(bkey.first,
									memberMapAref[bkey.first])->getView(),
									cfgMgr->getVersionedConfigurationByName(bkey.first,bkey.second)->getView(),
									ConfigurationVersion() /* destinationVersion*/,
									author,
									mergeApproach /*Rename,Replace,Skip*/,
									uidConversionMap, groupidConversionMap,
									i==0  /* fillRecordConversionMaps */,
									i==1  /* applyRecordConversionMaps */,
									config->getConfigurationName() ==
											ConfigurationManager::XDAQ_APPLICATION_CONFIG_NAME /* generateUniqueDataColumns */
											); //dont make destination version the first time

					if(i==1)
					{
						__SUP_COUTV__(newVersion);

						try
						{
							//save all temporary tables to persistent tables
							//finish off the version creation
							newVersion = saveModifiedVersionXML(xmlOut,cfgMgr,
									bkey.first,
									ConfigurationVersion() /*original source version*/,
									false /* makeTemporary */,
									config,
									newVersion /*temporary modified version*/,
									false /*ignore duplicates*/,
									true /*look for equivalent*/);
						}
						catch(std::runtime_error& e)
						{
							__SUP_SS__ << "There was an error saving the '" <<
									config->getConfigurationName() << "' merge result to a persistent table version. " <<
									"Perhaps you can modify this table in one of the groups to resolve this issue, and then re-merge." <<
									__E__ << e.what();
							__SS_THROW__;
						}

						__SUP_COUTV__(newVersion);


						memberMapAref[bkey.first] = newVersion;
					}
				} //end member version conflict handling
			} //end B member map loop
		} //end context and config loop
	} //end top level conversion map or not loop

	//Now save groups

	if(!skippingContextPair)
	{
		__SUP_COUT__ << "New context member map complete." << __E__;
		__SUP_COUTV__(StringMacros::mapToString(memberMapAContext));

		//save the new configuration group
		ConfigurationGroupKey newKeyContext =
				cfgMgr->saveNewConfigurationGroup(groupANameContext,memberMapAContext,
						"Merger of group " + groupANameContext + " (" + groupAKeyContext.toString() + ") and " +
						groupBNameContext + " (" + groupBKeyContext.toString() + ").");

		//return new resulting group
		xmlOut.addTextElementToData("ContextGroupName", groupANameContext);
		xmlOut.addTextElementToData("ContextGroupKey", newKeyContext.toString());
	}
	if(!skippingConfigPair)
	{
		__SUP_COUT__ << "New config member map complete." << __E__;
		__SUP_COUTV__(StringMacros::mapToString(memberMapAConfig));
		//save the new configuration group
		ConfigurationGroupKey newKeyConfig =
				cfgMgr->saveNewConfigurationGroup(groupANameConfig,memberMapAConfig,
						"Merger of group " + groupANameConfig + " (" + groupAKeyConfig.toString() + ") and " +
						groupBNameConfig + " (" + groupBKeyConfig.toString() + ").");

		//return new resulting group
		xmlOut.addTextElementToData("ConfigGroupName", groupANameConfig);
		xmlOut.addTextElementToData("ConfigGroupKey", newKeyConfig.toString());
	}



} //end handleMergeGroupsXML
catch(std::runtime_error& e)
{
	__SUP_SS__ << "Error merging context group pair " <<
			groupANameContext << " (" << groupAKeyContext << ") & " <<
			groupBNameContext << " (" << groupBKeyContext << ") and config group pair " <<
			groupANameConfig << " (" << groupAKeyConfig << ") & " <<
			groupBNameConfig << " (" << groupBKeyConfig << ") with approach '" <<
			mergeApproach << "': \n\n" <<
			e.what() << std::endl;
	__SUP_COUT_ERR__ << "\n" << ss.str() << std::endl;
	xmlOut.addTextElementToData("Error", ss.str());
}
catch(...)
{
	__SUP_SS__ << "Unknown error merging context group pair " <<
			groupANameContext << " (" << groupAKeyContext << ") & " <<
			groupBNameContext << " (" << groupBKeyContext << ") and config group pair " <<
			groupANameConfig << " (" << groupAKeyConfig << ") & " <<
			groupBNameConfig << " (" << groupBKeyConfig << ") with approach '" <<
			mergeApproach << ".' \n\n";
	__SUP_COUT_ERR__ << "\n" << ss.str() << std::endl;
	xmlOut.addTextElementToData("Error", ss.str());
}

//========================================================================================================================
//handleSavePlanCommandSequenceXML
void ConfigurationGUISupervisor::handleSavePlanCommandSequenceXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr,
		const std::string& groupName, const ConfigurationGroupKey& groupKey,
		const std::string& modifiedTables, const std::string& author,
		const std::string& planName,
		const std::string& commandString)
try
{
	__MOUT__ << "handleSavePlanCommandSequenceXML" << std::endl;

	//	setup active tables based on input group and modified tables
	setupActiveTablesXML(
			xmlOut,
			cfgMgr,
			groupName, groupKey,
			modifiedTables,
			true /* refresh all */, false /* getGroupInfo */,
			0 /* returnMemberMap */, false /* outputActiveTables */);



	TableEditStruct planTable(IterateConfiguration::PLAN_TABLE,cfgMgr);	// Table ready for editing!
	TableEditStruct targetTable(IterateConfiguration::TARGET_TABLE,cfgMgr);	// Table ready for editing!

	//create table-edit struct for each iterate command type
	std::map<std::string, TableEditStruct> commandTypeToCommandTableMap;
	for(const auto& commandPair : IterateConfiguration::commandToTableMap_)
		if(commandPair.second != "") //skip tables with no parameters
			commandTypeToCommandTableMap.emplace(std::pair<std::string,TableEditStruct>(
				commandPair.first,
				TableEditStruct(commandPair.second,cfgMgr)));


	//try to catch any errors while editing..
	//	if errors delete temporary plan view (if created here)
	try
	{

		//	Steps:
		//		Reset plan commands
		// 			Remove all commands in group "<plan>-Plan"
		//				Delete linked command parameters row (in separate table)
		//				If no group remaining, then delete row.
		//
		//		Save plan commands (if modified)
		//			Create rows and add them to group "<plan>-Plan"
		//				create row for command paramaters and add to proper table

		std::string groupName = planName + "-Plan";
		__SUP_COUT__ << "Handling commands for group " << groupName << std::endl;

		unsigned int groupIdCol = planTable.cfgView_->findCol(IterateConfiguration::planTableCols_.GroupID_);
		unsigned int cmdTypeCol = planTable.cfgView_->findCol(IterateConfiguration::planTableCols_.CommandType_);


		unsigned int targetGroupIdCol 	= targetTable.cfgView_->findCol(IterateConfiguration::targetCols_.GroupID_);
		unsigned int targetTableCol 	= targetTable.cfgView_->findCol(IterateConfiguration::targetCols_.TargetLink_);
		unsigned int targetUIDCol 		= targetTable.cfgView_->findCol(IterateConfiguration::targetCols_.TargetLinkUID_);

		std::string groupLinkIndex = planTable.cfgView_->getColumnInfo(groupIdCol).getChildLinkIndex();
		__SUP_COUT__ << "groupLinkIndex: " << groupLinkIndex << std::endl;

		std::pair<unsigned int /*link col*/, unsigned int /*link id col*/> commandUidLink;
		{
			bool isGroup; //local because we know is uid link
			planTable.cfgView_->getChildLink(
					planTable.cfgView_->findCol(
							IterateConfiguration::planTableCols_.CommandLink_),
					isGroup,commandUidLink);
		}

		unsigned int cmdRow, cmdCol;
		std::string targetGroupName;

		//Reset existing plan commands
		{
			std::string targetUID, cmdType;

			for(unsigned int row=0;row < planTable.cfgView_->getNumberOfRows(); ++row)
			{
				targetUID = planTable.cfgView_->getDataView()[row][planTable.cfgView_->getColUID()];
				__SUP_COUT__ << "targetUID: " << targetUID << std::endl;

				//remove command from plan group.. if no more groups, delete
				if(planTable.cfgView_->isEntryInGroup(row,
						groupLinkIndex,groupName))
				{
					__SUP_COUT__ << "Removing." << std::endl;


					//delete linked command
					//	find linked UID in table (mapped by type)
					cmdType = planTable.cfgView_->getDataView()[row][cmdTypeCol];
					if(commandTypeToCommandTableMap.find(cmdType) !=
							commandTypeToCommandTableMap.end()) //skip if invalid command type
					{
						cmdRow = commandTypeToCommandTableMap[cmdType].cfgView_->findRow(
								commandTypeToCommandTableMap[cmdType].cfgView_->getColUID(),
								planTable.cfgView_->getDataView()[row][commandUidLink.second]);

						//before deleting row...
						//look for target group
						//	remove all targets in group
						try
						{
							cmdCol = commandTypeToCommandTableMap[cmdType].cfgView_->findCol(
									IterateConfiguration::commandTargetCols_.TargetsLinkGroupID_);
							targetGroupName =
									commandTypeToCommandTableMap[cmdType].cfgView_->getDataView()
									[cmdRow][cmdCol];


							for(unsigned int trow=0;
									trow < targetTable.cfgView_->getNumberOfRows();
									++trow)
							{
								//remove command from target group..
								if(targetTable.cfgView_->isEntryInGroup(
										trow,
										commandTypeToCommandTableMap[cmdType].cfgView_->getColumnInfo(cmdCol).getChildLinkIndex(),
										targetGroupName))
								{
									__SUP_COUT__ << "Removing target." << std::endl;
									//remove command entry in plan table
									if(targetTable.cfgView_->removeRowFromGroup(trow,
											targetGroupIdCol,
											targetGroupName,true /*deleteRowIfNoGroup*/))
										--trow; //since row was deleted, go back!
								}
							}
						}
						catch(...)
						{
							__SUP_COUT__ << "No targets." << std::endl;
						}

						//now no more targets, delete row

						commandTypeToCommandTableMap[cmdType].cfgView_->deleteRow(cmdRow);

						commandTypeToCommandTableMap[cmdType].modified_ = true;
					}

					//remove command entry in plan table
					if(planTable.cfgView_->removeRowFromGroup(row,groupIdCol,
							groupName,true /*deleteRowIfNoGroup*/))
						--row; //since row was deleted, go back!
				}
			}
		}

		//Done resetting existing plan
		//Now save new commands


		std::vector<IterateConfiguration::Command> commands;

		//extract command sequence and add to table
		//	into vector with type, and params
		{
			std::istringstream f(commandString);
			std::string commandSubString, paramSubString, paramValue;
			int i;
			while (getline(f, commandSubString, ';'))
			{
				//__SUP_COUT__ << "commandSubString " << commandSubString << std::endl;
				std::istringstream g(commandSubString);

				i = 0;
				while (getline(g, paramSubString, ','))
				{
					//__SUP_COUT__ << "paramSubString " << paramSubString << std::endl;
					if(i == 0) //type
					{
						if(paramSubString != "type")
						{
							__SUP_SS__ << "Invalid command sequence" << std::endl;
							throw std::runtime_error(ss.str());
						}
						//create command object
						commands.push_back(IterateConfiguration::Command());

						getline(g, paramValue, ','); ++i;
						//__SUP_COUT__ << "paramValue " << paramValue << std::endl;
						commands.back().type_ = paramValue;
					}
					else // params
					{
						getline(g, paramValue, ','); ++i;
						//__SUP_COUT__ << "paramValue " << paramValue << std::endl;

						commands.back().params_.emplace(
								std::pair<
								std::string /*param name*/,
								std::string /*param value*/> (
										paramSubString,
										StringMacros::decodeURIComponent(paramValue)
								));
					}

					++i;
				}
			}

		} //end extract command sequence

		__SUP_COUT__ << "commands size " << commands.size() << std::endl;

		//at this point, have extracted commands

		//now save commands to plan group
		//	group should be "<plan>-Plan"


		unsigned int row, tgtRow;
		unsigned int targetIndex;
		std::string targetStr, cmdUID;

		for(auto& command:commands)
		{

			__SUP_COUT__ << "command " <<
					command.type_ << std::endl;
			__SUP_COUT__ << "table " <<
					IterateConfiguration::commandToTableMap_.at(command.type_) << std::endl;

			//create command entry at plan level
			row = planTable.cfgView_->addRow(author,"planCommand");
			planTable.cfgView_->addRowToGroup(row,groupIdCol,groupName);

			//set command type
			planTable.cfgView_->setURIEncodedValue(
					command.type_,
					row,
					cmdTypeCol);

			//set command status true
			planTable.cfgView_->setValueAsString(
					"1",
					row,
					planTable.cfgView_->getColStatus());

			//create command specifics
			if(commandTypeToCommandTableMap.find(command.type_) !=
					commandTypeToCommandTableMap.end()) //if table exists in map! (some commands may have no parameters)
			{
				__SUP_COUT__ << "table " << commandTypeToCommandTableMap[command.type_].configName_ << std::endl;

				//at this point have config, tempVersion, and createdFlag

				//create command parameter entry at command level
				cmdRow = commandTypeToCommandTableMap[command.type_].cfgView_->addRow(
						author,true /*incrementUniqueData*/,command.type_ + "_COMMAND_");


				//parameters are linked
				//now set value of all parameters
				//	find parameter column, and set value
				//	if special target parameter, extract targets
				for(auto& param:command.params_)
				{
					__SUP_COUT__ << "\t param " <<
							param.first << " : " <<
							param.second << std::endl;

					if(param.first ==
							IterateConfiguration::targetParams_.Tables_)
					{
						__SUP_COUT__ << "\t\t found target tables" << __E__;
						std::istringstream f(param.second);

						targetIndex = 0;
						while (getline(f, targetStr, '='))
						{
							__SUP_COUT__ << "\t\t targetStr = " << targetStr << __E__;
							if(!command.targets_.size() ||
									command.targets_.back().table_ != "")
							{
								__SUP_COUT__ << "\t\t make targetStr = " << targetStr << __E__;
								//make new target
								command.addTarget();
								command.targets_.back().table_ = targetStr;
							}
							else //file existing target
								command.targets_[targetIndex++].table_ = targetStr;
						}

						continue; //go to next parameter
					}

					if(param.first ==
							IterateConfiguration::targetParams_.UIDs_)
					{
						__SUP_COUT__ << "\t\t found target UIDs" << __E__;
						std::istringstream f(param.second);

						targetIndex = 0;
						while (getline(f, targetStr, '='))
						{
							__SUP_COUT__ << "\t\t targetStr = " << targetStr << __E__;
							if(!command.targets_.size() ||
									command.targets_.back().UID_ != "")
							{
								__SUP_COUT__ << "\t\t make targetStr = " << targetStr << __E__;
								//make new target
								command.addTarget();
								command.targets_.back().UID_ = targetStr;
							}
							else //file existing target
								command.targets_[targetIndex++].UID_ = targetStr;
						}
						continue;
					}

					cmdCol = commandTypeToCommandTableMap[command.type_].cfgView_->findCol(
							param.first);

					__SUP_COUT__ << "param col " << cmdCol << std::endl;

					commandTypeToCommandTableMap[command.type_].cfgView_->setURIEncodedValue(
							param.second,cmdRow,cmdCol);
				} //end parameter loop

				cmdUID = commandTypeToCommandTableMap[command.type_].cfgView_->getDataView()
						[cmdRow][commandTypeToCommandTableMap[command.type_].cfgView_->getColUID()];

				if(command.targets_.size())
				{
					//if targets, create group in target table

					__SUP_COUT__ << "targets found for command UID=" << cmdUID << __E__;

					//create link from command table to target
					cmdCol = commandTypeToCommandTableMap[command.type_].cfgView_->findCol(
							IterateConfiguration::commandTargetCols_.TargetsLink_);
					commandTypeToCommandTableMap[command.type_].cfgView_->setValueAsString(
							IterateConfiguration::TARGET_TABLE,
							cmdRow,
							cmdCol);

					cmdCol = commandTypeToCommandTableMap[command.type_].cfgView_->findCol(
							IterateConfiguration::commandTargetCols_.TargetsLinkGroupID_);
					commandTypeToCommandTableMap[command.type_].cfgView_->setValueAsString(
							cmdUID + "_Targets",
							cmdRow,
							cmdCol);

					//create row(s) for each target in target table with correct groupID

					for(const auto& target:command.targets_)
					{
						__SUP_COUT__ << target.table_ << " " << target.UID_ << __E__;

						//create target entry in target table in group
						tgtRow = targetTable.cfgView_->addRow(author,"commandTarget");
						targetTable.cfgView_->addRowToGroup(
								tgtRow,
								targetGroupIdCol,
								cmdUID + "_Targets");

						//set target table
						targetTable.cfgView_->setValueAsString(
								target.table_,
								tgtRow,
								targetTableCol);

						//set target UID
						targetTable.cfgView_->setValueAsString(
								target.UID_,
								tgtRow,
								targetUIDCol);
					}
				} //end target handling


				//add link at plan level to created UID
				planTable.cfgView_->setValueAsString(
						commandTypeToCommandTableMap[command.type_].configName_,
						row,
						commandUidLink.first);
				planTable.cfgView_->setValueAsString(
						cmdUID,
						row,
						commandUidLink.second);

				__SUP_COUT__ << "linked to uid = " <<
						cmdUID << std::endl;

				commandTypeToCommandTableMap[command.type_].modified_ = true;
			} //done with command specifics

		} //end command loop


		//commands are created in the temporary tables
		//	validate with init

		planTable.cfgView_->print();
		planTable.cfgView_->init(); //verify new table (throws runtime_errors)

		__SUP_COUT__ << "requestType tables:" << std::endl;

		for(auto& modifiedConfig : commandTypeToCommandTableMap)
		{
			modifiedConfig.second.cfgView_->print();
			modifiedConfig.second.cfgView_->init();
		}

		targetTable.cfgView_->print();
		targetTable.cfgView_->init(); //verify new table (throws runtime_errors)

	} //end try for plan
	catch(...)
	{
		__SUP_COUT__ << "Handling command table errors while saving. Erasing all newly created versions." << std::endl;

		//erase all temporary tables if created here

		if(planTable.createdTemporaryVersion_) //if temporary version created here
		{
			__SUP_COUT__ << "Erasing temporary version " << planTable.configName_ <<
									"-v" << planTable.temporaryVersion_ << std::endl;
			//erase with proper version management
			cfgMgr->eraseTemporaryVersion(planTable.configName_,planTable.temporaryVersion_);
		}

		if(targetTable.createdTemporaryVersion_) //if temporary version created here
		{
			__SUP_COUT__ << "Erasing temporary version " << targetTable.configName_ <<
									"-v" << targetTable.temporaryVersion_ << std::endl;
			//erase with proper version management
			cfgMgr->eraseTemporaryVersion(targetTable.configName_,targetTable.temporaryVersion_);
		}

		for(auto& modifiedConfig : commandTypeToCommandTableMap)
		{
			if(modifiedConfig.second.createdTemporaryVersion_) //if temporary version created here
			{
				__SUP_COUT__ << "Erasing temporary version " << modifiedConfig.second.configName_ <<
						"-v" << modifiedConfig.second.temporaryVersion_ << std::endl;
				//erase with proper version management
				cfgMgr->eraseTemporaryVersion(modifiedConfig.second.configName_,
						modifiedConfig.second.temporaryVersion_);
			}
		}

		throw; //re-throw
	}

	//all edits are complete and tables verified
	//	need to save all edits properly
	//	if not modified, discard

	ConfigurationVersion finalVersion = saveModifiedVersionXML(xmlOut,cfgMgr,
			planTable.configName_,
			planTable.originalVersion_,true /*make temporary*/,
			planTable.config_,planTable.temporaryVersion_,true /*ignoreDuplicates*/); //save temporary version properly

	__SUP_COUT__ << "Final plan version is " << planTable.configName_ << "-v" <<
			finalVersion << std::endl;

	finalVersion = saveModifiedVersionXML(xmlOut,cfgMgr,
			targetTable.configName_,
			targetTable.originalVersion_,true /*make temporary*/,
			targetTable.config_,targetTable.temporaryVersion_,true /*ignoreDuplicates*/); //save temporary version properly

	__SUP_COUT__ << "Final target version is " << targetTable.configName_ << "-v" <<
			finalVersion << std::endl;

	for(auto& modifiedConfig : commandTypeToCommandTableMap)
	{
		if(!modifiedConfig.second.modified_)
		{
			if(modifiedConfig.second.createdTemporaryVersion_) //if temporary version created here
			{
				__SUP_COUT__ << "Erasing unmodified temporary version " << modifiedConfig.second.configName_ <<
						"-v" << modifiedConfig.second.temporaryVersion_ << std::endl;
				//erase with proper version management
				cfgMgr->eraseTemporaryVersion(modifiedConfig.second.configName_,
						modifiedConfig.second.temporaryVersion_);
			}
			continue;
		}

		finalVersion = saveModifiedVersionXML(xmlOut,cfgMgr,
				modifiedConfig.second.configName_,
				modifiedConfig.second.originalVersion_,true /*make temporary*/,
				modifiedConfig.second.config_,
				modifiedConfig.second.temporaryVersion_,true /*ignoreDuplicates*/); //save temporary version properly

		__SUP_COUT__ << "Final version is " << modifiedConfig.second.configName_ << "-v" <<
				finalVersion << std::endl;
	}

	handleFillModifiedTablesXML(xmlOut,cfgMgr);
}
catch(std::runtime_error& e)
{
	__SUP_SS__ << "Error detected saving Iteration Plan!\n\n " << e.what() << std::endl;
	__SUP_COUT_ERR__ << "\n" << ss.str() << std::endl;
	xmlOut.addTextElementToData("Error", ss.str());
}
catch(...)
{
	__SUP_SS__ << "Error detected saving Iteration Plan!\n\n "<< std::endl;
	__SUP_COUT_ERR__ << "\n" << ss.str() << std::endl;
	xmlOut.addTextElementToData("Error", ss.str());
}  //end handleSavePlanCommandSequenceXML

//========================================================================================================================
//handleSaveTreeNodeEditXML
//	Changes the value specified by UID/Column
//	 in the specified version of the table.
//
//	Error, if the specified version is not the active one.
//	If the version is not temporary make a new temporary version
//
//	return this information on success
//	<resultingTargetTableVersion = xxx>
void ConfigurationGUISupervisor::handleSaveTreeNodeEditXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr, const std::string& configName,
		ConfigurationVersion version, const std::string& type,
		const std::string& uid, const std::string& colName, const std::string& newValue,
		const std::string& author)
try
{
	__SUP_COUT__ << "table " <<
			configName << "(" << version << ")" << std::endl;


	//get the current table/version
	//check if the value is new
	//if new edit value (in a temporary version only)


	//get table and activate target version
	ConfigurationBase* config = cfgMgr->getConfigurationByName(configName);
	try
	{
		config->setActiveView(version);
	}
	catch(...)
	{
		__SUP_COUT__ << "Failed to find stored version, so attempting to load version: " <<
				version << std::endl;
		cfgMgr->getVersionedConfigurationByName(
				configName, version);
	}

	__SUP_COUT__ << "Active version is " << config->getViewVersion() << std::endl;

	if(version != config->getViewVersion())
	{
		__SUP_SS__ << "Target table version (" << version <<
				") is not the currently active version (" << config->getViewVersion()
				<< ". Try refreshing the tree." << std::endl;
		throw std::runtime_error(ss.str());
	}

	unsigned int col = -1;
	if(type == "uid" || type == "delete-uid")
		col = config->getView().getColUID();
	else if(
			type == "link-UID" ||
			type == "link-GroupID" ||
			type == "value" ||
			type == "value-groupid" ||
			type == "value-bool"||
			type == "value-bitmap")
		col = config->getView().findCol(colName);
	else if(
			type == "table" ||
			type == "link-comment" ||
			type == "table-newGroupRow" ||
			type == "table-newUIDRow" ||
			type == "table-newRow"); // column N/A
	else
	{
		__SUP_SS__ << "Impossible! Unrecognized edit type: " << type << std::endl;
		throw std::runtime_error(ss.str());
	}

	//check if the comment value is new before making temporary version
	if(type == "table" ||
			type == "link-comment")
	{
		//editing comment, so check if comment is different
		if(config->getView().isURIEncodedCommentTheSame(newValue))
		{
			__SUP_SS__ << "Comment '" << newValue <<
					"' is the same as the current comment. No need to save change." <<
					std::endl;
			throw std::runtime_error(ss.str());
		}

	}


	//version handling:
	//	always make a new temporary-version from source-version
	//	edit temporary-version
	//		if edit fails
	//			delete temporary-version
	//		else
	//			return new temporary-version
	//			if source-version was temporary
	//				then delete source-version

	ConfigurationVersion temporaryVersion = config->createTemporaryView(version);

	__SUP_COUT__ << "Created temporary version " << temporaryVersion << std::endl;

	ConfigurationView* cfgView = config->getTemporaryView(temporaryVersion);

	//edit/verify new table (throws runtime_errors)
	try
	{
		//have view so edit it
		if(type == "table" ||
				type == "link-comment")
		{
			//edit comment
			cfgView->setURIEncodedComment(newValue);
		}
		else if(type == "table-newRow" ||
				type == "table-newUIDRow")
		{
			//add row
			unsigned int row = cfgView->addRow(author,true /*incrementUniqueData*/);

			//if ViewColumnInfo::COL_NAME_STATUS exists, set it to true
			try
			{
				col = cfgView->getColStatus();
				cfgView->setValueAsString("1",row,col);
			}
			catch(...) {} //if not, ignore

			//set UID value
			cfgView->setURIEncodedValue(newValue,row,cfgView->getColUID());
		}
		else if(type == "table-newGroupRow")
		{
			//add row
			unsigned int row = cfgView->addRow(author,true /*incrementUniqueData*/);

			//get index value and group id value
			unsigned int csvIndex = newValue.find(',');

			std::string linkIndex = newValue.substr(0,csvIndex);
			std::string groupId = newValue.substr(csvIndex+1);

			//get new row UID value from second part of string
			csvIndex = groupId.find(',');
			std::string newRowUID = groupId.substr(csvIndex+1);
			groupId = groupId.substr(0,csvIndex);

			__SUP_COUT__ << "newValue " << linkIndex << "," <<
					groupId << "," <<
					newRowUID << std::endl;

			//set UID value
			cfgView->setURIEncodedValue(newRowUID,row,cfgView->getColUID());

			//find groupId column from link index
			col = cfgView->getColLinkGroupID(linkIndex);

			//set group id
			cfgView->setURIEncodedValue(groupId,row,col);

			//if ViewColumnInfo::COL_NAME_STATUS exists, set it to true
			try
			{
				col = cfgView->getColStatus();
				cfgView->setValueAsString("1",row,col);
			}
			catch(...) {} //if not, ignore

		}
		else if(type == "delete-uid")
		{
			//delete row
			unsigned int row = cfgView->findRow(col,uid);
			cfgView->deleteRow(row);
		}
		else if(type == "uid" ||
				type == "value" ||
				type == "value-groupid" ||
				type == "value-bool" ||
				type == "value-bitmap")
		{
			unsigned int row = cfgView->findRow(cfgView->getColUID(),uid);
			if(!cfgView->setURIEncodedValue(newValue,row,col,author))
			{
				//no change! so discard
				__SUP_SS__ << "Value '" << newValue <<
						"' is the same as the current value. No need to save change to tree node." <<
						std::endl;
				throw std::runtime_error(ss.str());
			}
		}
		else if(type == "link-UID" || type == "link-GroupID")
		{
			bool isGroup;
			std::pair<unsigned int /*link col*/, unsigned int /*link id col*/> linkPair;
			if(!cfgView->getChildLink(col,isGroup,linkPair))
			{
				//not a link ?!
				__SUP_SS__ << "Col '" << colName <<
						"' is not a link column." <<
						std::endl;
				throw std::runtime_error(ss.str());
			}

			__SUP_COUT__ << "linkPair " << linkPair.first << "," <<
					linkPair.second << std::endl;

			std::string linkIndex = cfgView->getColumnInfo(col).getChildLinkIndex();

			__SUP_COUT__ << "linkIndex " << linkIndex << std::endl;

			//find table value and id value
			unsigned int csvIndexStart = 0,csvIndex = newValue.find(',');

			std::string newTable = newValue.substr(csvIndexStart,csvIndex);
			csvIndexStart = csvIndex + 1;
			csvIndex = newValue.find(',',csvIndexStart);
			std::string newLinkId =
					newValue.substr(csvIndexStart,csvIndex-csvIndexStart); //if no more commas will take the rest of string

			__SUP_COUT__ << "newValue " << newTable << "," <<
					newLinkId << std::endl;

			//change target table in two parts
			unsigned int row = cfgView->findRow(cfgView->getColUID(),uid);
			bool changed = false;
			if(!cfgView->setURIEncodedValue(newTable,row,
					linkPair.first,author))
			{
				//no change
				__SUP_COUT__ << "Value '" << newTable <<
						"' is the same as the current value." <<
						std::endl;
			}
			else
				changed = true;

			if(!cfgView->setURIEncodedValue(newLinkId,row,
					linkPair.second,author))
			{
				//no change
				__SUP_COUT__ << "Value '" << newLinkId <<
						"' is the same as the current value." <<
						std::endl;
			}
			else
				changed = true;



			//handle groupID links slightly differently
			//	have to look at changing link table too!
			//if group ID set all in member list to be members of group
			if(type == "link-GroupID")
			{
				bool secondaryChanged = false;

				//first close out main target table
				if(!changed) //if no changes throw out new version
				{
					__SUP_COUT__ << "No changes to primary view. Erasing temporary table." << std::endl;
					config->eraseView(temporaryVersion);
				}
				else	//if changes, save it
				{
					try
					{
						cfgView->init(); //verify new table (throws runtime_errors)

						saveModifiedVersionXML(xmlOut,cfgMgr,configName,version,true /*make temporary*/,
								config,temporaryVersion, true /*ignoreDuplicates*/); //save temporary version properly
					}
					catch(std::runtime_error& e) //erase temporary view before re-throwing error
					{
						__SUP_COUT__ << "Caught error while editing main table. Erasing temporary version." << std::endl;
						config->eraseView(temporaryVersion);
						changed = false; //undo changed bool

						//send warning so that, secondary table can still be changed
						xmlOut.addTextElementToData("Warning", "Error saving primary tree node! " + std::string(e.what()));
					}
				}

				//now, onto linked table

				//get the current linked table/version
				//check if the value is new
				//if new edit value (in a temporary version only)

				csvIndexStart = csvIndex + 1;
				csvIndex = newValue.find(',',csvIndexStart);
				version = ConfigurationVersion(
						newValue.substr(csvIndexStart,csvIndex-csvIndexStart)); //if no more commas will take the rest of string


				if(newTable == ViewColumnInfo::DATATYPE_LINK_DEFAULT)
				{
					//done, since init was already tested
					// the result should be purposely DISCONNECTED link
					return;
				}

				//get table and activate target version
				config = cfgMgr->getConfigurationByName(newTable);
				try
				{
					config->setActiveView(version);
				}
				catch(...)
				{
					__SUP_COUT__ << "Failed to find stored version, so attempting to load version: " <<
							version << std::endl;
					cfgMgr->getVersionedConfigurationByName(
							newTable, version);
				}

				__SUP_COUT__ << "Active version is " << config->getViewVersion() << std::endl;

				if(version != config->getViewVersion())
				{
					__SUP_SS__ << "Target table version (" << version <<
							") is not the currently active version (" << config->getViewVersion()
							<< ". Try refreshing the tree." << std::endl;
					throw std::runtime_error(ss.str());
				}


				//create temporary version for editing
				temporaryVersion = config->createTemporaryView(version);

				__SUP_COUT__ << "Created temporary version " << temporaryVersion << std::endl;

				cfgView = config->getTemporaryView(temporaryVersion);

				col = cfgView->getColLinkGroupID(linkIndex);

				__SUP_COUT__ << "target col " << col << std::endl;


				//extract vector of members to be
				std::vector<std::string> memberUIDs;
				do
				{
					csvIndexStart = csvIndex + 1;
					csvIndex = newValue.find(',',csvIndexStart);
					memberUIDs.push_back(newValue.substr(csvIndexStart,csvIndex-csvIndexStart));
					__SUP_COUT__ << "memberUIDs: " << memberUIDs.back() << std::endl;
				} while(csvIndex != (unsigned int)std::string::npos); //no more commas


				//for each row,
				//	check if should be in group
				//		if should be but is not
				//			add to group, CHANGE
				//		if should not be but is
				//			remove from group, CHANGE
				//

				std::string targetUID;
				bool shouldBeInGroup;
				bool defaultIsInGroup = false; //use to indicate if a recent new member was created
				bool isInGroup;

				for(unsigned int row=0;row<cfgView->getNumberOfRows();++row)
				{
					targetUID = cfgView->getDataView()[row][cfgView->getColUID()];
					__SUP_COUT__ << "targetUID: " << targetUID << std::endl;

					shouldBeInGroup = false;
					for(unsigned int i=0;i<memberUIDs.size();++i)
						if(targetUID == memberUIDs[i])
						{
							//found in member uid list
							shouldBeInGroup = true;
							break;
						}

					isInGroup = cfgView->isEntryInGroup(row,linkIndex,newLinkId);

					//if should be but is not
					if(shouldBeInGroup && !isInGroup)
					{

						__SUP_COUT__ << "Changed YES: " << row << std::endl;
						secondaryChanged = true;

						cfgView->addRowToGroup(row,col,newLinkId);

					}//if should not be but is
					else if(!shouldBeInGroup &&	isInGroup)
					{

						__SUP_COUT__ << "Changed NO: " << row << std::endl;
						secondaryChanged = true;

						cfgView->removeRowFromGroup(row,col,newLinkId);
					}
					else if(targetUID ==
							cfgView->getDefaultRowValues()[cfgView->getColUID()]  &&
							isInGroup)
					{
						//use to indicate if a recent new member was created
						defaultIsInGroup = true;
					}
				}


				//first close out main target table
				if(!secondaryChanged) //if no changes throw out new version
				{
					__SUP_COUT__ << "No changes to secondary view. Erasing temporary table." << std::endl;
					config->eraseView(temporaryVersion);
				}
				else	//if changes, save it
				{
					try
					{
						cfgView->init(); //verify new table (throws runtime_errors)

						saveModifiedVersionXML(xmlOut,cfgMgr,newTable,version,true /*make temporary*/,
								config,temporaryVersion,true /*ignoreDuplicates*/); //save temporary version properly
					}
					catch(std::runtime_error& e) //erase temporary view before re-throwing error
					{
						__SUP_COUT__ << "Caught error while editing secondary table. Erasing temporary version." << std::endl;
						config->eraseView(temporaryVersion);
						secondaryChanged = false; //undo changed bool

						//send warning so that, secondary table can still be changed
						xmlOut.addTextElementToData("Warning", "Error saving secondary tree node! " + std::string(e.what()));
					}
				}

				//block error message if default is in group, assume new member was just created
				//	RAR: block because its hard to detect if changes were recently made (one idea: to check if all other values are defaults, to assume it was just created)
				if(0 && !changed && !secondaryChanged && !defaultIsInGroup)
				{
					__SUP_SS__ << "Link to table '" << newTable <<
							"', linkID '" << newLinkId <<
							"', and selected group members are the same as the current value. " <<
							"No need to save changes to tree." <<
							std::endl;
					throw std::runtime_error(ss.str());
				}

				return;	//exit since table inits were already tested
			}
			else if(0 && !changed) //block error message because sometimes things get setup twice depending on the path of the user (e.g. when editing links in tree-view)
			{						//RAR: block also becuase versions are temporary at this point anyway, might as well abuse temporary versions
				__SUP_SS__ << "Link to table '" << newTable <<
						"' and linkID '" << newLinkId <<
						"' are the same as the current values. No need to save change to tree node." <<
						std::endl;
				throw std::runtime_error(ss.str());
			}
		}

		cfgView->init(); //verify new table (throws runtime_errors)
	}
	catch(...) //erase temporary view before re-throwing error
	{
		__SUP_COUT__ << "Caught error while editing. Erasing temporary version." << std::endl;
		config->eraseView(temporaryVersion);
		throw;
	}

	saveModifiedVersionXML(xmlOut,cfgMgr,configName,version,true /*make temporary*/,
			config,temporaryVersion,true /*ignoreDuplicates*/); //save temporary version properly
}
catch(std::runtime_error& e)
{
	__SUP_SS__ << "Error saving tree node! " << std::string(e.what()) << std::endl;
	__SUP_COUT_ERR__ << "\n" << ss.str() << std::endl;
	xmlOut.addTextElementToData("Error", ss.str());
}
catch(...)
{
	__SUP_SS__ << "Error saving tree node! " << std::endl;
	__SUP_COUT_ERR__ << "\n" << ss.str() << std::endl;
	xmlOut.addTextElementToData("Error", ss.str());
}

//========================================================================================================================
//handleGetConfigurationGroupXML
//
//	give the detail of specific System Configuration specified
//		groupKey=-1 returns latest
//
//	Find historical group keys
//		and figure out all member configurations versions

//
//	return this information
//	<group name=xxx key=xxx>
//		<historical key=xxx>
//		<historical key=xxx>
//		....
//		<config name=xxx version=xxx />
//			<historical version=xxx>
//			<historical version=xxx>
//			...
//		</config>
//		<config name=xxx version=xxx>
//		...
//	</configuration>
void ConfigurationGUISupervisor::handleGetConfigurationGroupXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr, const std::string& groupName,
		ConfigurationGroupKey groupKey)
try
{
	char tmpIntStr[100];
	DOMElement* parentEl,* configEl;

	//steps:
	//	if invalid key, get latest key
	//	get specific group with key
	//		give member names and versions
	//		get all configuration groups to locate historical keys
	//	get all groups to find historical keys


//	std::set<std::string /*name+version*/> allGroups =
//			cfgMgr->getConfigurationInterface()->getAllConfigurationGroupNames(groupName);
//	std::string name;
//	ConfigurationGroupKey key;
//	//put them in a set to sort them as ConfigurationGroupKey defines for operator<
//	std::set<ConfigurationGroupKey> sortedKeys;
//	for(auto& group: allGroups)
//	{
//		//now uses database filter
//		ConfigurationGroupKey::getGroupNameAndKey(group,name,key);
//		//if(name == groupName)
//		sortedKeys.emplace(key);
//	}

	const GroupInfo& groupInfo = cfgMgr->getGroupInfo(groupName);
	const std::set<ConfigurationGroupKey>& sortedKeys = groupInfo.keys_; //rename

	if(groupKey.isInvalid() || //if invalid or not found, get latest
			sortedKeys.find(groupKey) == sortedKeys.end())
	{
		if(sortedKeys.size())
			groupKey =* sortedKeys.rbegin();
		__SUP_COUT__ << "Group key requested was invalid or not found, going with latest " <<
				groupKey << std::endl;
	}



	xmlOut.addTextElementToData("ConfigurationGroupName", groupName);
	xmlOut.addTextElementToData("ConfigurationGroupKey", groupKey.toString());

	//add all other sorted keys for this groupName
	for(auto& keyInOrder:sortedKeys)
		xmlOut.addTextElementToData("HistoricalConfigurationGroupKey", keyInOrder.toString());


	parentEl = xmlOut.addTextElementToData("ConfigurationGroupMembers", "");


	//	get specific group with key
	std::map<std::string /*name*/, ConfigurationVersion /*version*/> memberMap;
	std::map<std::string /*name*/, std::string /*alias*/> groupMemberAliases;

	__SUP_COUT__ << "groupName=" << groupName << std::endl;
	__SUP_COUT__ << "groupKey=" << groupKey << std::endl;

	const std::map<std::string, ConfigurationInfo>& allCfgInfo = cfgMgr->getAllConfigurationInfo();
	std::map<std::string, ConfigurationInfo>::const_iterator it;

	//load group so comments can be had
	//	and also group metadata (author, comment, createTime)
	//bool commentsLoaded = false;
	try
	{
		std::string groupAuthor, groupComment, groupCreationTime, groupTypeString;
		cfgMgr->loadConfigurationGroup(groupName,groupKey,
				false /*doActivate*/,&memberMap,0 /*progressBar*/,0 /*accumulateErrors*/,
				&groupComment, &groupAuthor, &groupCreationTime,
				false /*doNotLoadMember*/, &groupTypeString, &groupMemberAliases);

		//commentsLoaded = true;

		xmlOut.addTextElementToData("ConfigurationGroupAuthor", groupAuthor);
		xmlOut.addTextElementToData("ConfigurationGroupComment", groupComment);
		xmlOut.addTextElementToData("ConfigurationGroupCreationTime", groupCreationTime);
		xmlOut.addTextElementToData("ConfigurationGroupType", groupTypeString);
	}
	catch(const std::runtime_error& e)
	{
	  __SUP_SS__ <<"Configuration group \"" +
				groupName + "(" + groupKey.toString() + ")" +
	    "\" members can not be loaded!\n\n" + e.what();
	  __SUP_COUT_ERR__ << ss.str();
	    xmlOut.addTextElementToData("Error",ss.str());
		//return;
	}
	catch(...)
	{
	  __SUP_SS__ << "Configuration group \"" +
				groupName + "(" + groupKey.toString() + ")" +
	    "\" members can not be loaded!" << __E__;
	  __SUP_COUT_ERR__ << ss.str();
	  xmlOut.addTextElementToData("Error",ss.str());
		//return;
	}

	__COUTV__(StringMacros::mapToString(groupMemberAliases));

	std::map<std::string,std::map<std::string,ConfigurationVersion> > versionAliases =
			cfgMgr->getVersionAliases();

	__SUP_COUT__ << "# of table version aliases: " << versionAliases.size() << std::endl;



	//Seperate loop to get name and version
	for(auto& memberPair:memberMap)
	  {
		xmlOut.addTextElementToParent("MemberName", memberPair.first, parentEl);

		//if member is in groupMemberAliases, then alias version
		if(groupMemberAliases.find(memberPair.first) != groupMemberAliases.end())
			configEl = xmlOut.addTextElementToParent("MemberVersion",
					ConfigurationManager::ALIAS_VERSION_PREAMBLE +
					groupMemberAliases[memberPair.first], //return the ALIAS:<alias>
					parentEl);
		else
			configEl = xmlOut.addTextElementToParent("MemberVersion", memberPair.second.toString(),
					parentEl);
	
		it = allCfgInfo.find(memberPair.first);
		if(it == allCfgInfo.end())
		{
			xmlOut.addTextElementToData("Error","Configuration \"" +
					memberPair.first +
					"\" can not be retrieved!");
			continue;
		}

		if(versionAliases.find(it->first) != versionAliases.end())
		  for (auto& aliasVersion:versionAliases[it->first])
		    xmlOut.addTextElementToParent("ConfigurationExistingVersion",
						  ConfigurationManager::ALIAS_VERSION_PREAMBLE + aliasVersion.first,
						  configEl);
		
		for (auto& version:it->second.versions_)
		  //if(version == memberPair.second) continue; //CHANGED by RAR on 11/14/2016 (might as well show all versions in list to avoid user confusion)
		  //else
		  xmlOut.addTextElementToParent("ConfigurationExistingVersion", version.toString(), configEl);
	  }
	//Seperate loop just for getting the Member Comment
	for(auto& memberPair:memberMap)
	{
		//__SUP_COUT__ << "\tMember config " << memberPair.first << ":" <<
		//		memberPair.second << std::endl;

		//xmlOut.addTextElementToParent("MemberName", memberPair.first, parentEl);
		//if(commentsLoaded)
			xmlOut.addTextElementToParent("MemberComment",
					allCfgInfo.at(memberPair.first).configurationPtr_->getView().getComment(),
					parentEl);
		//else
		//	xmlOut.addTextElementToParent("MemberComment", "", parentEl);


			//	__SUP_COUT__ << "\tMember config " << memberPair.first << ":" <<
			//	memberPair.second << std::endl;

		//configEl = xmlOut.addTextElementToParent("MemberVersion", memberPair.second.toString(), parentEl);

		/*	it = allCfgInfo.find(memberPair.first);
		if(it == allCfgInfo.end())
		{
			xmlOut.addTextElementToData("Error","Configuration \"" +
					memberPair.first +
					"\" can not be retrieved!");
			return;
		}
		*/
		//include aliases for this table
		/*if(versionAliases.find(it->first) != versionAliases.end())
			for (auto& aliasVersion:versionAliases[it->first])
				xmlOut.addTextElementToParent("ConfigurationExistingVersion",
						ConfigurationManager::ALIAS_VERSION_PREAMBLE + aliasVersion.first,
						configEl);

		for (auto& version:it->second.versions_)
			//if(version == memberPair.second) continue; //CHANGED by RAR on 11/14/2016 (might as well show all versions in list to avoid user confusion)
			//else
			xmlOut.addTextElementToParent("ConfigurationExistingVersion", version.toString(), configEl);
		*/
	}


	return;
}
catch(std::runtime_error& e)
{
	__SUP_SS__ << ("Error!\n\n" + std::string(e.what())) << std::endl;
	__SUP_COUT_ERR__ << "\n" << ss.str();
	xmlOut.addTextElementToData("Error", ss.str());
}
catch(...)
{
	__SUP_SS__ << ("Error!\n\n") << std::endl;
	__SUP_COUT_ERR__ << "\n" << ss.str();
	xmlOut.addTextElementToData("Error", ss.str());
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
//<subconfiguration name=xxx version=xxx rowCount=xxx chunkReq=xxx chunkSz=xxx>
//	<existing version=xxx>
//	<existing version=xxx>
//	....
//	<colhdr name=xxx>
//	<colhdr name=xxx>
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
//
//
// Note: options.. if allowIllegalColumns then attempts to load data to current mockup column names
//	if not allowIllegalColumns, then it is still ok if the source has more or less columns:
//	the client is notified through "TableWarnings" field in this case.
void ConfigurationGUISupervisor::handleGetConfigurationXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr, const std::string& tableName,
		ConfigurationVersion version, bool allowIllegalColumns)
try
{
	char tmpIntStr[100];
	DOMElement *parentEl, *subparentEl;

	std::string accumulatedErrors = "";

	const std::map<std::string, ConfigurationInfo>& allCfgInfo = //if allowIllegalColumns, then also refresh
			cfgMgr->getAllConfigurationInfo(allowIllegalColumns,
					allowIllegalColumns?&accumulatedErrors:0,tableName); //filter errors by tableName

	ConfigurationBase* config = cfgMgr->getConfigurationByName(tableName);

	//send all config names along with
	//	and check for specific version
	xmlOut.addTextElementToData("ExistingConfigurationNames", ViewColumnInfo::DATATYPE_LINK_DEFAULT);
	for(auto& configPair:allCfgInfo)
	{
		xmlOut.addTextElementToData("ExistingConfigurationNames",
				configPair.first);
		if(configPair.first == tableName && //check that version exists
				configPair.second.versions_.find(version) ==
						configPair.second.versions_.end())
		{
			__SUP_COUT__ << "Version not found, so using mockup." << std::endl;
			version = ConfigurationVersion(); //use INVALID
		}
	}

	xmlOut.addTextElementToData("ConfigurationName", tableName);	//table name
	xmlOut.addTextElementToData("ConfigurationDescription",
			config->getConfigurationDescription());	//table name


	//existing table versions
	{
		//get version aliases for translation
		std::map<std::string /*table name*/,std::map<
		std::string /*version alias*/,ConfigurationVersion /*aliased version*/> > versionAliases;
		try
		{
			//use whatever backbone is currently active
			versionAliases = cfgMgr->getVersionAliases();
			for(const auto& aliases:versionAliases)
				for(const auto& alias:aliases.second)
					__SUP_COUT__ << "ALIAS: " << aliases.first << " " << alias.first << " ==> " << alias.second << std::endl;
		}
		catch(const std::runtime_error& e)
		{
			__SUP_COUT__ << "Could not get backbone information for version aliases: " << e.what() << __E__;
		}

		auto tableIterator = versionAliases.find(tableName);

		parentEl = xmlOut.addTextElementToData("ConfigurationVersions", "");
		for(const ConfigurationVersion& v:allCfgInfo.at(tableName).versions_)
		{
			subparentEl = xmlOut.addTextElementToParent("Version", v.toString(), parentEl);

			if(tableIterator != versionAliases.end())
			{
				//check if this version has one or many aliases
				for(const auto& aliasPair : tableIterator->second)
				{
					//					__SUP_COUT__ << "Checking " << aliasPair.second << " --> " <<
					//							aliasPair.first << " for " << v << __E__;
					if(v == aliasPair.second)
					{
						__SUP_COUT__ << "Found Alias " << aliasPair.second << " --> " <<
								aliasPair.first << __E__;
						xmlOut.addTextElementToParent("VersionAlias", aliasPair.first, subparentEl);
					}
				}
			}

		}
	}

	//table columns and then rows (from config view)

	//get view pointer
	ConfigurationView* cfgViewPtr;
	if(version.isInvalid()) //use mock-up
	{
		cfgViewPtr = config->getMockupViewP();
	}
	else					//use view version
	{
		try
		{
			cfgViewPtr = cfgMgr->getVersionedConfigurationByName(tableName,version)->getViewP();
		}
		catch(std::runtime_error& e) //default to mock-up for fail-safe in GUI editor
		{
			__SUP_SS__ << "Failed to get table " << tableName <<
					" version " << version <<
					"... defaulting to mock-up! " <<
					std::endl;
			ss << "\n\n...Here is why it failed:\n\n" << e.what() << std::endl;

			__SUP_COUT_ERR__ << "\n" << ss.str();
			version = ConfigurationVersion();
			cfgViewPtr = config->getMockupViewP();

			xmlOut.addTextElementToData("Error", "Error getting view! " + ss.str());
		}
		catch(...) //default to mock-up for fail-safe in GUI editor
		{
			__SUP_SS__ << "Failed to get table " << tableName <<
					" version: " << version <<
					"... defaulting to mock-up! " <<
					"(You may want to try again to see what was partially loaded into cache before failure. " <<
					"If you think, the failure is due to a column name change, " <<
					"you can also try to Copy the failing view to the new column names using " <<
					"'Copy and Move' functionality.)" <<
					std::endl;

			__SUP_COUT_ERR__ << "\n" << ss.str();
			version = ConfigurationVersion();
			cfgViewPtr = config->getMockupViewP();

			xmlOut.addTextElementToData("Error", "Error getting view! " + ss.str());
		}
	}
	xmlOut.addTextElementToData("ConfigurationVersion", version.toString());	//table version

	//get 'columns' of view
	DOMElement* choicesParentEl;
	parentEl = xmlOut.addTextElementToData("CurrentVersionColumnHeaders", "");

	std::vector<ViewColumnInfo> colInfo = cfgViewPtr->getColumnsInfo();

	for(int i=0;i<(int)colInfo.size();++i)	//column headers and types
	{
		//		__SUP_COUT__ << "\t\tCol " << i << ": " << colInfo[i].getType()  << "() " <<
		//				colInfo[i].getName() << " "
		//				<< colInfo[i].getStorageName() << " " << colInfo[i].getDataType() << std::endl;

		xmlOut.addTextElementToParent("ColumnHeader", colInfo[i].getName(), parentEl);
		xmlOut.addTextElementToParent("ColumnType", colInfo[i].getType(), parentEl);
		xmlOut.addTextElementToParent("ColumnDataType", colInfo[i].getDataType(), parentEl);

		choicesParentEl =
				xmlOut.addTextElementToParent("ColumnChoices", "", parentEl);
		//add data choices if necessary
		if(colInfo[i].getType() == ViewColumnInfo::TYPE_FIXED_CHOICE_DATA ||
				colInfo[i].getType() == ViewColumnInfo::TYPE_BITMAP_DATA ||
				colInfo[i].isChildLink())
		{
			for(auto& choice:colInfo[i].getDataChoices())
				xmlOut.addTextElementToParent("ColumnChoice", choice, choicesParentEl);
		}
	}

	//verify mockup columns after columns are posted to xmlOut
	try
	{
		if(version.isInvalid())
			cfgViewPtr->init();
	}
	catch(std::runtime_error& e)
	{
		//append accumulated errors, because they may be most useful
		throw std::runtime_error(e.what() + std::string("\n\n") + accumulatedErrors);
	}
	catch(...)
	{
		throw;
	}

	parentEl = xmlOut.addTextElementToData("CurrentVersionRows", "");

	for(int r=0;r<(int)cfgViewPtr->getNumberOfRows();++r)
	{
		//__SUP_COUT__ << "\t\tRow " << r << ": "  << std::endl;

		sprintf(tmpIntStr,"%d",r);
		DOMElement* tmpParentEl = xmlOut.addTextElementToParent("Row", tmpIntStr, parentEl);

		for(int c=0;c<(int)cfgViewPtr->getNumberOfColumns();++c)
		{
			if(colInfo[c].getDataType() == ViewColumnInfo::DATATYPE_TIME)
			{
				std::string timeAsString;
				cfgViewPtr->getValue(timeAsString,r,c);
				xmlOut.addTextElementToParent("Entry", timeAsString, tmpParentEl);
			}
			else
				xmlOut.addTextElementToParent("Entry", cfgViewPtr->getDataView()[r][c], tmpParentEl);
		}
	}

	//add "other" fields associated with configView
	xmlOut.addTextElementToData("TableComment", cfgViewPtr->getComment());
	xmlOut.addTextElementToData("TableAuthor", cfgViewPtr->getAuthor());
	xmlOut.addTextElementToData("TableCreationTime", std::to_string(cfgViewPtr->getCreationTime()));
	xmlOut.addTextElementToData("TableLastAccessTime", std::to_string(cfgViewPtr->getLastAccessTime()));

	//add to xml the default row values
	std::vector<std::string> defaultRowValues =
			cfgViewPtr->getDefaultRowValues();
	//don't give author and time.. force default author, let JS fill time
	for(unsigned int c = 0; c<defaultRowValues.size()-2; ++c)
	{
//		__SUP_COUT__ << "Default for c" << c << "=" <<
//				cfgViewPtr->getColumnInfo(c).getName() << " is " <<
//				defaultRowValues[c] << std::endl;
		xmlOut.addTextElementToData("DefaultRowValue", defaultRowValues[c]);
	}


	if(accumulatedErrors != "") //add accumulated errors to xmlOut
	{
		__SUP_SS__ << (std::string("Column errors were allowed for this request, so maybe you can ignore this, ") +
				"but please note the following errors:\n" + accumulatedErrors) << std::endl;
		__SUP_COUT_ERR__ << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
	}
	else if(!version.isTemporaryVersion() && //not temporary (these are not filled from interface source)
			(cfgViewPtr->getDataColumnSize() !=
					cfgViewPtr->getNumberOfColumns() ||
					cfgViewPtr->getSourceColumnMismatch() != 0)) //check for column size mismatch
	{
		__SUP_SS__ << "\n\nThere were warnings found when loading the table " <<
				tableName << ":v" << version << ". Please see the details below:\n\n" <<
				"The source column size was found to be " << cfgViewPtr->getDataColumnSize() <<
				", and the current number of columns for this table is " <<
				cfgViewPtr->getNumberOfColumns() << ". This resulted in a count of " <<
				cfgViewPtr->getSourceColumnMismatch() << " source column mismatches, and a count of " <<
				cfgViewPtr->getSourceColumnMissing() << " table entries missing in " <<
				cfgViewPtr->getNumberOfRows() << " row(s) of data." << std::endl;



		const std::set<std::string> srcColNames = cfgViewPtr->getSourceColumnNames();
		ss << "\n\nSource column names in ALPHABETICAL order were as follows:\n";
		char index = 'a';
		std::string preIndexStr = "";
		for(auto& srcColName:srcColNames)
		{
			ss << "\n\t" << preIndexStr << index << ". " <<  srcColName;
			if(index == 'z') //wrap-around
			{
				preIndexStr += 'a'; //keep adding index 'digits' for wrap-around
				index = 'a';
			}
			else
				++index;
		}
		ss << std::endl;

		std::set<std::string> destColNames = cfgViewPtr->getColumnStorageNames();
		ss << "\n\nCurrent table column names in ALPHABETICAL order are as follows:\n";
		index = 'a';
		preIndexStr = "";
		for(auto& destColName:destColNames)
		{
			ss << "\n\t" << preIndexStr << index << ". " << destColName;
			if(index == 'z') //wrap-around
			{
				preIndexStr += 'a'; //keep adding index 'digits' for wrap-around
				index = 'a';
			}
			else
				++index;
		}
		ss << std::endl;

		__SUP_COUT__ << "\n" << ss.str();
		xmlOut.addTextElementToData("TableWarnings",ss.str());
	}
}
catch(std::runtime_error& e)
{
	__SUP_COUT__ << "Error detected!\n\n " << e.what() << std::endl;
	xmlOut.addTextElementToData("Error", "Error getting view! " + std::string(e.what()));
}
catch(...)
{
	__SUP_COUT__ << "Error detected!\n\n "<< std::endl;
	xmlOut.addTextElementToData("Error", "Error getting view! ");
}


//========================================================================================================================
//saveModifiedVersionXML
//
// once source version has been modified in temporary version
//	this function finishes it off.
ConfigurationVersion ConfigurationGUISupervisor::saveModifiedVersionXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr, const std::string& configName,
		ConfigurationVersion originalVersion,
		bool makeTemporary,
		ConfigurationBase*  config,
		ConfigurationVersion temporaryModifiedVersion,
		bool ignoreDuplicates,
		bool lookForEquivalent)
{
	bool needToEraseTemporarySource = (originalVersion.isTemporaryVersion() && !makeTemporary);


	//check for duplicate tables already in cache
	if(!ignoreDuplicates)
	{
		__SUP_COUT__ << "Checking for duplicate tables..." << std::endl;

		ConfigurationVersion duplicateVersion;

		{
			//"DEEP" checking
			//	load into cache 'recent' versions for this table
			//		'recent' := those already in cache, plus highest version numbers not in cache
			const std::map<std::string, ConfigurationInfo>& allCfgInfo =
					cfgMgr->getAllConfigurationInfo(); //do not refresh

			auto versionReverseIterator = allCfgInfo.at(configName).versions_.rbegin(); //get reverse iterator
			__SUP_COUT__ << "Filling up cached from " <<
					config->getNumberOfStoredViews() <<
					" to max count of " << config->MAX_VIEWS_IN_CACHE << std::endl;
			for(;config->getNumberOfStoredViews() < config->MAX_VIEWS_IN_CACHE &&
				versionReverseIterator != allCfgInfo.at(configName).versions_.rend();++versionReverseIterator)
			{
				__SUP_COUT__ << "Versions in reverse order " << *versionReverseIterator << std::endl;
				try
				{
					cfgMgr->getVersionedConfigurationByName(configName,*versionReverseIterator); //load to cache
				}
				catch(const std::runtime_error& e)
				{
					__SUP_COUT__ << "Error loadiing historical version, but ignoring: " << e.what() << __E__;
				}
			}
		}

		__SUP_COUT__ << "Checking duplicate..." << std::endl;


		duplicateVersion = config->checkForDuplicate(temporaryModifiedVersion,
				(!originalVersion.isTemporaryVersion() && !makeTemporary)?
						ConfigurationVersion()://if from persistent to persistent, then include original version in search
						originalVersion);

		if(lookForEquivalent && !duplicateVersion.isInvalid())
		{
			//found an equivalent!
			__SUP_COUT__ << "Equivalent table found in version v" << duplicateVersion << std::endl;

			//if duplicate version was temporary, do not use
			if(duplicateVersion.isTemporaryVersion() && !makeTemporary)
			{
				__SUP_COUT__ << "Need persistent. Duplicate version was temporary. Abandoning duplicate." << __E__;
				duplicateVersion = ConfigurationVersion(); //set invalid
			}
			else
			{
				//erase and return equivalent version

				//erase modified equivalent version
				cfgMgr->eraseTemporaryVersion(configName,temporaryModifiedVersion);

				//erase original if needed
				if(needToEraseTemporarySource)
					cfgMgr->eraseTemporaryVersion(configName,originalVersion);

				xmlOut.addTextElementToData("savedName", configName);
				xmlOut.addTextElementToData("savedVersion", duplicateVersion.toString());
				xmlOut.addTextElementToData("foundEquivalentVersion", "1");

				__SUP_COUT__ << "\t\t equivalent AssignedVersion: " << duplicateVersion << std::endl;

				return duplicateVersion;
			}
		}

		if(!duplicateVersion.isInvalid())
		{
			__SUP_SS__ << "This version is identical to another version currently cached v" <<
					duplicateVersion << ". No reason to save a duplicate." << std::endl;
			__SUP_COUT_ERR__ << "\n" << ss.str();

			//delete temporaryModifiedVersion
			config->eraseView(temporaryModifiedVersion);
			throw std::runtime_error(ss.str());
		}

		__SUP_COUT__ << "Check for duplicate tables complete." << std::endl;
	}


	if(makeTemporary)
		__SUP_COUT__ << "\t\t**************************** Save as temporary table version" << std::endl;
	else
		__SUP_COUT__ << "\t\t**************************** Save as new table version" << std::endl;



	ConfigurationVersion newAssignedVersion =
			cfgMgr->saveNewConfiguration(configName,temporaryModifiedVersion,
					makeTemporary);

	if(needToEraseTemporarySource)
		cfgMgr->eraseTemporaryVersion(configName,originalVersion);

	xmlOut.addTextElementToData("savedName", configName);
	xmlOut.addTextElementToData("savedVersion", newAssignedVersion.toString());

	__SUP_COUT__ << "\t\t newAssignedVersion: " << newAssignedVersion << std::endl;
	return newAssignedVersion;
}

//========================================================================================================================
//handleCreateConfigurationXML
//
//	Save the detail of specific Sub-System Configuration specified
//		by configName and version
//		...starting from dataOffset
//
//	Note: if starting version is -1 start from mock-up
void ConfigurationGUISupervisor::handleCreateConfigurationXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr, const std::string& configName, ConfigurationVersion version,
		bool makeTemporary, const std::string& data, const int& dataOffset,
		const std::string& author, const std::string& comment,
		bool sourceTableAsIs, bool lookForEquivalent)
try
{
	//__SUP_COUT__ << "handleCreateConfigurationXML: " << configName << " version: " << version
	//		<< " dataOffset: " << dataOffset << std::endl;

	//__SUP_COUT__ << "data: " << data << std::endl;

	//create temporary version from starting version
	if(!version.isInvalid()) //if not using mock-up, make sure starting version is loaded
	{
		try
		{
			cfgMgr->getVersionedConfigurationByName(configName,version);
		}
		catch(...)
		{
			//force to mockup
			version = ConfigurationVersion();
		}
	}

	ConfigurationBase* config = cfgMgr->getConfigurationByName(configName);

	//check that the source version has the right number of columns
	//	if there is a mismatch, start from mockup
	if(!version.isInvalid()) //if not using mock-up, then the starting version is the active one
	{
		//compare active to mockup column counts
		if(config->getViewP()->getDataColumnSize() !=
				config->getMockupViewP()->getNumberOfColumns() ||
				config->getViewP()->getSourceColumnMismatch() != 0)
		{
			__SUP_COUT__ << "config->getViewP()->getNumberOfColumns() " << config->getViewP()->getNumberOfColumns() << std::endl;
			__SUP_COUT__ << "config->getMockupViewP()->getNumberOfColumns() " << config->getMockupViewP()->getNumberOfColumns()  << std::endl;
			__SUP_COUT__ << "config->getViewP()->getSourceColumnMismatch() " << config->getViewP()->getSourceColumnMismatch() << std::endl;
			__SUP_COUT_INFO__ << "Source view v" << version <<
					" has a mismatch in the number of columns, so using mockup as source." << std::endl;
			version = ConfigurationVersion(); //invalid = mockup
		}
	}

	//create a temporary version from the source version
	ConfigurationVersion temporaryVersion = config->createTemporaryView(version);

	__SUP_COUT__ << "\t\ttemporaryVersion: " << temporaryVersion << std::endl;

	ConfigurationView* cfgView = config->getTemporaryView(temporaryVersion);

	int retVal;
	try
	{
		//returns -1 on error that data was unchanged
		retVal = sourceTableAsIs?0:cfgView->fillFromCSV(data,dataOffset,author);
		cfgView->setURIEncodedComment(comment);
		__SUP_COUT__ << "Table comment was set to:\n\t" << cfgView->getComment() << std::endl;

	}
	catch(...) //erase temporary view before re-throwing error
	{
		__SUP_COUT__ << "Caught error while editing. Erasing temporary version." << std::endl;
		config->eraseView(temporaryVersion);
		throw;
	}

	//Note: be careful with any further table operations at this point..
	//	must catch errors and erase temporary version on failure.

	//only consider it an error if source version was persistent version
	//	allow it if source version is temporary and we are making a persistent version now
	//	also, allow it if version tracking is off.
	if(retVal < 0 &&
			(!version.isTemporaryVersion() || makeTemporary) &&
			ConfigurationInterface::isVersionTrackingEnabled()
			)
	{
		if(!version.isInvalid() && //if source version was mockup, then consider it attempt to create a blank table
				!version.isScratchVersion()) //if source version was scratch, then consider it attempt to make it persistent
		{
			__SUP_SS__ << "No rows were modified! No reason to fill a view with same content." << std::endl;
			__SUP_COUT_ERR__ << "\n" << ss.str();
			//delete temporaryVersion
			config->eraseView(temporaryVersion);
			throw std::runtime_error(ss.str());
		}
		else if(version.isInvalid())
			__SUP_COUT__ << "This was interpreted as an attempt to create a blank table." << std::endl;
		else if(version.isScratchVersion())
			__SUP_COUT__ << "This was interpreted as an attempt to make a persistent version of the scratch table." << std::endl;
		else
		{__SUP_SS__; throw std::runtime_error(ss.str()+"impossible!");}
	}
	else if(retVal < 0 &&
			(version.isTemporaryVersion() && !makeTemporary))
	{
		__SUP_COUT__ << "Allowing the static data because this is converting from temporary to persistent version." << std::endl;
	}
	else if(retVal < 0 &&
			!ConfigurationInterface::isVersionTrackingEnabled())
	{
		__SUP_COUT__ << "Allowing the static data because version tracking is OFF." << std::endl;
	}
	else if(retVal < 0)
	{
		__SUP_SS__ << "This should not be possible! Fatal error." << std::endl;
		//delete temporaryVersion
		config->eraseView(temporaryVersion);
		throw std::runtime_error(ss.str());
	}

	//note: if sourceTableAsIs, accept equivalent versions
	saveModifiedVersionXML(xmlOut,cfgMgr,configName,version,makeTemporary,
			config,temporaryVersion,false /*ignoreDuplicates*/,
			lookForEquivalent || sourceTableAsIs /*lookForEquivalent*/);
}
catch(std::runtime_error& e)
{
	__SUP_COUT__ << "Error detected!\n\n " << e.what() << std::endl;
	xmlOut.addTextElementToData("Error", "Error saving new view!\n " +
			std::string(e.what()));
}
catch(...)
{
	__SUP_COUT__ << "Error detected!\n\n "<< std::endl;
	xmlOut.addTextElementToData("Error", "Error saving new view! ");
}


//========================================================================================================================
//	refreshUserSession
//		Finds/creates the active user session based on username&  actionSessionIndex
//
//		Returns a configurationMangager instance dedictated to the user.
//		This configurationManager will have at least empty instances of all base configurations (no null pointers)
//		and will load the backbone configurations to specified backboneVersion
//
//		If backboneVersion is -1, then latest, and backboneVersion passed by reference will be updated
ConfigurationManagerRW* ConfigurationGUISupervisor::refreshUserSession(std::string username,
		uint64_t activeSessionIndex, bool refresh)
//, ConfigurationVersion& backboneVersion)
{
	activeSessionIndex = 0; //make session by username for now! (may never want to change back)

	std::stringstream ssMapKey;
	ssMapKey << username << ":" << activeSessionIndex;
	std::string mapKey = ssMapKey.str();
	__SUP_COUT__ << "Config Session: " << mapKey << " ... out of size: " << userConfigurationManagers_.size() << std::endl;

	time_t now = time(0);

	//create new config mgr if not one for active session index
	if(userConfigurationManagers_.find(mapKey) == userConfigurationManagers_.end())
	{
		__SUP_COUT_INFO__ << "Creating new Configuration Manager." << std::endl;
		userConfigurationManagers_[mapKey] = new ConfigurationManagerRW(username);

		//update configuration info for each new configuration manager
		//	IMPORTANTLY this also fills all configuration manager pointers with instances,
		//	so we are not dealing with changing pointers later on
		userConfigurationManagers_[mapKey]->getAllConfigurationInfo(true);	//load empty instance of everything important
	}
	else if(userLastUseTime_.find(mapKey) == userLastUseTime_.end())
	{
		__SUP_SS__ << "Fatal error managing userLastUseTime_!" << std::endl;
		__SUP_COUT_ERR__ << "\n" << ss.str();
		throw std::runtime_error(ss.str());
	}
	else if(refresh || (now - userLastUseTime_[mapKey]) >
	CONFIGURATION_MANAGER_REFRESH_THRESHOLD) //check if should refresh all config info
	{
		__SUP_COUT_INFO__ << "Refreshing all configuration info." << std::endl;
		userConfigurationManagers_[mapKey]->getAllConfigurationInfo(true);
	}

	//load backbone configurations always based on backboneVersion
	//if backboneVersion is -1, then latest
	//backboneVersion = 0;// userConfigurationManagers_[mapKey]->loadConfigurationBackbone(backboneVersion);

	//update active sessionIndex last use time
	userLastUseTime_[mapKey] = now;

	//check for stale sessions and remove them (so config user maps do not grow forever)
	for (std::map<std::string, time_t> ::iterator it=userLastUseTime_.begin(); it!=userLastUseTime_.end(); ++it)
		if(now - it->second > CONFIGURATION_MANAGER_EXPIRATION_TIME) // expired!
		{
			__SUP_COUT__ << now << ":" << it->second << " = " << now - it->second << std::endl;
			delete userConfigurationManagers_[it->first]; //call destructor
			if(!(userConfigurationManagers_.erase(it->first)))	//erase by key
			{
				__SUP_SS__ << "Fatal error erasing configuration manager by key!" << std::endl;
				__SUP_COUT_ERR__ << "\n" << ss.str();
				throw std::runtime_error(ss.str());
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
//			Append a "bumped" system key to name
//			Save based on list of configuration name/ConfigurationVersion
//
//		configList parameter is comma separated configuration name and version
//
//		Note: if version of -1 (INVALID/MOCKUP) is given and there are no other existing table versions...
//			a new table version is generated using the mockup table.
//
//		Table Version Alias Handling:
//			Allow table versions to be specified as an alias with ALIAS: preamble. Aliased versions
//			will be translated according to the active backbone at activation time.
//
//
void ConfigurationGUISupervisor::handleCreateConfigurationGroupXML(
		HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr, const std::string& groupName,
		const std::string& configList, bool allowDuplicates, bool ignoreWarnings,
		const std::string& groupComment, bool lookForEquivalent)
try
{
	__SUP_COUT__ << "handleCreateConfigurationGroupXML \n";

	xmlOut.addTextElementToData("AttemptedNewGroupName",groupName);

	//make sure not using partial tables or anything weird when creating the group
	//	so start from scratch and load backbone
	const std::map<std::string, ConfigurationInfo>& allCfgInfo = cfgMgr->getAllConfigurationInfo(true);
	cfgMgr->loadConfigurationBackbone();

	std::map<std::string /*tableName*/,
		std::map<std::string /*aliasName*/,
			ConfigurationVersion /*version*/> > versionAliases =
					cfgMgr->getVersionAliases();
	for(const auto& aliases:versionAliases)
		for(const auto& alias:aliases.second)
		__SUP_COUT__ << aliases.first << " " << alias.first << " " << alias.second << std::endl;

	std::map<std::string /*name*/, ConfigurationVersion /*version*/> groupMembers;
	std::map<std::string /*name*/, std::string /*alias*/> groupAliases;

	std::string name, versionStr, alias;
	ConfigurationVersion version;
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
			__SUP_SS__ << "Incomplete Configuration-Version pair!" << std::endl;
			__SUP_COUT_ERR__ << "\n" << ss.str();
			xmlOut.addTextElementToData("Error", ss.str());
			return;
		}

		versionStr = configList.substr(i,c-i);
		i = c+1;
		c = configList.find(',',i);

		//__SUP_COUT__ << "name: " << name << std::endl;
		//__SUP_COUT__ << "versionStr: " << versionStr << std::endl;

		//check if version is an alias and convert
		if(versionStr.find(ConfigurationManager::ALIAS_VERSION_PREAMBLE) == 0)
		{
			alias = versionStr.substr(
					ConfigurationManager::ALIAS_VERSION_PREAMBLE.size());

			__SUP_COUT__ << "Found alias " << name << " " << versionStr << __E__;

			//convert alias to version
			if(versionAliases.find(name) != versionAliases.end() &&
					versionAliases[name].find(alias) != versionAliases[name].end())
			{

				version = versionAliases[name][alias];
				__SUP_COUT__ << "version alias '" << alias <<
						"'translated to: " << version << std::endl;

				groupAliases[name] = alias;
			}
			else
			{
				__SUP_SS__ << "version alias '" << versionStr.substr(
						ConfigurationManager::ALIAS_VERSION_PREAMBLE.size()) <<
								"' was not found in active version aliases!" << std::endl;
				__SUP_COUT_ERR__ << "\n" << ss.str();
				xmlOut.addTextElementToData("Error",
						ss.str());
				return;
			}
		}
		else
			version = ConfigurationVersion(versionStr);

		if(version.isTemporaryVersion())
		{
			__SUP_SS__ << "Groups can not be created using temporary member tables. " <<
					"Table member '" << name << "' with temporary version '" << version <<
					"' is illegal." << std::endl;
			xmlOut.addTextElementToData("Error", ss.str());
			return;
		}

		// enforce that table exists
		if(allCfgInfo.find(name) == allCfgInfo.end())
		{
			__SUP_SS__ << "Groups can not be created using mock-up member tables of undefined tables. " <<
					"Table member '" << name << "' is not defined." << std::endl;
			xmlOut.addTextElementToData("Error", ss.str());
			return;
		}

		if(version.isMockupVersion())
		{
			//if mockup, then generate a new persistent version to use based on mockup
			ConfigurationBase* config =	cfgMgr->getConfigurationByName(name);
			//create a temporary version from the mockup as source version
			ConfigurationVersion temporaryVersion = config->createTemporaryView();
			__SUP_COUT__ << "\t\ttemporaryVersion: " << temporaryVersion << std::endl;


			//if other versions exist check for another mockup, and use that instead
			__SUP_COUT__ << "Creating version from mock-up for name: " << name <<
					" inputVersionStr: " << versionStr << std::endl;

			//set table comment
			config->getTemporaryView(temporaryVersion)->setComment("Auto-generated from mock-up.");

			//finish off the version creation
			version = saveModifiedVersionXML(xmlOut,cfgMgr,name,
					ConfigurationVersion() /*original source is mockup*/,
					false /* makeTemporary */,
					config,
					temporaryVersion /*temporary modified version*/,
					false /*ignore duplicates*/,
					true /*look for equivalent*/);

			__SUP_COUT__ << "Using mockup version: " << version << std::endl;

		}

		//__SUP_COUT__ << "version: " << version << std::endl;
		groupMembers[name] = version;
	} //end member verification loop

	__COUTV__(StringMacros::mapToString(groupAliases));

	if(!allowDuplicates)
	{
		__SUP_COUT__ << "Checking for duplicate groups..." << std::endl;
		ConfigurationGroupKey foundKey =
				cfgMgr->findConfigurationGroup(groupName,groupMembers,groupAliases);

		if(!foundKey.isInvalid())
		{
			//return found equivalent key
			xmlOut.addTextElementToData("ConfigurationGroupName",groupName);
			xmlOut.addTextElementToData("ConfigurationGroupKey",foundKey.toString());

			if(lookForEquivalent)
			{
				__SUP_COUT__ << "Found equivalent group key (" << foundKey << ") for " <<
						groupName << "." << std::endl;
				//allow this equivalent group to be the response without an error
				xmlOut.addTextElementToData("foundEquivalentKey","1"); //indicator

				//insert get configuration info
				handleGetConfigurationGroupXML(xmlOut,cfgMgr,groupName,foundKey);
				return;
			}
			else //treat as error, if not looking for equivalent
			{
				__SUP_COUT__ << "Treating duplicate group as error." << std::endl;
				__SUP_SS__ << ("Failed to create configuration group: " + groupName +
						". It is a duplicate of an existing group key (" + foundKey.toString() + ")");
				__SUP_COUT_ERR__ << ss.str() << std::endl;
				xmlOut.addTextElementToData("Error",ss.str());
				return;
			}
		}

		__SUP_COUT__ << "Check for duplicate groups complete." << std::endl;
	}

	//check the group for errors before creating group
	try
	{
		cfgMgr->loadMemberMap(groupMembers);

		std::string accumulateErrors = "";
		for(auto& groupMemberPair:groupMembers)
		{
			ConfigurationView* cfgViewPtr =
					cfgMgr->getConfigurationByName(groupMemberPair.first)->getViewP();
			if(cfgViewPtr->getDataColumnSize() !=
					cfgViewPtr->getNumberOfColumns() ||
					cfgViewPtr->getSourceColumnMismatch() != 0) //check for column size mismatch
			{
				__SUP_SS__ << "\n\nThere were errors found in loading a member table " <<
						groupMemberPair.first << ":v" << cfgViewPtr->getVersion() <<
						". Please see the details below:\n\n" <<
						"The source column size was found to be " <<
						cfgViewPtr->getDataColumnSize() <<
						", and the current number of columns for this table is " <<
						cfgViewPtr->getNumberOfColumns() << ". This resulted in a count of " <<
						cfgViewPtr->getSourceColumnMismatch() << " source column mismatches, and a count of " <<
						cfgViewPtr->getSourceColumnMissing() << " table entries missing in " <<
						cfgViewPtr->getNumberOfRows() << " row(s) of data." <<
						std::endl;

				const std::set<std::string> srcColNames = cfgViewPtr->getSourceColumnNames();
				ss << "\n\nSource column names were as follows:\n";
				char index = 'a';
				for(auto& srcColName:srcColNames)
					ss << "\n\t" <<index++ << ". " <<  srcColName;
				ss << std::endl;

				std::set<std::string> destColNames = cfgViewPtr->getColumnStorageNames();
				ss << "\n\nCurrent table column names are as follows:\n";
				index = 'a';
				for(auto& destColName:destColNames)
					ss << "\n\t" << index++ << ". " << destColName;
				ss << std::endl;

				__SUP_COUT_ERR__ << "\n" << ss.str();
				xmlOut.addTextElementToData("Error", ss.str());
				return;
			}
		}
	}
	catch(std::runtime_error& e)
	{
		__SUP_SS__ << "Failed to create config group: " << groupName <<
				".\nThere were problems loading the chosen members:\n\n" <<
				e.what() << std::endl;
		__SUP_COUT_ERR__ << "\n" << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
		return;
	}
	catch(...)
	{
		__SUP_SS__ << "Failed to create config group: " << groupName << std::endl;
		__SUP_COUT_ERR__ << "\n" << ss.str();
		xmlOut.addTextElementToData("Error", ss.str());
		return;
	}

	//check the tree for warnings before creating group
	std::string accumulateTreeErrs;
	cfgMgr->getChildren(&groupMembers,&accumulateTreeErrs);
	if(accumulateTreeErrs != "" )
	{
		__SUP_COUT_WARN__ << "\n" << accumulateTreeErrs << std::endl;
		if(!ignoreWarnings)
		{
			xmlOut.addTextElementToData("TreeErrors",
					accumulateTreeErrs);
			return;
		}
	}

	ConfigurationGroupKey newKey;
	try
	{
		__COUT__ << "Saving new group..." << __E__;
		newKey = cfgMgr->saveNewConfigurationGroup(groupName,groupMembers,
				groupComment,&groupAliases);
	}
	catch(std::runtime_error& e)
	{
		__SUP_COUT_ERR__ << "Failed to create config group: " << groupName << std::endl;
		__SUP_COUT_ERR__ << "\n\n" << e.what() << std::endl;
		xmlOut.addTextElementToData("Error", "Failed to create configuration group: " + groupName +
				".\n\n" + e.what());
		return;
	}
	catch(...)
	{
		__SUP_COUT_ERR__ << "Failed to create config group: " << groupName << std::endl;
		xmlOut.addTextElementToData("Error", "Failed to create configuration group: " + groupName);
		return;
	}

	//insert get configuration info
	__COUT__ << "Loading new group..." << __E__;
	handleGetConfigurationGroupXML(xmlOut,cfgMgr,groupName,newKey);
}
catch(std::runtime_error& e)
{
	__SUP_COUT__ << "Error detected!\n\n " << e.what() << std::endl;
	xmlOut.addTextElementToData("Error", "Error saving group! " + std::string(e.what()));
}
catch(...)
{
	__SUP_COUT__ << "Error detected!\n\n "<< std::endl;
	xmlOut.addTextElementToData("Error", "Error saving group! ");
}


//========================================================================================================================
//	handleDeleteConfigurationInfoXML
//
//		return nothing except Error in xmlOut
//
void ConfigurationGUISupervisor::handleDeleteConfigurationInfoXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr,
		std::string& configName)
{

	if ( 0 == rename( (CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT).c_str() ,
			(CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT + ".unused").c_str() ) )
		__SUP_COUT_INFO__ << ( "Table Info File successfully renamed: " +
				(CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT + ".unused")) << std::endl;
	else
	{
		__SUP_COUT_ERR__ << ( "Error renaming file to " +
				(CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT + ".unused")) << std::endl;

		xmlOut.addTextElementToData("Error",
				( "Error renaming Table Info File to " +
						(CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT + ".unused")));
		return;
	}

	//reload all with refresh to remove new configuration
	cfgMgr->getAllConfigurationInfo(true);
}

//========================================================================================================================
//	handleSaveConfigurationInfoXML
//
//		write new info file for configName based CSV column info
//			data="type,name,dataType;type,name,dataType;..."
//		return resulting handleGetConfigurationXML mock-up view
//
void ConfigurationGUISupervisor::handleSaveConfigurationInfoXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr,
		std::string& configName, const std::string& data,
		const std::string& tableDescription, const std::string& columnChoicesCSV,
		bool allowOverwrite)
{
	//create all caps name and validate
	//	only allow alpha-numeric names with Configuration at end
	std::string capsName;
	try
	{
		capsName = ConfigurationBase::convertToCaps(configName, true);
	}
	catch(std::runtime_error& e)
	{	//error! non-alpha
		xmlOut.addTextElementToData("Error",e.what());
		return;
	}

	if(!allowOverwrite)
	{
		FILE* fp = fopen((CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT).c_str(), "r");
		if(fp)
		{
			fclose(fp);
			xmlOut.addTextElementToData("ConfigurationName",
					configName);
			xmlOut.addTextElementToData("OverwriteError",
					"1");
			xmlOut.addTextElementToData("Error",
					"File already exists! ('" +
					(CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT) +
					"')");
			return;
		}
	}

	__SUP_COUT__ << "capsName=" << capsName << std::endl;
	__SUP_COUT__ << "configName=" << configName << std::endl;
	__SUP_COUT__ << "tableDescription=" << tableDescription << std::endl;
	__SUP_COUT__ << "columnChoicesCSV=" << columnChoicesCSV << std::endl;

	//create preview string to validate column info before write to file
	std::stringstream outss;

	outss << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n";
	outss << "\t<ROOT xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"ConfigurationInfo.xsd\">\n";
	outss << "\t\t<CONFIGURATION Name=\"" <<
			configName	<< "\">\n";
	outss << "\t\t\t<VIEW Name=\"" << capsName <<
			"\" Type=\"File,Database,DatabaseTest\" Description=\"" <<
			tableDescription << "\">\n";

	//each column is represented by 3 fields
	//	- type, name, dataType
	int i = 0; //use to parse data std::string
	int j = data.find(',',i); //find next field delimiter
	int k = data.find(';',i); //find next col delimiter


	std::istringstream columnChoicesISS(columnChoicesCSV);
	std::string columnChoicesString;
	std::string columnType;

	while(k != (int)(std::string::npos))
	{
		//type
		columnType = data.substr(i,j-i);
		outss << "\t\t\t\t<COLUMN Type=\"";
		outss << columnType;

		i=j+1;
		j = data.find(',',i); //find next field delimiter

		//name and storage name
		outss << "\" \t Name=\"";
		capsName = data.substr(i,j-i); //not caps yet
		outss << capsName;
		outss << "\" \t StorageName=\"";

		try
		{
			outss << ConfigurationBase::convertToCaps(capsName); //now caps
		}
		catch(std::runtime_error& e)
		{	//error! non-alpha
			xmlOut.addTextElementToData("Error", std::string("For column name '") +
					data.substr(i,j-i) + "' - " + e.what());
			return;
		}

		i=j+1;
		j = data.find(',',i); //find next field delimiter

		//data type
		outss << "\" \t	DataType=\"";
		outss << data.substr(i,k-i);


		//fixed data choices for ViewColumnInfo::TYPE_FIXED_CHOICE_DATA
		getline(columnChoicesISS, columnChoicesString, ';');
		//__SUP_COUT__ << "columnChoicesString = " << columnChoicesString << std::endl;
		outss << "\" \t	DataChoices=\"";
		outss << columnChoicesString;


		//end column info
		outss << "\"/>\n";

		i = k+1;
		j = data.find(',',i); //find next field delimiter
		k = data.find(';',i); //find new col delimiter
	}

	outss << "\t\t\t</VIEW>\n";
	outss << "\t\t</CONFIGURATION>\n";
	outss << "\t</ROOT>\n";

	__SUP_COUT__ << outss.str() << std::endl;

	FILE* fp = fopen((CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT).c_str(), "w");
	if(!fp)
	{
		xmlOut.addTextElementToData("Error", "Failed to open destination Configuration Info file:" +
				(CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT));
		return;
	}

	fprintf(fp,"%s",outss.str().c_str());
	fclose(fp);

	//reload all config info with refresh AND reset to pick up possibly new config
	// check for errors related to this configName
	std::string accumulatedErrors = "";
	cfgMgr->getAllConfigurationInfo(true,&accumulatedErrors,configName);

	//if errors associated with this config name stop and report
	if(accumulatedErrors != "")
	{
		__SUP_SS__ << ("The new version of the '" + configName + "' table column info was saved, however errors were detected reading back the configuration '" +
				configName +
				"' after the save attempt:\n\n" + accumulatedErrors) << std::endl;

		__SUP_COUT_ERR__ << ss.str() << std::endl;
		xmlOut.addTextElementToData("Error", ss.str());

		//if error detected reading back then move the saved configuration info to .unused
		// // This was disabled by RAR on 11/4/2016.. (just keep broken info files)
		// // ... especially since now there is a Delete button
		if(0)
		{
			//configuration info is illegal so report error, and disable file


			//if error detected //move file to ".unused"
			if ( 0 == rename( (CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT).c_str() ,
					(CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT + ".unused").c_str() ) )
				__SUP_COUT_INFO__ << ( "File successfully renamed: " +
						(CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT + ".unused")) << std::endl;
			else


				__SUP_COUT_ERR__ << ( "Error renaming file to " +
						(CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT + ".unused")) << std::endl;

			//reload all with refresh to remove new configuration
			cfgMgr->getAllConfigurationInfo(true);
		}
		return;
	}

	//return the new configuration info
	handleGetConfigurationXML(xmlOut,cfgMgr,configName,ConfigurationVersion());

	//debug all table column info
	//FIXME -- possibly remove this debug feature in future
	const std::map<std::string, ConfigurationInfo>& allCfgInfo = cfgMgr->getAllConfigurationInfo();

	//give a print out of currently illegal configuration column info
	__SUP_COUT_INFO__ << "Looking for errors in all configuration column info..." << std::endl;
	for(const auto& cfgInfo: allCfgInfo)
	{
		try
		{
			cfgMgr->getConfigurationByName(cfgInfo.first)->getMockupViewP()->init();
		}
		catch(std::runtime_error& e)
		{
			__SUP_COUT_WARN__ << "\n\n##############################################\n" <<
					"Error identified in column info of configuration '" <<
					cfgInfo.first << "':\n\n" <<
					e.what() << "\n\n" << std::endl;
		}
	}
}

//========================================================================================================================
//	handleSetGroupAliasInBackboneXML
//		open current backbone
//		modify GroupAliases
//		save as new version of groupAliases
//		return new version of groupAliases
//
//Note: very similar to ConfigurationGUISupervisor::handleSetVersionAliasInBackboneXML
void ConfigurationGUISupervisor::handleSetGroupAliasInBackboneXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr, const std::string& groupAlias,
		const std::string& groupName, ConfigurationGroupKey groupKey,
		const std::string& author)
try
{
	cfgMgr->loadConfigurationBackbone();
	std::map<std::string, ConfigurationVersion> activeVersions = cfgMgr->getActiveVersions();

	const std::string groupAliasesTableName = ConfigurationManager::GROUP_ALIASES_CONFIG_NAME;
	if(activeVersions.find(groupAliasesTableName) == activeVersions.end())
	{
		__SUP_SS__ << "Active version of " << groupAliasesTableName << " missing!" << std::endl;
		xmlOut.addTextElementToData("Error", ss.str());
		return;
	}

	//put all old backbone versions in xmlOut
	const std::set<std::string> backboneMembers = cfgMgr->getBackboneMemberNames();
	for(auto& memberName: backboneMembers)
	{
		__SUP_COUT__ << "activeVersions[\"" << memberName << "\"]=" <<
				activeVersions[memberName] << std::endl;

		xmlOut.addTextElementToData("oldBackboneName",
				memberName);
		xmlOut.addTextElementToData("oldBackboneVersion",
				activeVersions[memberName].toString());
	}

	//make a temporary version from active view
	//modify the chosen groupAlias row
	//save as new version

	ConfigurationBase* config = cfgMgr->getConfigurationByName(groupAliasesTableName);
	ConfigurationVersion originalVersion = activeVersions[groupAliasesTableName];
	ConfigurationVersion temporaryVersion = config->createTemporaryView(originalVersion);

	__SUP_COUT__ << "\t\t temporaryVersion: " << temporaryVersion << std::endl;
	bool isDifferent = false;

	try
	{
		ConfigurationView* configView = config->getTemporaryView(temporaryVersion);

		unsigned int col = configView->findCol("GroupKeyAlias");

		//only make a new version if we are changing compared to active backbone


		unsigned int row = -1;
		//find groupAlias row
		try	{ row = configView->findRow(col,groupAlias); }
		catch (...) {}
		if(row == (unsigned int)-1) //if row not found then add a row
		{
			isDifferent = true;
			row = configView->addRow();

			//set all columns in new row
			col = configView->findCol("CommentDescription");
			configView->setValue("This Group Alias was automatically setup by the server." ,
					row, col);
			col = configView->findCol("GroupKeyAlias");
			configView->setValue(groupAlias, row, col);
		}

		__SUP_COUT__ << "\t\t row: " << row << std::endl;

		col = configView->findCol("GroupName");

		__SUP_COUT__ << "\t\t groupName: " << groupName << " vs " <<
				configView->getDataView()[row][col] << std::endl;
		if(groupName != configView->getDataView()[row][col])
		{
			configView->setValue(groupName, row, col);
			isDifferent = true;
		}

		col = configView->findCol("GroupKey");
		__SUP_COUT__ << "\t\t groupKey: " << groupKey << " vs " <<
				configView->getDataView()[row][col] << std::endl;
		if(groupKey.toString() != configView->getDataView()[row][col])
		{
			configView->setValue(groupKey.toString(), row, col);
			isDifferent = true;
		}

		if(isDifferent)	//set author/time of new version if different
		{
			configView->setValue(author, row, configView->findCol("Author"));
			configView->setValue(time(0), row, configView->findCol("RecordInsertionTime"));
		}
	}
	catch(...)
	{
		__SUP_COUT_ERR__ << "Error editing Group Alias view!" << std::endl;

		//delete temporaryVersion
		config->eraseView(temporaryVersion);
		throw;
	}


	ConfigurationVersion newAssignedVersion;
	if(isDifferent)	//make new version if different
	{
		__SUP_COUT__ << "\t\t**************************** Save as new table version" << std::endl;

		//newAssignedVersion =
		//		cfgMgr->saveNewConfiguration(groupAliasesTableName,temporaryVersion);

		//save or find equivalent

		newAssignedVersion = saveModifiedVersionXML(xmlOut,cfgMgr,
				config->getConfigurationName(),originalVersion, false /*makeTemporary*/,
				config,temporaryVersion,false /*ignoreDuplicates*/,true /*lookForEquivalent*/);

	}
	else	//use existing version
	{
		__SUP_COUT__ << "\t\t**************************** Using the existing table version" << std::endl;

		//delete temporaryVersion
		config->eraseView(temporaryVersion);
		newAssignedVersion = activeVersions[groupAliasesTableName];

		xmlOut.addTextElementToData("savedName", groupAliasesTableName);
		xmlOut.addTextElementToData("savedVersion", newAssignedVersion.toString());
	}

	__SUP_COUT__ << "\t\t newAssignedVersion: " << newAssignedVersion << std::endl;


}
catch(std::runtime_error& e)
{
	__SUP_COUT_ERR__ << "Error detected!\n\n " << e.what() << std::endl;
	xmlOut.addTextElementToData("Error", "Error saving new Group Alias view!\n " +
			std::string(e.what()));
}
catch(...)
{
	__SUP_COUT_ERR__ << "Error detected!\n\n "<< std::endl;
	xmlOut.addTextElementToData("Error", "Error saving new Group Alias view! ");
}

//========================================================================================================================
//	handleSetVersionAliasInBackboneXML
//		open current backbone
//		modify VersionAliases
//		save as new version of VersionAliases
//		return new version of VersionAliases
//
//Note: very similar to ConfigurationGUISupervisor::handleSetGroupAliasInBackboneXML
void ConfigurationGUISupervisor::handleSetVersionAliasInBackboneXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr,
		const std::string& versionAlias,
		const std::string& configName, ConfigurationVersion version, const std::string& author)
try
{
	cfgMgr->loadConfigurationBackbone();
	std::map<std::string, ConfigurationVersion> activeVersions = cfgMgr->getActiveVersions();

	const std::string versionAliasesTableName = ConfigurationManager::VERSION_ALIASES_CONFIG_NAME;
	if(activeVersions.find(versionAliasesTableName) == activeVersions.end())
	{
		__SUP_SS__ << "Active version of " << versionAliasesTableName << " missing!" << std::endl;
		xmlOut.addTextElementToData("Error", ss.str());
		return;
	}

	//put all old backbone versions in xmlOut
	const std::set<std::string> backboneMembers = cfgMgr->getBackboneMemberNames();
	for(auto& memberName: backboneMembers)
	{
		__SUP_COUT__ << "activeVersions[\"" << memberName << "\"]=" <<
				activeVersions[memberName] << std::endl;

		xmlOut.addTextElementToData("oldBackboneName",
				memberName);
		xmlOut.addTextElementToData("oldBackboneVersion",
				activeVersions[memberName].toString());
	}

	//make a temporary version from active view
	//modify the chosen versionAlias row
	//save as new version

	ConfigurationBase* config = cfgMgr->getConfigurationByName(versionAliasesTableName);
	ConfigurationVersion originalVersion = activeVersions[versionAliasesTableName];
	ConfigurationVersion temporaryVersion = config->createTemporaryView(originalVersion);

	__SUP_COUT__ << "\t\t temporaryVersion: " << temporaryVersion << std::endl;

	bool isDifferent = false;

	try
	{
		ConfigurationView* configView = config->getTemporaryView(temporaryVersion);

		unsigned int col;
		unsigned int col2 = configView->findCol("VersionAlias");
		unsigned int col3 = configView->findCol("ConfigurationName");

		//only make a new version if we are changing compared to active backbone


		unsigned int row = -1;
		//find configName, versionAlias pair
		//	NOTE: only accept the first pair, repeats are ignored.
		try	{
			unsigned int tmpRow = -1;
			do
			{	//start looking from beyond last find
				tmpRow = configView->findRow(col3,configName,tmpRow+1);
			} while (configView->getDataView()[tmpRow][col2] != versionAlias);
			//at this point the first pair was found! (else exception was thrown)
			row = tmpRow;
		}
		catch (...) {}
		if(row == (unsigned int)-1) //if row not found then add a row
		{
			isDifferent = true;
			row = configView->addRow();

			//set all columns in new row
			col = configView->findCol("CommentDescription");
			configView->setValue(std::string("Entry was added by server in ") +
					"ConfigurationGUISupervisor::setVersionAliasInActiveBackbone()." ,
					row, col);

			col = configView->findCol("VersionAliasUID");
			configView->setValue(configName.substr(0,configName.rfind("Configuration")) +
					versionAlias, row, col);

			configView->setValue(versionAlias, row, col2);
			configView->setValue(configName, row, col3);
		}

		__SUP_COUT__ << "\t\t row: " << row << std::endl;

		col = configView->findCol("Version");
		__SUP_COUT__ << "\t\t version: " << version << " vs " <<
				configView->getDataView()[row][col] << std::endl;
		if(version.toString() != configView->getDataView()[row][col])
		{
			configView->setValue(version.toString(), row, col);
			isDifferent = true;
		}

		if(isDifferent)	//set author/time of new version if different
		{
			configView->setValue(author, row, configView->findCol("Author"));
			configView->setValue(time(0), row, configView->findCol("RecordInsertionTime"));
		}
	}
	catch(...)
	{
		__SUP_COUT_ERR__ << "Error editing Version Alias view!" << std::endl;

		//delete temporaryVersion
		config->eraseView(temporaryVersion);
		throw;
	}

	ConfigurationVersion newAssignedVersion;
	if(isDifferent)	//make new version if different
	{
		__SUP_COUT__ << "\t\t**************************** Save as new table version" << std::endl;

		//newAssignedVersion  =
		//		cfgMgr->saveNewConfiguration(versionAliasesTableName,temporaryVersion);

		newAssignedVersion = saveModifiedVersionXML(xmlOut,cfgMgr,
						config->getConfigurationName(),originalVersion, false /*makeTemporary*/,
						config,temporaryVersion,false /*ignoreDuplicates*/,true /*lookForEquivalent*/);
	}
	else	//use existing version
	{
		__SUP_COUT__ << "\t\t**************************** Using existing table version" << std::endl;

		//delete temporaryVersion
		config->eraseView(temporaryVersion);
		newAssignedVersion = activeVersions[versionAliasesTableName];

		xmlOut.addTextElementToData("savedAlias", versionAliasesTableName);
		xmlOut.addTextElementToData("savedVersion", newAssignedVersion.toString());
	}

	__SUP_COUT__ << "\t\t newAssignedVersion: " << newAssignedVersion << std::endl;
}
catch(std::runtime_error& e)
{
	__SUP_COUT__ << "Error detected!\n\n " << e.what() << std::endl;
	xmlOut.addTextElementToData("Error", "Error saving new Version Alias view!\n " +
			std::string(e.what()));
}
catch(...)
{
	__SUP_COUT__ << "Error detected!\n\n "<< std::endl;
	xmlOut.addTextElementToData("Error", "Error saving new Version Alias view! ");
}

//========================================================================================================================
//	handleAliasGroupMembersInBackboneXML
//		open current backbone
//		modify VersionAliases
//		save as new version of VersionAliases
//		return new version of VersionAliases
void ConfigurationGUISupervisor::handleAliasGroupMembersInBackboneXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr,
		const std::string& versionAlias,
		const std::string& groupName, ConfigurationGroupKey groupKey, const std::string& author)
try
{
	cfgMgr->loadConfigurationBackbone();
	std::map<std::string, ConfigurationVersion> activeVersions = cfgMgr->getActiveVersions();

	const std::string versionAliasesTableName = ConfigurationManager::VERSION_ALIASES_CONFIG_NAME;
	if(activeVersions.find(versionAliasesTableName) == activeVersions.end())
	{
		__SUP_SS__ << "Active version of " << versionAliasesTableName << " missing!" << std::endl;
		xmlOut.addTextElementToData("Error", ss.str());
		return;
	}

	//put all old backbone versions in xmlOut
	const std::set<std::string> backboneMembers = cfgMgr->getBackboneMemberNames();
	for(auto& memberName: backboneMembers)
	{
		__SUP_COUT__ << "activeVersions[\"" << memberName << "\"]=" <<
				activeVersions[memberName] << std::endl;

		xmlOut.addTextElementToData("oldBackboneName",
				memberName);
		xmlOut.addTextElementToData("oldBackboneVersion",
				activeVersions[memberName].toString());
	}

	//make a temporary version from active view
	//modify the chosen versionAlias row
	//save as new version


	ConfigurationBase* config = cfgMgr->getConfigurationByName(versionAliasesTableName);
	ConfigurationVersion temporaryVersion = config->
			createTemporaryView(activeVersions[versionAliasesTableName]);

	__SUP_COUT__ << "\t\t temporaryVersion: " << temporaryVersion << std::endl;

	ConfigurationView* configView = config->getTemporaryView(temporaryVersion);


	//only make a new version if we are changing compared to active backbone
	bool isDifferent = false;

	//get member names and versions
	std::map<std::string /*name*/, ConfigurationVersion /*version*/> memberMap;
	try
	{
		cfgMgr->loadConfigurationGroup(groupName,groupKey,
				false /*doActivate*/,&memberMap,0,0,0,0,0, //defaults
				true /*doNotLoadMember*/);
	}
	catch(...)
	{
		xmlOut.addTextElementToData("Error","Configuration group \"" +
				ConfigurationGroupKey::getFullGroupString(groupName,groupKey) +
				"\" can not be retrieved!");
		return;
	}

	unsigned int col;
	unsigned int col2 = configView->findCol("VersionAlias");
	unsigned int col3 = configView->findCol("ConfigurationName");

	for(auto& memberPair:memberMap)
	{
		bool thisMemberIsDifferent = false;
		unsigned int row = -1;

		__SUP_COUT__ << "Adding alias for " << memberPair.first <<
				"_v" << memberPair.second << std::endl;

		//find configName, versionAlias pair
		//	NOTE: only accept the first pair, repeats are ignored.
		try	{
			unsigned int tmpRow = -1;
			do
			{	//start looking from beyond last find
				tmpRow = configView->findRow(col3,memberPair.first,tmpRow+1);
				__SUP_COUT__ << configView->getDataView()[tmpRow][col2] << std::endl;
			} while (configView->getDataView()[tmpRow][col2] != versionAlias);
			//at this point the first pair was found! (else exception was thrown)
			row = tmpRow;
		}
		catch (...) {}
		if(row == (unsigned int)-1) //if row not found then add a row
		{
			thisMemberIsDifferent = true;
			row = configView->addRow();

			//set all columns in new row
			col = configView->findCol("CommentDescription");
			configView->setValue(std::string("Entry was added by server in ") +
					"ConfigurationGUISupervisor::setVersionAliasInActiveBackbone()." ,
					row, col);

			col = configView->findCol("VersionAliasId");
			configView->setValue(memberPair.first.substr(
					0,memberPair.first.rfind("Configuration")) +
					versionAlias, row, col);

			configView->setValue(versionAlias, row, col2);
			configView->setValue(memberPair.first, row, col3);
		}

		__SUP_COUT__ << "\t\t row: " << row << std::endl;

		col = configView->findCol("Version");
		__SUP_COUT__ << "\t\t col: " << col << std::endl;
		__SUP_COUT__ << "\t\t version: " << memberPair.second << " vs " <<
				configView->getDataView()[row][col] << std::endl;
		if(memberPair.second.toString() !=
				configView->getDataView()[row][col])
		{
			configView->setValue(memberPair.second.toString(), row, col);
			thisMemberIsDifferent = true;
		}

		if(thisMemberIsDifferent)	//change author and time if row is different
		{
			configView->setValue(author, row, configView->findCol("Author"));
			configView->setValue(time(0), row, configView->findCol("RecordInsertionTime"));
		}

		if(thisMemberIsDifferent)
			isDifferent = true;
	}

	//configView->print();

	ConfigurationVersion newAssignedVersion;
	if(isDifferent)	//make new version if different
	{
		__SUP_COUT__ << "\t\t**************************** Save as new table version" << std::endl;

		newAssignedVersion =
				cfgMgr->saveNewConfiguration(versionAliasesTableName,temporaryVersion);
	}
	else	//use existing version
	{
		__SUP_COUT__ << "\t\t**************************** Using existing table version" << std::endl;

		//delete temporaryVersion
		config->eraseView(temporaryVersion);
		newAssignedVersion = activeVersions[versionAliasesTableName];
	}

	xmlOut.addTextElementToData("savedAlias", versionAliasesTableName);
	xmlOut.addTextElementToData("savedVersion", newAssignedVersion.toString());
	__SUP_COUT__ << "\t\t newAssignedVersion: " << newAssignedVersion << std::endl;
}
catch(std::runtime_error& e)
{
	__SUP_COUT__ << "Error detected!\n\n " << e.what() << std::endl;
	xmlOut.addTextElementToData("Error", "Error saving new Version Alias view!\n " +
			std::string(e.what()));
}
catch(...)
{
	__SUP_COUT__ << "Error detected!\n\n "<< std::endl;
	xmlOut.addTextElementToData("Error", "Error saving new Version Alias view! ");
}

//========================================================================================================================
//	handleGroupAliasesXML
//
//		return aliases and backbone groupAlias table version
//
//		return this information:
//		<backbone groupTableName=xxx version=xxx>
//		<group alias=xxx name=xxx key=xxx comment=xxx>
//		<group alias=xxx name=xxx key=xxx comment=xxx>
//		...
//
void ConfigurationGUISupervisor::handleGroupAliasesXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr)
{
	cfgMgr->loadConfigurationBackbone();
	std::map<std::string, ConfigurationVersion> activeVersions = cfgMgr->getActiveVersions();

	std::string groupAliasesTableName = ConfigurationManager::GROUP_ALIASES_CONFIG_NAME;
	if(activeVersions.find(groupAliasesTableName) == activeVersions.end())
	{
		__SUP_SS__ << "\nActive version of " << groupAliasesTableName << " missing! " <<
				groupAliasesTableName << " is a required member of the Backbone configuration group." <<
				"\n\nLikely you need to activate a valid Backbone group." <<
				std::endl;
		xmlOut.addTextElementToData("Error", ss.str());
		return;
	}
	__SUP_COUT__ << "activeVersions[\"" << groupAliasesTableName << "\"]=" <<
			activeVersions[groupAliasesTableName] << std::endl;
	xmlOut.addTextElementToData("GroupAliasesConfigurationName",
			groupAliasesTableName);
	xmlOut.addTextElementToData("GroupAliasesConfigurationVersion",
			activeVersions[groupAliasesTableName].toString());

	std::vector<std::pair<std::string,ConfigurationTree> > aliasNodePairs =
			cfgMgr->getNode(groupAliasesTableName).getChildren();

	std::string groupName, groupKey, groupComment, groupType;
	for(auto& aliasNodePair:aliasNodePairs)
	{
		groupName = aliasNodePair.second.getNode("GroupName").getValueAsString();
		groupKey = aliasNodePair.second.getNode("GroupKey").getValueAsString();

		xmlOut.addTextElementToData("GroupAlias", aliasNodePair.first);
		xmlOut.addTextElementToData("GroupName", groupName);
		xmlOut.addTextElementToData("GroupKey", groupKey);
		xmlOut.addTextElementToData("AliasComment",
				aliasNodePair.second.getNode("CommentDescription").getValueAsString());

		//get group comment
		groupComment = ""; //clear just in case failure
		groupType = "Invalid";
		try
		{
			cfgMgr->loadConfigurationGroup(groupName,ConfigurationGroupKey(groupKey),
					0,0,0,0,&groupComment,0,0, //mostly defaults
					true /*doNotLoadMembers*/,&groupType);
		}
		catch(...)
		{
			__SUP_COUT_WARN__ << "Failed to load group '" << groupName << "(" << groupKey <<
					")' to extract group comment and type." << std::endl;
		}
		xmlOut.addTextElementToData("GroupComment", groupComment);
		xmlOut.addTextElementToData("GroupType", groupType);
	}
}

//========================================================================================================================
//	handleConfigurationVersionAliasesXML
//
//		return version aliases and backbone versionAliases table version
//
//		return this information:
//		<backbone aliasTableName=xxx version=xxx>
//		<version alias=xxx name=xxx version=xxx comment=xxx>
//		<version alias=xxx name=xxx version=xxx comment=xxx>
//		...
//
void ConfigurationGUISupervisor::handleVersionAliasesXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr)
{
	cfgMgr->loadConfigurationBackbone();
	std::map<std::string, ConfigurationVersion> activeVersions = cfgMgr->getActiveVersions();

	std::string versionAliasesTableName = ConfigurationManager::VERSION_ALIASES_CONFIG_NAME;
	if(activeVersions.find(versionAliasesTableName) == activeVersions.end())
	{
		__SUP_SS__ << "Active version of VersionAliases  missing!" <<
				"Make sure you have a valid active Backbone Group." << std::endl;
		xmlOut.addTextElementToData("Error", ss.str());
		return;
	}
	__SUP_COUT__ << "activeVersions[\"" << versionAliasesTableName << "\"]=" <<
			activeVersions[versionAliasesTableName] << std::endl;
	xmlOut.addTextElementToData("VersionAliasesVersion",
			activeVersions[versionAliasesTableName].toString());

	std::vector<std::pair<std::string,ConfigurationTree> > aliasNodePairs =
			cfgMgr->getNode(versionAliasesTableName).getChildren();

	for(auto& aliasNodePair:aliasNodePairs)
	{
		//note : these are column names in the versionAliasesTableName table
		//VersionAlias, ConfigurationName, Version, CommentDescription
		xmlOut.addTextElementToData("VersionAlias",
				aliasNodePair.second.getNode("VersionAlias").getValueAsString());
		xmlOut.addTextElementToData("ConfigurationName",
				aliasNodePair.second.getNode("ConfigurationName").getValueAsString());
		xmlOut.addTextElementToData("Version",
				aliasNodePair.second.getNode("Version").getValueAsString());
		xmlOut.addTextElementToData("Comment",
				aliasNodePair.second.getNode("CommentDescription").getValueAsString());
	}
}

//========================================================================================================================
//	handleGetConfigurationGroupTypeXML
//
//		return this information based on member table list
//		<ConfigurationGroupType value=xxx>
//
void ConfigurationGUISupervisor::handleGetConfigurationGroupTypeXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr,
		const std::string& configList)
{

	std::map<std::string /*name*/, ConfigurationVersion /*version*/> memberMap;
	std::string name, versionStr;
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
			__SUP_SS__ << "Incomplete Configuration-Version pair!" << std::endl;
			__SUP_COUT_ERR__ << "\n" << ss.str();
			xmlOut.addTextElementToData("Error", ss.str());
			return;
		}

		versionStr = configList.substr(i,c-i);
		i = c+1;
		c = configList.find(',',i);

		memberMap[name] = ConfigurationVersion(versionStr);
	}

	std::string groupTypeString = "";
	//try to determine type, dont report errors, just mark "Invalid"
	try
	{
		//determine the type configuration group
		groupTypeString = cfgMgr->getTypeNameOfGroup(memberMap);
		xmlOut.addTextElementToData("ConfigurationGroupType", groupTypeString);
	}
	catch(std::runtime_error& e)
	{
		__SUP_SS__ << "Configuration group has invalid type! " << e.what() << std::endl;
		__SUP_COUT__ << "\n" << ss.str();
		groupTypeString = "Invalid";
		xmlOut.addTextElementToData("ConfigurationGroupType", groupTypeString);
	}
	catch(...)
	{
		__SUP_SS__ << "Configuration group has invalid type! " << std::endl;
		__SUP_COUT__ << "\n" << ss.str();
		groupTypeString = "Invalid";
		xmlOut.addTextElementToData("ConfigurationGroupType", groupTypeString);
	}
}

//========================================================================================================================
//	handleConfigurationGroupsXML
//
//		if returnMembers then
//			return type, comment and members
//		else just name and key
//
//		return this information
//		<group name=xxx key=xxx>
//			<config name=xxx version=xxx />
//			<config name=xxx version=xxx />
//			...
//		</group>
//		<group name=xxx key=xxx>...</group>
//		...
//
void ConfigurationGUISupervisor::handleConfigurationGroupsXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr, bool returnMembers)
{
	DOMElement* parentEl;

	//get all group info from cache (if no cache, get from interface)

	if(!cfgMgr->getAllGroupInfo().size()) //empty cache is strange, attempt to get from interface
	{
		__SUP_COUT__ << "Cache is empty? Attempting to regenerate." << __E__;
		cfgMgr->getAllConfigurationInfo(true /*refresh*/);
	}

	const std::map<std::string, GroupInfo>&	allGroupInfo = cfgMgr->getAllGroupInfo();

//	ConfigurationInterface* theInterface = cfgMgr->getConfigurationInterface();
//	std::set<std::string /*name*/>  configGroups = theInterface->getAllConfigurationGroupNames();
//	__SUP_COUT__ << "Number of Config groups: " << configGroups.size() << std::endl;
//
//	ConfigurationGroupKey groupKey;
//	std::string groupName;
//
//	std::map<std::string /*groupName*/,std::set<ConfigurationGroupKey> > allGroupsWithKeys;
//	for(auto& groupString:configGroups)
//	{
//		ConfigurationGroupKey::getGroupNameAndKey(groupString,groupName,groupKey);
//		allGroupsWithKeys[groupName].emplace(groupKey);
//
//		//__SUP_COUT__ << "Config group " << groupString << " := " << groupName <<
//		//"(" << groupKey << ")" << std::endl;
//	}

	ConfigurationGroupKey groupKey;
	std::string groupName;
	std::string groupString, groupTypeString, groupComment, groupCreationTime, groupAuthor;
	for(auto& groupInfo:allGroupInfo)
	{
		groupName = groupInfo.first;
		if(groupInfo.second.keys_.size() == 0)
		{
			__SUP_COUT__ << "Group name '" << groupName << "' found, but no keys so ignoring." << __E__;
			continue;
		}

		groupKey = *(groupInfo.second.keys_.rbegin());

		xmlOut.addTextElementToData("ConfigurationGroupName", groupName);
		xmlOut.addTextElementToData("ConfigurationGroupKey", groupKey.toString());

		//trusting the cache!
		xmlOut.addTextElementToData("ConfigurationGroupType", groupInfo.second.latestKeyGroupTypeString_);
		xmlOut.addTextElementToData("ConfigurationGroupComment", groupInfo.second.latestKeyGroupComment_);
		xmlOut.addTextElementToData("ConfigurationGroupAuthor", groupInfo.second.latestKeyGroupAuthor_);
		xmlOut.addTextElementToData("ConfigurationGroupCreationTime", groupInfo.second.latestKeyGroupCreationTime_);

		if(returnMembers)
		{

			//groupTypeString = "Invalid";
			//groupComment = ""; //clear just in case failure

			//groupString = ConfigurationGroupKey::getFullGroupString(groupName,groupKey);

			//__SUP_COUT__ << "Latest Config group " << groupString << " := " << groupName <<
			//		"(" << groupKey << ")" << std::endl;

 			parentEl = xmlOut.addTextElementToData("ConfigurationGroupMembers", "");
//
//			std::map<std::string /*name*/, ConfigurationVersion /*version*/> memberMap;
//			//try to determine type, dont report errors, just mark "Invalid"
//			try
//			{
//				//determine the type configuration group
//				memberMap = cfgMgr->loadConfigurationGroup(groupName,groupKey,
//						0,0,0,&groupComment,0,0, //mostly defaults
//						true /*doNotLoadMembers*/,&groupTypeString);
//				//groupTypeString = cfgMgr->getTypeNameOfGroup(memberMap);
//				xmlOut.addTextElementToData("ConfigurationGroupType", groupTypeString);
//				xmlOut.addTextElementToData("ConfigurationGroupComment", groupComment);
//			}
//			catch(std::runtime_error& e)
//			{
//				__SUP_SS__ << "Configuration group \"" + groupString +
//						"\" has invalid type! " + e.what() << std::endl;
//				__SUP_COUT__ << "\n" << ss.str();
//				groupTypeString = "Invalid";
//				xmlOut.addTextElementToData("ConfigurationGroupType", groupTypeString);
//				xmlOut.addTextElementToData("ConfigurationGroupComment", groupComment);
//				continue;
//			}
//			catch(...)
//			{
//				__SUP_SS__ << "Configuration group \"" + groupString +
//						"\" has invalid type! " << std::endl;
//				__SUP_COUT__ << "\n" << ss.str();
//				groupTypeString = "Invalid";
//				xmlOut.addTextElementToData("ConfigurationGroupType", groupTypeString);
//				xmlOut.addTextElementToData("ConfigurationGroupComment", groupComment);
//				continue;
//			}

			for(auto& memberPair:groupInfo.second.latestKeyMemberMap_)
			{
				//__SUP_COUT__ << "\tMember config " << memberPair.first << ":" << memberPair.second << std::endl;
				xmlOut.addTextElementToParent("MemberName", memberPair.first, parentEl);
				xmlOut.addTextElementToParent("MemberVersion", memberPair.second.toString(), parentEl);
			}
		} // end if returnMembers


		//add other group keys to xml for this group name
		//	but just empty members (not displayed anyway)
		for(auto& keyInSet: groupInfo.second.keys_)
		{
			if(keyInSet == groupKey) continue; //skip the lastest
			xmlOut.addTextElementToData("ConfigurationGroupName", groupName);
			xmlOut.addTextElementToData("ConfigurationGroupKey", keyInSet.toString());

			//assume latest in cache reflects others (for speed)
			xmlOut.addTextElementToData("ConfigurationGroupType", groupInfo.second.latestKeyGroupTypeString_);
			xmlOut.addTextElementToData("ConfigurationGroupComment", groupInfo.second.latestKeyGroupComment_);
			xmlOut.addTextElementToData("ConfigurationGroupAuthor", groupInfo.second.latestKeyGroupAuthor_);
			xmlOut.addTextElementToData("ConfigurationGroupCreationTime", groupInfo.second.latestKeyGroupCreationTime_);

			if(returnMembers)
			{
				xmlOut.addTextElementToData("ConfigurationGroupMembers", "");

				//TODO -- make loadingHistoricalInfo an input parameter
				bool loadingHistoricalInfo = false;
				if(loadingHistoricalInfo)
				{


					groupComment = ""; //clear just in case failure
					try
					{
						cfgMgr->loadConfigurationGroup(groupName,keyInSet,
								0,0,0,0,&groupComment,0,0, //mostly defaults
								true /*doNotLoadMembers*/,&groupTypeString);
					}
					catch(...)
					{
						groupTypeString = "Invalid";
						__SUP_COUT_WARN__ << "Failed to load group '" << groupName << "(" << keyInSet <<
								")' to extract group comment and type." << std::endl;
					}

					xmlOut.addTextElementToData("ConfigurationGroupType", groupTypeString);
					xmlOut.addTextElementToData("ConfigurationGroupComment", groupComment);
					xmlOut.addTextElementToData("ConfigurationGroupAuthor", groupAuthor);
					xmlOut.addTextElementToData("ConfigurationGroupCreationTime", groupCreationTime);
				}
			}

		} //end other key loop
	} //end primary group loop

}

//========================================================================================================================
//	handleConfigurationsXML
//
//		return this information
//		<subconfiguration name=xxx>
//			<version key=xxx />
//			<version key=xxx />
//			...
//		</subconfiguration>
//		<subconfiguration name=xxx>...</subconfiguration>
//		...
//
void ConfigurationGUISupervisor::handleConfigurationsXML(HttpXmlDocument& xmlOut,
		ConfigurationManagerRW* cfgMgr,
		bool allowIllegalColumns)
{
	DOMElement* parentEl;

	std::string accumulatedErrors = "";
	const std::map<std::string, ConfigurationInfo>& allCfgInfo = cfgMgr->getAllConfigurationInfo(
			allowIllegalColumns,allowIllegalColumns?&accumulatedErrors:0); //if allowIllegalColumns, then also refresh
	std::map<std::string, ConfigurationInfo>::const_iterator it = allCfgInfo.begin();

	__SUP_COUT__ << "# of configuration tables found: " << allCfgInfo.size() << std::endl;

	std::map<std::string,std::map<std::string,ConfigurationVersion> > versionAliases =
			cfgMgr->getVersionAliases();

	__SUP_COUT__ << "# of configuration tables w/aliases: " << versionAliases.size() << std::endl;


	while(it != allCfgInfo.end())
	{
		//for each subconfiguration name
		//get existing version keys

		//__SUP_COUT__ << "Name: " << it->first << " - #ofVersions: " << it->second.versions_.size() << std::endl;

		//add system subconfiguration name
		xmlOut.addTextElementToData("ConfigurationName", it->first);
		parentEl = xmlOut.addTextElementToData("ConfigurationVersions", "");

		//include aliases for this table (if the versions exist)
		if(versionAliases.find(it->first) != versionAliases.end())
			for (auto& aliasVersion:versionAliases[it->first])
				if(it->second.versions_.find(aliasVersion.second) !=
						it->second.versions_.end())
				//if(aliasVersion.first != ConfigurationManager::SCRATCH_VERSION_ALIAS) //NOT NEEDED IF SCRATCH IS ALWAYS ALIAS
					xmlOut.addTextElementToParent("Version",
						ConfigurationManager::ALIAS_VERSION_PREAMBLE + aliasVersion.first,
						parentEl);
//				else //NOT NEEDED IF SCRATCH IS ALWAYS ALIAS
//					__SUP_COUT_ERR__ << "Alias for table " << it->first << " is a reserved alias '" <<
//						ConfigurationManager::SCRATCH_VERSION_ALIAS << "' - this is illegal." << std::endl;

//		//if scratch version exists, add an alias for it /NOT NEEDED IF SCRATCH IS ALWAYS ALIAS
//		if(it->second.versions_.find(ConfigurationVersion(ConfigurationVersion::SCRATCH)) !=
//				it->second.versions_.end())
//			xmlOut.addTextElementToParent("Version",
//					ConfigurationManager::ALIAS_VERSION_PREAMBLE + ConfigurationManager::SCRATCH_VERSION_ALIAS,
//					parentEl);

		//get all table versions for the current table
		//	except skip scratch version
		for (auto& version:it->second.versions_)
			if(!version.isScratchVersion())
				xmlOut.addTextElementToParent("Version", version.toString(), parentEl);

		++it;
	}

	if(accumulatedErrors != "")
		xmlOut.addTextElementToData("Error", std::string("Column errors were allowed for this request, ") +
				"but please note the following errors:\n" + accumulatedErrors);
}

//========================================================================================================================
//	testXDAQContext
//		test activation of context group
void ConfigurationGUISupervisor::testXDAQContext()
{

	try
	{
		__SUP_COUT__ << "Attempting test activation of the context group." << std::endl;
		ConfigurationManager cfgMgr; //create instance to activate saved groups
	}
	catch(...)
	{
		__SUP_COUT_WARN__ << "The test activation of the context group failed. Ignoring." << std::endl;
	}
	return;

	/////////////////////////////////
	//below has been used for debugging.


	//behave like a user
	//start with top level xdaq context
	//	then add and delete rows proof-of-concept
	//export xml xdaq config file

	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//behave like a new user
	//
	//ConfigurationManagerRW cfgMgrInst("ExampleUser");
		//
	//ConfigurationManagerRW* cfgMgr =& cfgMgrInst;

	//std::map<std::string, ConfigurationVersion> groupMembers;
	//groupMembers["DesktopIcon"] = ConfigurationVersion(2);
//	cfgMgr->saveNewConfigurationGroup("test",
//			groupMembers, "test comment");

	//	//
	//	const std::map<std::string, ConfigurationInfo>& allCfgInfo = cfgMgr->getAllConfigurationInfo(true);
	//	__SUP_COUT__ << "allCfgInfo.size() = " << allCfgInfo.size() << std::endl;
	//	for(auto& mapIt : allCfgInfo)
	//	{
	//		__SUP_COUT__ << "Config Name: " << mapIt.first << std::endl;
	//		__SUP_COUT__ << "\t\tExisting Versions: " << mapIt.second.versions_.size() << std::endl;
	//
	//		//get version key for the current system subconfiguration key
	//		for (auto& v:mapIt.second.versions_)
	//		{
	//			__SUP_COUT__ << "\t\t" << v << std::endl;
	//		}
	//	}

	//testXDAQContext just a test bed for navigating the new config tree
	//cfgMgr->testXDAQContext();




	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////


}






