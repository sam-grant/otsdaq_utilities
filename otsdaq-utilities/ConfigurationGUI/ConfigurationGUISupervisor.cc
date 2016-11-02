#include "otsdaq-utilities/ConfigurationGUI/ConfigurationGUISupervisor.h"
#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/Macros/CoutHeaderMacros.h"
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"


//#include "otsdaq-core/ConfigurationPluginDataFormats/ConfigurationAliases.h"
//#include "otsdaq-core/ConfigurationPluginDataFormats/Configurations.h"
//#include "otsdaq-core/ConfigurationPluginDataFormats/DetectorConfiguration.h"
//#include "otsdaq-core/ConfigurationPluginDataFormats/DataManagerConfiguration.h"
//#include "otsdaq-core/ConfigurationPluginDataFormats/DefaultConfigurations.h"
//#include "otsdaq-core/ConfigurationPluginDataFormats/VersionAliases.h"

//#include "otsdaq-core/ConfigurationDataFormats/ROCDACs.h"


#include <xdaq/NamespaceURI.h>

#include <iostream>
#include <map>
#include <utility>



using namespace ots;

#undef 	__MF_SUBJECT__
#define __MF_SUBJECT__ "CfgGUI"

#define CONFIG_INFO_PATH	std::string(getenv("CONFIGURATION_INFO_PATH")) + "/"
#define CONFIG_INFO_EXT		std::string("Info.xml")


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

	__MOUT__ << "comment/uncomment here for debugging Configuration!" << std::endl;

	//FIXME
	__MOUT__ << "To prove the concept..." << std::endl;

	return;

	testXDAQContext(); //test new config

	return;

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
	cgicc::Cgicc cgi(in);
	std::string configName = CgiDataUtilities::getData(cgi,"tableEditor"); //from GET
	if(configName != "")
		*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/ConfigurationTableEditor.html?urn=" <<
		this->getApplicationDescriptor()->getLocalId() <<"'></frameset></html>";
	else
		*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/ConfigurationGUI.html?urn=" <<
		this->getApplicationDescriptor()->getLocalId() <<"'></frameset></html>";
}

//========================================================================================================================
void ConfigurationGUISupervisor::request(xgi::Input * in, xgi::Output * out)
throw (xgi::exception::Exception)
{
	cgicc::Cgicc cgi(in);
	std::string Command = CgiDataUtilities::getOrPostData(cgi,"RequestType");//from GET or POST

	__MOUT__ << "Command " << Command << " files: " << cgi.getFiles().size() << std::endl;

	//Commands
	//	saveConfigurationInfo
	//	getGroupAliases
	//	setGroupAliasInActiveBackbone
	//	getVersionAliases
	//	getConfigurationGroups
	//	getConfigurations
	//	getContextMemberNames
	//	getBackboneMemberNames
	//	getSpecificConfigurationGroup
	//	saveNewConfigurationGroup
	//	getSpecificConfiguration
	//	saveSpecificConfiguration
	//	getTreeView
	//	activateConfigGroup
	//	getActiveConfigGroups
	//  copyViewToCurrentColumns

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
	std::string 	refresh = CgiDataUtilities::getData(cgi,"refresh"); 	//from GET
	__MOUT__ << "refresh: " << refresh << std::endl;
	//refresh to reload from info files and db (maintains temporary views!)
	ConfigurationManagerRW* cfgMgr = refreshUserSession(userName, activeSessionIndex,
			(refresh == "1") );

	if(Command == "saveConfigurationInfo")
	{
		std::string configName = CgiDataUtilities::getData(cgi,"configName"); //from GET
		std::string columnCSV = CgiDataUtilities::postData(cgi,"columnCSV"); //from POST
		std::string allowOverwrite = CgiDataUtilities::getData(cgi,"allowOverwrite"); //from GET

		__MOUT__ << "configName: " << configName << std::endl;
		__MOUT__ << "columnCSV: " << columnCSV << std::endl;
		__MOUT__ << "allowOverwrite: " << allowOverwrite << std::endl;

		if(!theRemoteWebUsers_.isWizardMode(theSupervisorsConfiguration_))
		{
			__SS__ << "Improper permissions for saving configuration info." << std::endl;
			xmldoc.addTextElementToData("Error", ss.str());
		}
		else
			handleSaveConfigurationInfoXML(xmldoc,cfgMgr,configName,columnCSV,allowOverwrite=="1");
	}
	else if(Command == "getGroupAliases")
	{
		handleGroupAliasesXML(xmldoc,cfgMgr);
	}
	else if(Command == "setGroupAliasInActiveBackbone")
	{
		std::string groupAlias = CgiDataUtilities::getData(cgi,"groupAlias"); //from GET
		std::string groupName = CgiDataUtilities::getData(cgi,"groupName"); //from GET
		std::string groupKey = CgiDataUtilities::getData(cgi,"groupKey"); //from GET

		__MOUT__ << "groupAlias: " << groupAlias << std::endl;
		__MOUT__ << "groupName: " << groupName << std::endl;
		__MOUT__ << "groupKey: " << groupKey << std::endl;

		handleSetGroupAliasInBackboneXML(xmldoc,cfgMgr,groupAlias,groupName,
				ConfigurationGroupKey(groupKey),userName);
	}
	else if(Command == "getVersionAliases")
	{
		handleVersionAliasesXML(xmldoc,cfgMgr);
	}
	else if(Command == "getConfigurationGroups")
	{
		handleConfigurationGroupsXML(xmldoc,cfgMgr);
	}
	else if(Command == "getConfigurations")
	{
		std::string allowIllegalColumns = CgiDataUtilities::getData(cgi,"allowIllegalColumns"); //from GET

		__MOUT__ << "allowIllegalColumns: " << allowIllegalColumns << std::endl;

		handleConfigurationsXML(xmldoc,cfgMgr, allowIllegalColumns == "1");
	}
	else if(Command == "getContextMemberNames")
	{
		std::set<std::string> members = cfgMgr->getContextMemberNames();

		for(auto& member:members)
			xmldoc.addTextElementToData("ContextMember", member);
	}
	else if(Command == "getBackboneMemberNames")
	{
		std::set<std::string> members = cfgMgr->getBackboneMemberNames();

		for(auto& member:members)
			xmldoc.addTextElementToData("BackboneMember", member);
	}
	else if(Command == "getSpecificConfigurationGroup")
	{
		std::string 	groupName = CgiDataUtilities::getData(cgi,"groupName"); 	//from GET
		std::string 	groupKey = CgiDataUtilities::getData(cgi,"groupKey"); 	//from GET

		__MOUT__ << "groupName: " << groupName << std::endl;
		__MOUT__ << "groupKey: " << groupKey << std::endl;

		handleGetConfigurationGroupXML(xmldoc,cfgMgr,groupName,ConfigurationGroupKey(groupKey));
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

		std::string 	allowIllegalColumns = CgiDataUtilities::getData(cgi,"allowIllegalColumns"); //from GET
		__MOUT__ << "allowIllegalColumns: " << allowIllegalColumns << std::endl;

		__MOUT__ << "getSpecificConfiguration: " << configName << " versionStr: " << versionStr
				<< " chunkSize: " << chunkSize << " dataOffset: " << dataOffset << std::endl;

		ConfigurationVersion version;
		std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
		if(versionStr == "" && //take latest version if no version specified
				allCfgInfo[configName].versions_.size())
			version = *(allCfgInfo[configName].versions_.rbegin());
		else					//else take specified version
			version = atoi(versionStr.c_str());

		__MOUT__ << " version: " << version << std::endl;

		handleGetConfigurationXML(xmldoc,cfgMgr,configName,ConfigurationVersion(version),allowIllegalColumns=="1");
	}
	else if(Command == "saveSpecificConfiguration")
	{
		std::string 	configName 	= CgiDataUtilities::getData	    (cgi,"configName"); //from GET
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
				temporary,data,dataOffset,userName);
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
		handleFillTreeViewXML(xmldoc,cfgMgr,configGroup,ConfigurationGroupKey(configGroupKey),
				startPath,depth);
	}
	else if(Command == "activateConfigGroup")
	{
		std::string 	groupName 	= CgiDataUtilities::getData(cgi,"groupName");
		std::string 	groupKey 	= CgiDataUtilities::getData(cgi,"groupKey");

		__MOUT__ << "Activating config: " << groupName <<
				"(" << groupKey << ")" << std::endl;

		try
		{
			cfgMgr->activateConfigurationGroup(groupName, ConfigurationGroupKey(groupKey));
		}
		catch(std::runtime_error& e)
		{
			__MOUT__ << "Error detected!\n\n " << e.what() << std::endl;
			xmldoc.addTextElementToData("Error", "Error activating config group '" +
					groupName +	"(" + groupKey + ")" + "'! " +
					std::string(e.what()));
		}
		catch(cet::exception& e)
		{
			__MOUT__ << "Error detected!\n\n " << e.what() << std::endl;
			xmldoc.addTextElementToData("Error", "Error activating config group '" +
					groupName +	"(" + groupKey + ")" + "'! " +
					std::string(e.what()));
		}
		catch(...)
		{
			__MOUT__ << "Error detected!" << std::endl;
			throw; //unexpected exception!
		}
	}
	else if(Command == "getActiveConfigGroups"); //do nothing, since they are always returned
	else if(Command == "copyViewToCurrentColumns")
	{
		std::string 	configName 		= CgiDataUtilities::getData(cgi,"configName"); //from GET
		std::string 	sourceVersion 	= CgiDataUtilities::getData(cgi,"sourceVersion");

		__MOUT__ << "configName: " << configName << std::endl;
		__MOUT__ << "sourceVersion: " << sourceVersion << std::endl;
		__MOUT__ << "userName: " << userName << std::endl;

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

			__MOUT__ << "New temporary version = " << newTemporaryVersion << std::endl;
		}
		catch(std::runtime_error &e)
		{
			__MOUT__ << "Error detected!\n\n " << e.what() << std::endl;
			xmldoc.addTextElementToData("Error", "Error copying view from '" +
					configName +	"_v" + sourceVersion + "'! " +
					std::string(e.what()));
		}
		catch(...)
		{
			__MOUT__ << "Error detected!\n\n " << std::endl;
			xmldoc.addTextElementToData("Error", "Error copying view from '" +
					configName +	"_v" + sourceVersion + "'! ");
		}

		handleGetConfigurationXML(xmldoc,cfgMgr,configName,newTemporaryVersion);
	}
	else
		__MOUT__ << "Command request not recognized." << std::endl;

	__MOUT__ << "Wrapping up..." << std::endl;

	//always add active config groups to xml response
	std::map<std::string /*type*/,
	std::pair<std::string /*groupName*/,
	ConfigurationGroupKey>> activeGroupMap =
			cfgMgr->getActiveGlobalConfiguration();

	for(auto &type:activeGroupMap)
	{
		xmldoc.addTextElementToData(type.first + "-ActiveGroupName",
				type.second.first);
		xmldoc.addTextElementToData(type.first + "-ActiveGroupKey",
				type.second.second.toString());
	}

	__MOUT__ << "Error field=" << xmldoc.getMatchingValue("Error") << std::endl;

	//return xml doc holding server response
	xmldoc.outputXmlDocument((std::ostringstream*) out, false, true); //true for debug printout
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
		const std::string &groupName, const ConfigurationGroupKey &groupKey,
		const std::string &startPath, int depth)
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
	xmldoc.addTextElementToData("configGroup", groupName);
	xmldoc.addTextElementToData("configGroupKey", groupKey.toString());

	try {
		std::map<std::string /*name*/, ConfigurationVersion /*version*/> memberMap =
				cfgMgr->loadConfigurationGroup(groupName,groupKey);

		//add all active configuration pairs to xmldoc
		std::map<std::string, ConfigurationVersion> allActivePairs = cfgMgr->getActiveVersions();
		for(auto &activePair: allActivePairs)
		{
			xmldoc.addTextElementToData("MemberName", activePair.first);
			xmldoc.addTextElementToData("MemberVersion", activePair.second.toString());
		}

		DOMElement* parentEl = xmldoc.addTextElementToData("tree", startPath);

		if(depth == 0) return; //already returned root node in itself

		std::map<std::string,ConfigurationTree> rootMap;

		if(startPath == "/") //then consider the configurationManager the root node
			rootMap = cfgMgr->getChildren(memberMap);
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

			xmldoc.addTextElementToParent("LinkConfigurationName", t.getConfigurationName(),
					parentEl);
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
void ConfigurationGUISupervisor::handleGetConfigurationGroupXML(HttpXmlDocument &xmldoc,
		ConfigurationManagerRW *cfgMgr, const std::string &groupName, ConfigurationGroupKey groupKey)
{
	char tmpIntStr[100];
	DOMElement *parentEl, *configEl;

	//steps:
	//	get specific group with key
	//		give member names and versions
	//		get all configuration groups to locate historical keys
	//	get all groups to find historical keys


	xmldoc.addTextElementToData("ConfigurationGroupName", groupName);
	xmldoc.addTextElementToData("ConfigurationGroupKey", groupKey.toString());
	parentEl = xmldoc.addTextElementToData("ConfigurationGroupMembers", "");

	//	get specific group with key
	std::map<std::string /*name*/, ConfigurationVersion /*version*/> memberMap;
	try
	{
		memberMap = cfgMgr->getConfigurationInterface()->getConfigurationGroupMembers(
				ConfigurationGroupKey::getFullGroupString(groupName,groupKey));
	}
	catch(...)
	{
		xmldoc.addTextElementToData("Error","Configuration group \"" +
				ConfigurationGroupKey::getFullGroupString(groupName,groupKey) +
				"\" can not be retrieved!");
		return;
	}


	__MOUT__ << "groupName=" << groupName << std::endl;
	__MOUT__ << "groupKey=" << groupKey << std::endl;

	std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();
	std::map<std::string, ConfigurationInfo>::const_iterator it;

	for(auto &memberPair:memberMap)
	{
		__MOUT__ << "\tMember config " << memberPair.first << ":" << memberPair.second << std::endl;
		xmldoc.addTextElementToParent("MemberName", memberPair.first, parentEl);
		configEl = xmldoc.addTextElementToParent("MemberVersion", memberPair.second.toString(), parentEl);

		it = allCfgInfo.find(memberPair.first);
		if(it == allCfgInfo.end())
		{
			xmldoc.addTextElementToData("Error","Configuration \"" +
					memberPair.first +
					"\" can not be retrieved!");
			return;
		}

		for (auto &version:it->second.versions_)
			if(version == memberPair.second) continue;
			else xmldoc.addTextElementToParent("ConfigurationExistingVersion", version.toString(), configEl);
	}

	std::set<std::string /*name+version*/> allGroups =
			cfgMgr->getConfigurationInterface()->getAllConfigurationGroupNames();
	std::string name;
	ConfigurationGroupKey key;
	for(auto &group: allGroups)
	{
		ConfigurationGroupKey::getGroupNameAndKey(group,name,key);
		//add all other keys for this groupName
		if(name == groupName && key != groupKey)
			xmldoc.addTextElementToData("HistoricalConfigurationGroupKey", key.toString());
	}

	return;
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
void ConfigurationGUISupervisor::handleGetConfigurationXML(HttpXmlDocument &xmldoc,
		ConfigurationManagerRW *cfgMgr, const std::string &configName,
		ConfigurationVersion version, bool allowIllegalColumns)
try
{
	char tmpIntStr[100];
	DOMElement* parentEl;

	std::string accumulatedErrors = "";
	std::map<std::string, ConfigurationInfo> allCfgInfo = //if allowIllegalColumns, then also refresh
			cfgMgr->getAllConfigurationInfo(allowIllegalColumns,
					allowIllegalColumns?&accumulatedErrors:0,configName); //filter errors by configName

	//send all config names along with
	//	and check for specific version
	for(auto &configPair:allCfgInfo)
	{
		xmldoc.addTextElementToData("ExistingConfigurationNames",
				configPair.first);
		if(configPair.first == configName && //check that version exists
				configPair.second.versions_.find(version) ==
						configPair.second.versions_.end())
		{
			__MOUT__ << "Version not found, so using mockup." << std::endl;
			version = ConfigurationVersion(); //use INVALID
		}
	}

	xmldoc.addTextElementToData("ConfigurationName", configName);	//table name

	//existing table versions
	parentEl = xmldoc.addTextElementToData("ConfigurationVersions", "");
	for (auto &v:allCfgInfo[configName.c_str()].versions_)
		xmldoc.addTextElementToParent("Version", v.toString(), parentEl);


	//table columns and then rows (from config view)

	//get view pointer
	ConfigurationView* cfgViewPtr;
	if(version.isInvalid()) //use mockup
	{
		cfgViewPtr = cfgMgr->getConfigurationByName(configName)->getMockupViewP();
	}
	else					//use view version
	{
		try
		{
			cfgViewPtr = cfgMgr->getVersionedConfigurationByName(configName,version)->getViewP();
		}
		catch(std::runtime_error &e) //default to mockup for fail-safe in GUI editor
		{
			__SS__ << "Failed to get configuration name: " << configName <<
					" and version: " << version <<
					"... defaulting to mockup! " <<
					std::endl;
			ss << "\n\n...Here is why it failed:\n\n" << e.what() << std::endl;

			std::cout << ss.str();
			version = ConfigurationVersion();
			cfgViewPtr = cfgMgr->getConfigurationByName(configName)->getMockupViewP();

			xmldoc.addTextElementToData("Error", "Error getting view! " + ss.str());
		}
		catch(...) //default to mockup for fail-safe in GUI editor
		{
			__SS__ << "Failed to get configuration name: " << configName <<
					" and version: " << version <<
					"... defaulting to mockup! " <<
					"(You may want to try again to see what was partially loaded into cache before failure. " <<
					"If you think, the failure is due to a column name change, " <<
					"you can also try to Copy the failing view to the new column names using " <<
					"'Copy and Move' functionality.)" <<
					std::endl;
			std::cout << ss.str();
			version = ConfigurationVersion();
			cfgViewPtr = cfgMgr->getConfigurationByName(configName)->getMockupViewP();

			xmldoc.addTextElementToData("Error", "Error getting view! " + ss.str());
		}
	}
	xmldoc.addTextElementToData("ConfigurationVersion", version.toString());	//table version

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

	//verify mockup columns after columns are posted to xmldoc
	if(version.isInvalid())
		cfgViewPtr->init();

	parentEl = xmldoc.addTextElementToData("CurrentVersionRows", "");

	for(int r=0;r<(int)cfgViewPtr->getNumberOfRows();++r)
	{
		//__MOUT__ << "\t\tRow " << r << ": "  << std::endl;

		sprintf(tmpIntStr,"%d",r);
		DOMElement* tmpParentEl = xmldoc.addTextElementToParent("Row", tmpIntStr, parentEl);

		for(int c=0;c<(int)cfgViewPtr->getNumberOfColumns();++c)
				xmldoc.addTextElementToParent("Entry", cfgViewPtr->getValueAsString(r,c), tmpParentEl);
	}

	if(accumulatedErrors != "") //add accumulated errors to xmldoc
		xmldoc.addTextElementToData("Error", std::string("Column errors were allowed for this request, ") +
				"but please note the following errors:\n" + accumulatedErrors);
}
catch(std::runtime_error &e)
{
	__MOUT__ << "Error detected!\n\n " << e.what() << std::endl;
	xmldoc.addTextElementToData("Error", "Error getting view! " + std::string(e.what()));
}
catch(...)
{
	__MOUT__ << "Error detected!\n\n "<< std::endl;
	xmldoc.addTextElementToData("Error", "Error getting view! ");
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
		bool makeTemporary, const std::string &data, const int &dataOffset, const std::string &author)
try
{
	//__MOUT__ << "handleCreateConfigurationXML: " << configName << " version: " << version
	//		<< " dataOffset: " << dataOffset << std::endl;

	//__MOUT__ << "data: " << data << std::endl;

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
	ConfigurationVersion temporaryVersion = cfgMgr->getConfigurationByName(configName)->
			createTemporaryView(version);

	__MOUT__ << "\t\ttemporaryVersion: " << temporaryVersion << std::endl;

	//returns -1 on error that data was unchanged
	int retVal = cfgMgr->getConfigurationByName(configName)->
			getTemporaryView(temporaryVersion)->fillFromCSV(data,dataOffset,author);

	//only consider it an error if source version was persistent version
	//	allow it if it is temporary and we are making a persistent version now
	if(retVal < 0 && (!version.isTemporaryVersion() || makeTemporary))
	{
		__SS__ << "No rows were modified! No reason to fill a view with same content." << std::endl;
		throw std::runtime_error(ss.str());
	}
	else
		__MOUT__ << "Allowing the static data because this is converting from temporary to persistent version" << std::endl;

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
		uint64_t activeSessionIndex, bool refresh)
//, ConfigurationVersion &backboneVersion)
{
	activeSessionIndex = 0; //make session by username for now! (may never want to change back)

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
	else if(refresh || now - userLastUseTime_[mapKey] >
	CONFIGURATION_MANAGER_REFRESH_THRESHOLD) //check if should refresh all config info
	{
		__MOUT_INFO__ << "Refreshing all configuration info." << std::endl;
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
//			Append a "bumped" system key to name
//			Save based on list of configuration name/ConfigurationVersion
//
//		configList parameter is comma separated configuration name and version
//
void ConfigurationGUISupervisor::handleCreateConfigurationGroupXML	(HttpXmlDocument &xmldoc,
		ConfigurationManagerRW *cfgMgr, const std::string &groupName,
		const std::string &configList, bool allowDuplicates)
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


	if(!allowDuplicates)
	{
		ConfigurationGroupKey foundKey =
				cfgMgr->findConfigurationGroup(groupName,groupMembers);
		if(!foundKey.isInvalid())
		{
			//return old keys
			xmldoc.addTextElementToData("ConfigurationGroupName",groupName);
			xmldoc.addTextElementToData("ConfigurationGroupKey",foundKey.toString());
			xmldoc.addTextElementToData("Error",
					"Failed to create configuration group: " + groupName +
					". It is a duplicate of a previous group key (" + foundKey.toString() + ")");
			return;
		}
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
		return;
	}
	catch(...)
	{
		__MOUT_ERR__ << "Failed to create config group: " << groupName << std::endl;
		xmldoc.addTextElementToData("Error", "Failed to create configuration group: " + groupName);
		return;
	}

	handleGetConfigurationGroupXML(xmldoc,cfgMgr,groupName,newKey);
}

//========================================================================================================================
//	handleSaveConfigurationInfoXML
//
//		write new info file for configName based CSV column info
//			data="type,name,dataType;type,name,dataType;..."
//		return resulting handleGetConfigurationXML mock-up view
//
void ConfigurationGUISupervisor::handleSaveConfigurationInfoXML(HttpXmlDocument &xmldoc, ConfigurationManagerRW *cfgMgr,
		std::string& configName, const std::string& data,
		bool allowOverwrite)
{
	if(!allowOverwrite)
	{
		FILE *fp = fopen((CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT).c_str(), "r");
		if(fp)
		{
			fclose(fp);
			xmldoc.addTextElementToData("ConfigurationName",
					configName);
			xmldoc.addTextElementToData("OverwriteError",
					"1");
			xmldoc.addTextElementToData("Error",
					"File already exists! ('" +
					(CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT) +
					"')");
			return;
		}
	}

	//create all caps name and validate
	//	only allow alpha-numeric names with Configuration at end
	std::string capsName;
	try
	{
		capsName = ConfigurationBase::convertToCaps(configName, true);
	}
	catch(std::runtime_error &e)
	{	//error! non-alpha
		xmldoc.addTextElementToData("Error",e.what());
		return;
	}

	__MOUT__ << "capsName=" << capsName << std::endl;
	__MOUT__ << "configName=" << configName << std::endl;

	//create preview string to validate column info before write to file
	std::stringstream outss;
	char tmp[300];

	sprintf(tmp,"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n");
	outss << tmp;
	sprintf(tmp,"\t<ROOT xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"ConfigurationInfo.xsd\">\n");
	outss << tmp;
	sprintf(tmp,"\t\t<CONFIGURATION Name=\"%s\">\n",configName.c_str());
	outss << tmp;
	sprintf(tmp,"\t\t\t<VIEW Name=\"%s\" Type=\"File,Database,DatabaseTest\">\n",capsName.c_str());
	outss << tmp;

	//each column is represented by 3 fields
	//	- type, name, dataType
	int i = 0; //use to parse data std::string
	int j = data.find(',',i); //find next field delimiter
	int k = data.find(';',i); //find next col delimiter

	while(k != (int)(std::string::npos))
	{
		//type
		outss << "\t\t\t\t<COLUMN Type=\"";
		outss << data.substr(i,j-i);

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
		catch(std::runtime_error &e)
		{	//error! non-alpha
			xmldoc.addTextElementToData("Error", std::string("For column name '") +
					data.substr(i,j-i) + "' - " + e.what());
			return;
		}

		i=j+1;
		j = data.find(',',i); //find next field delimiter

		//data type
		outss << "\" \t	DataType=\"";
		outss << data.substr(i,k-i);
		outss << "\"/>\n";

		i = k+1;
		j = data.find(',',i); //find next field delimiter
		k = data.find(';',i); //find new col delimiter
	}

	sprintf(tmp,"\t\t\t</VIEW>\n");
	outss << tmp;
	sprintf(tmp,"\t\t</CONFIGURATION>\n");
	outss << tmp;
	sprintf(tmp,"\t</ROOT>\n");
	outss << tmp;


	FILE *fp = fopen((CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT).c_str(), "w");
	if(!fp)
	{
		xmldoc.addTextElementToData("Error", "Failed to open destination Configuration Info file:" +
				(CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT));
		return;
	}
	fprintf(fp,outss.str().c_str());
	fclose(fp);

	//reload all config info with refresh AND reset to pick up possibly new config
	cfgMgr->getAllConfigurationInfo(true);

	//if error detected reading back then move the saved configuration info to .unused
	try
	{
		handleGetConfigurationXML(xmldoc,cfgMgr,configName,ConfigurationVersion());
	}
	catch(std::runtime_error& e)
	{
		//configuration info is illegal so report error, and disable file

		__SS__ << "Error detected reading back the configuration '" << configName <<
				"' created by the new attempt to save column info: " <<
				e.what() << std::endl;
		__MOUT_ERR__ << ss.str() << std::endl;
		xmldoc.addTextElementToData("Error", ss.str());

		//if error detected //move file to ".unused"
		{
			if ( 0 == rename( (CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT).c_str() ,
					(CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT + ".unused").c_str() ) )
				__MOUT_INFO__ << ( "File successfully renamed: " +
						(CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT + ".unused")) << std::endl;
			else


				__MOUT_ERR__ << ( "Error renaming file to " +
						(CONFIG_INFO_PATH + configName + CONFIG_INFO_EXT + ".unused")) << std::endl;

			//reload all with refresh to remove new configuration
			cfgMgr->getAllConfigurationInfo(true);
		}
	}

	//debug all table column info
	//FIXME -- possibly remove this debug feature in future
	std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo();

	//give a print out of currently illegal configuration column info
	__MOUT_INFO__ << "Looking for errors in all configuration column info..." << std::endl;
	for(auto& cfgInfo: allCfgInfo)
	{
		try
		{
			cfgMgr->getConfigurationByName(cfgInfo.first)->getMockupViewP()->init();
		}
		catch(std::runtime_error& e)
		{
			__MOUT_WARN__ << "\n\n##############################################\n" <<
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
void ConfigurationGUISupervisor::handleSetGroupAliasInBackboneXML(HttpXmlDocument &xmldoc,
		ConfigurationManagerRW *cfgMgr, const std::string &groupAlias,
		const std::string& groupName, ConfigurationGroupKey groupKey, const std::string &author)
try
{
	cfgMgr->loadConfigurationBackbone();
	std::map<std::string, ConfigurationVersion> activeVersions = cfgMgr->getActiveVersions();

	if(activeVersions.find("GroupAliasesConfiguration") == activeVersions.end())
	{
		__SS__ << "Active version of GroupAliasesConfiguration missing!" << std::endl;
		xmldoc.addTextElementToData("Error", ss.str());
		return;
	}

	//put all old backbone versions in xmldoc

	const std::set<std::string> backboneMembers = cfgMgr->getBackboneMemberNames();
	for(auto &memberName: backboneMembers)
	{
		__MOUT__ << "activeVersions[\"" << memberName << "\"]=" <<
				activeVersions[memberName] << std::endl;

		xmldoc.addTextElementToData("oldBackboneName",
				memberName);
		xmldoc.addTextElementToData("oldBackboneVersion",
				activeVersions[memberName].toString());
	}

	//make a temporary version from active view
	//modify the chosen groupAlias row
	//save as new version

	std::string configName = "GroupAliasesConfiguration";
	ConfigurationVersion temporaryVersion = cfgMgr->getConfigurationByName(configName)->
			createTemporaryView(activeVersions["GroupAliasesConfiguration"]);

	__MOUT__ << "\t\t temporaryVersion: " << temporaryVersion << std::endl;

	ConfigurationView* configView = cfgMgr->getConfigurationByName(configName)->
			getTemporaryView(temporaryVersion);
	unsigned int row = configView->findRow(configView->findCol("GroupKeyAlias"),groupAlias);

	__MOUT__ << "\t\t row: " << row << std::endl;

	//only make a new version if we are changing compared to active backbone
	bool isDifferent = false;

	unsigned int col = configView->findCol("GroupName");

	__MOUT__ << "\t\t groupName: " << groupName << " vs " <<
			configView->getDataView()[row][col] << std::endl;
	if(groupName != configView->getDataView()[row][col])
	{
		configView->setValue(groupName, row, col);
		isDifferent = true;
	}

	col = configView->findCol("GroupKey");
	__MOUT__ << "\t\t groupKey: " << groupKey << " vs " <<
			configView->getDataView()[row][col] << std::endl;
	if(groupKey.toString() != configView->getDataView()[row][col])
	{
		configView->setValue(groupKey.toString(), row, col);
		isDifferent = true;
	}

	ConfigurationVersion newAssignedVersion;
	if(isDifferent)	//make new version if different
	{
		configView->setValue(author, row, configView->findCol("Author"));
		configView->setValue(time(0), row, configView->findCol("RecordInsertionTime"));

		__MOUT__ << "\t\t**************************** Save as new sub-config version" << std::endl;

		newAssignedVersion =
				cfgMgr->saveNewConfiguration(configName,temporaryVersion);
	}
	else	//use existing version
	{
		__MOUT__ << "\t\t**************************** Using existing sub-config version" << std::endl;

		newAssignedVersion = activeVersions["GroupAliasesConfiguration"];
	}

	xmldoc.addTextElementToData("savedAlias", configName);
	xmldoc.addTextElementToData("savedVersion", newAssignedVersion.toString());
	__MOUT__ << "\t\t newAssignedVersion: " << newAssignedVersion << std::endl;
}
catch(std::runtime_error &e)
{
	__MOUT__ << "Error detected!\n\n " << e.what() << std::endl;
	xmldoc.addTextElementToData("Error", "Error saving new Group Alias view! " +
			std::string(e.what()));
}
catch(...)
{
	__MOUT__ << "Error detected!\n\n "<< std::endl;
	xmldoc.addTextElementToData("Error", "Error saving new Group Alias view! ");
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
void ConfigurationGUISupervisor::handleGroupAliasesXML(HttpXmlDocument &xmldoc,
		ConfigurationManagerRW *cfgMgr)
{
	cfgMgr->loadConfigurationBackbone();
	std::map<std::string, ConfigurationVersion> activeVersions = cfgMgr->getActiveVersions();

	if(activeVersions.find("GroupAliasesConfiguration") == activeVersions.end())
	{
		__SS__ << "Active version of GroupAliasesConfiguration missing!" << std::endl;
		xmldoc.addTextElementToData("Error", ss.str());
		return;
	}
	__MOUT__ << "activeVersions[\"GroupAliasesConfiguration\"]=" <<
			activeVersions["GroupAliasesConfiguration"] << std::endl;
	xmldoc.addTextElementToData("GroupAliasesConfigurationName",
			"GroupAliasesConfiguration");
	xmldoc.addTextElementToData("GroupAliasesConfigurationVersion",
			activeVersions["GroupAliasesConfiguration"].toString());

	std::map<std::string,ConfigurationTree> aliasNodePairs =
			cfgMgr->getNode("GroupAliasesConfiguration").getChildren();

	for(auto& aliasNodePair:aliasNodePairs)
	{
		xmldoc.addTextElementToData("GroupAlias", aliasNodePair.first);
		xmldoc.addTextElementToData("GroupName",
				aliasNodePair.second.getNode("GroupName").getValueAsString());
		xmldoc.addTextElementToData("GroupKey",
				aliasNodePair.second.getNode("GroupKey").getValueAsString());
		xmldoc.addTextElementToData("GroupComment",
				aliasNodePair.second.getNode("CommentDescription").getValueAsString());
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
void ConfigurationGUISupervisor::handleVersionAliasesXML(HttpXmlDocument &xmldoc,
		ConfigurationManagerRW *cfgMgr)
{
	cfgMgr->loadConfigurationBackbone();
	std::map<std::string, ConfigurationVersion> activeVersions = cfgMgr->getActiveVersions();

	if(activeVersions.find("VersionAliases") == activeVersions.end())
	{
		__SS__ << "Active version of VersionAliases  missing!" << std::endl;
		xmldoc.addTextElementToData("Error", ss.str());
		return;
	}
	__MOUT__ << "activeVersions[\"VersionAliases\"]=" <<
			activeVersions["VersionAliases"] << std::endl;
	xmldoc.addTextElementToData("VersionAliasesVersion",
			activeVersions["VersionAliases"].toString());

	std::map<std::string,ConfigurationTree> aliasNodePairs =
			cfgMgr->getNode("VersionAliases").getChildren();

	for(auto& aliasNodePair:aliasNodePairs)
	{
		xmldoc.addTextElementToData("VersionAlias",
				aliasNodePair.second.getNode("VersionAlias").getValueAsString());
		xmldoc.addTextElementToData("ConfigurationName",
				aliasNodePair.second.getNode("ConfigurationName").getValueAsString());
		xmldoc.addTextElementToData("Version",
				aliasNodePair.second.getNode("Version").getValueAsString());
		xmldoc.addTextElementToData("Comment",
				aliasNodePair.second.getNode("CommentDescription").getValueAsString());
	}
}


//========================================================================================================================
//	handleConfigurationGroupsXML
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
void ConfigurationGUISupervisor::handleConfigurationGroupsXML(HttpXmlDocument &xmldoc, ConfigurationManagerRW *cfgMgr)
{
	DOMElement* parentEl;

	ConfigurationInterface* theInterface = cfgMgr->getConfigurationInterface();
	std::set<std::string /*name*/>  configGroups = theInterface->getAllConfigurationGroupNames();
	__MOUT__ << "Number of Config groups: " << configGroups.size() << std::endl;

	ConfigurationGroupKey groupKey;
	std::string groupName;

	for(auto &groupString:configGroups)
	{
		ConfigurationGroupKey::getGroupNameAndKey(groupString,groupName,groupKey);

		__MOUT__ << "Config group " << groupString << " := " << groupName <<
				"(" << groupKey << ")" << std::endl;

		xmldoc.addTextElementToData("ConfigurationGroupName", groupName);
		xmldoc.addTextElementToData("ConfigurationGroupKey", groupKey.toString());
		parentEl = xmldoc.addTextElementToData("ConfigurationGroupMembers", "");


		std::map<std::string /*name*/, ConfigurationVersion /*version*/> memberMap;

		//try to get members
		try
		{
			memberMap = theInterface->getConfigurationGroupMembers(groupString);
		}
		catch(std::runtime_error &e)
		{
			__SS__ << "Configuration group \"" + groupString +
					"\" has been corrupted! " + e.what() << std::endl;
			__MOUT__ << ss.str();
			xmldoc.addTextElementToData("Error",ss.str());
			xmldoc.addTextElementToData("ConfigurationGroupType", "Invalid");
			continue;
		}
		catch(...)
		{
			__SS__ << "Configuration group \"" + groupString +
					"\" has been corrupted! " << std::endl;
			__MOUT__ << ss.str();
			xmldoc.addTextElementToData("Error",ss.str());
			xmldoc.addTextElementToData("ConfigurationGroupType", "Invalid");
			continue;
		}

		//try to determine type, dont report errors, just mark "Invalid"
		try
		{
			//determine the type configuration group
			int groupType = cfgMgr->getTypeOfGroup(groupName,groupKey,memberMap);
			std::string groupTypeString =
					groupType==ConfigurationManager::CONTEXT_TYPE?"Context":
							(groupType==ConfigurationManager::BACKBONE_TYPE?
									"Backbone":"Configuration");
			xmldoc.addTextElementToData("ConfigurationGroupType", groupTypeString);
		}
		catch(std::runtime_error &e)
		{
			__SS__ << "Configuration group \"" + groupString +
					"\" has invalid type! " + e.what() << std::endl;
			__MOUT__ << ss.str();
			xmldoc.addTextElementToData("ConfigurationGroupType", "Invalid");
			continue;
		}
		catch(...)
		{
			__SS__ << "Configuration group \"" + groupString +
					"\" has invalid type! " << std::endl;
			__MOUT__ << ss.str();
			xmldoc.addTextElementToData("ConfigurationGroupType", "Invalid");
			continue;
		}

		for(auto &memberPair:memberMap)
		{
			__MOUT__ << "\tMember config " << memberPair.first << ":" << memberPair.second << std::endl;
			xmldoc.addTextElementToParent("MemberName", memberPair.first, parentEl);
			xmldoc.addTextElementToParent("MemberVersion", memberPair.second.toString(), parentEl);
		}
	}

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
void ConfigurationGUISupervisor::handleConfigurationsXML(HttpXmlDocument &xmldoc,
		ConfigurationManagerRW *cfgMgr,
		bool allowIllegalColumns)
{
	DOMElement* parentEl;

	std::string accumulatedErrors = "";
	std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo(
			allowIllegalColumns,allowIllegalColumns?&accumulatedErrors:0); //if allowIllegalColumns, then also refresh
	std::map<std::string, ConfigurationInfo>::const_iterator it = allCfgInfo.begin();

	__MOUT__ << "# of configuration tables found: " << allCfgInfo.size() << std::endl;
	while(it != allCfgInfo.end())
	{
		//for each subconfiguration name
		//get existing version keys

		__MOUT__ << "Name: " << it->first << " - #ofVersions: " << it->second.versions_.size() << std::endl;

		//add system subconfiguration name
		xmldoc.addTextElementToData("ConfigurationName", it->first);
		parentEl = xmldoc.addTextElementToData("ConfigurationVersions", "");

		//get version key for the current system subconfiguration key
		for (auto &version:it->second.versions_)
			xmldoc.addTextElementToParent("Version", version.toString(), parentEl);

		++it;
	}

	if(accumulatedErrors != "")
		xmldoc.addTextElementToData("Error", std::string("Column errors were allowed for this request, ") +
				"but please note the following errors:\n" + accumulatedErrors);
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
	//
	ConfigurationManagerRW *cfgMgr = &cfgMgrInst;
	//
	std::map<std::string, ConfigurationInfo> allCfgInfo = cfgMgr->getAllConfigurationInfo(true);
	__MOUT__ << "allCfgInfo.size() = " << allCfgInfo.size() << std::endl;
	for(auto& mapIt : allCfgInfo)
	{
		__MOUT__ << "Config Name: " << mapIt.first << std::endl;
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






