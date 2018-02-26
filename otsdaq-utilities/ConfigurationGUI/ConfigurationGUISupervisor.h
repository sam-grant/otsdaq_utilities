#ifndef _ots_ConfigurationGUISupervisor_h_
#define _ots_ConfigurationGUISupervisor_h_

#include "otsdaq-core/SOAPUtilities/SOAPMessenger.h"

#include "otsdaq-core/WebUsersUtilities/RemoteWebUsers.h"
#include "otsdaq-core/ConfigurationInterface/ConfigurationManagerRW.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"

#include <xdaq/Application.h>
#include <xgi/Method.h>

#include <xoap/MessageReference.h>
#include <xoap/MessageFactory.h>
#include <xoap/SOAPEnvelope.h>
#include <xoap/SOAPBody.h>
#include <xoap/domutils.h>
#include <xoap/Method.h>


#include <cgicc/HTMLClasses.h>
#include <cgicc/HTTPCookie.h>
#include <cgicc/HTMLDoctype.h>
#include <cgicc/HTTPHeader.h>

#include <string>
#include <map>
#include "otsdaq-core/SupervisorInfo/AllSupervisorInfo.h"


namespace ots
{

class ConfigurationGUISupervisor: public xdaq::Application, public SOAPMessenger
{

public:

    XDAQ_INSTANTIATOR();

    ConfigurationGUISupervisor            (xdaq::ApplicationStub* s) throw (xdaq::exception::Exception);
    virtual ~ConfigurationGUISupervisor   (void);

    void init                  (void);
    void destroy               (void);
    void Default               (xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    void request               (xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);

private:
    void 			handleSaveConfigurationInfoXML				(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, std::string& configName, const std::string& columnCSV, const std::string& tableDescription, const std::string& columnChoicesCSV, bool allowOverwrite=false);
    void 			handleDeleteConfigurationInfoXML			(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, std::string& configName);

    void 			handleGroupAliasesXML						(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr);
    void 			handleSetGroupAliasInBackboneXML			(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& groupAlias, const std::string& groupName, ConfigurationGroupKey groupKey, const std::string& author);
    void 			handleSetVersionAliasInBackboneXML			(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& versionAlias, const std::string& configName, ConfigurationVersion version, const std::string& author);
    void			handleAliasGroupMembersInBackboneXML		(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& versionAlias, const std::string& groupName, ConfigurationGroupKey groupKey, const std::string& author);
    void 			handleVersionAliasesXML						(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr);
    void 			handleConfigurationGroupsXML				(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, bool returnMembers);
    void 			handleGetConfigurationGroupXML				(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& groupName, ConfigurationGroupKey groupKey);
    void 			handleGetConfigurationGroupTypeXML			(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& configList);
    void			handleCreateConfigurationGroupXML			(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& groupName, const std::string& configList, bool allowDuplicates=false, bool ignoreWarnings=false, const std::string& groupComment = "", bool lookForEquivalent = false);

    void 			handleConfigurationsXML						(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, bool allowIllegalColumns);
    void 			handleGetConfigurationXML					(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& configName, ConfigurationVersion version, bool allowIllegalColumns=false);
    void 			handleCreateConfigurationXML				(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& configName, ConfigurationVersion version, bool makeTemporary, const std::string& data, const int& dataOffset, const std::string& author, const std::string& comment, bool sourceTableAsIs, bool lookForEquivalent);

    void			setupActiveTablesXML						(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& groupName, const ConfigurationGroupKey& groupKey, const std::string& modifiedTables, bool refreshAll = true, bool getGroupInfo = false, std::map<std::string /*name*/, ConfigurationVersion /*version*/>* returnMemberMap = 0, bool outputActiveTables = true, std::string* accumulatedErrors = 0);
    void 			handleFillTreeViewXML						(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& groupName, const ConfigurationGroupKey& groupKey, const std::string& startPath, unsigned int depth, bool hideStatusFalse, const std::string& modifiedTables, const std::string& filterList);
    static void		recursiveTreeToXML							(const ConfigurationTree& t, unsigned int depth, HttpXmlDocument& xmldoc, DOMElement* parentEl, bool hideStatusFalse);
    void 			handleFillTreeNodeCommonFieldsXML			(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& groupName, const ConfigurationGroupKey& groupKey, const std::string& startPath, unsigned int depth, const std::string& modifiedTables, const std::string& recordList, const std::string& fieldList);
    void 			handleFillUniqueFieldValuesForRecordsXML	(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& groupName, const ConfigurationGroupKey& groupKey, const std::string& startPath, const std::string& modifiedTables, const std::string& recordList, const std::string& fieldList);
    void 			handleFillGetTreeNodeFieldValuesXML			(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& groupName, const ConfigurationGroupKey& groupKey, const std::string& startPath, const std::string& modifiedTables, const std::string& recordList, const std::string& fieldList);
    void 			handleFillSetTreeNodeFieldValuesXML			(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& groupName, const ConfigurationGroupKey& groupKey, const std::string& startPath, const std::string& modifiedTables, const std::string& recordList, const std::string& fieldList, const std::string& valueList, const std::string& author);
    void			handleFillCreateTreeNodeRecordsXML			(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& groupName, const ConfigurationGroupKey& groupKey, const std::string& startPath, const std::string& modifiedTables, const std::string& recordList, const std::string& author);
    void			handleFillDeleteTreeNodeRecordsXML			(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& groupName, const ConfigurationGroupKey& groupKey, const std::string& startPath, const std::string& modifiedTables, const std::string& recordList);
    void			handleFillModifiedTablesXML					(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr);

    void 			handleSaveTreeNodeEditXML					(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& configName, ConfigurationVersion version, const std::string& type, const std::string& uid, const std::string& column, const std::string& newValue, const std::string& author);
    void 			handleGetAffectedGroupsXML					(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& rootGroupName, const ConfigurationGroupKey& rootGroupKey, const std::string& modifiedTables);
    void			handleGetLinkToChoicesXML					(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& linkToTableName, const ConfigurationVersion& linkToTableVersion, const std::string& linkIdType, const std::string& linkIndex, const std::string& linkInitId);

    void			handleSavePlanCommandSequenceXML			(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& groupName, const ConfigurationGroupKey& groupKey, const std::string& modifiedTables, const std::string& author, const std::string& planName, const std::string& commandString);


    ConfigurationVersion	saveModifiedVersionXML				(HttpXmlDocument& xmldoc, ConfigurationManagerRW* cfgMgr, const std::string& configName, ConfigurationVersion originalVersion, bool makeTemporary, ConfigurationBase*  config, ConfigurationVersion temporaryModifiedVersion, bool ignoreDuplicates = false, bool lookForEquivalent = false);


    void testXDAQContext(); //for debugging

    enum {
        USER_PERMISSIONS_THRESHOLD = 10,
        CONFIGURATION_MANAGER_EXPIRATION_TIME = 60*60*1, //1 hour, in seconds
        CONFIGURATION_MANAGER_REFRESH_THRESHOLD = 60*15, //15 minute, in seconds
    };

    AllSupervisorInfo 									allSupervisorInfo_;
    RemoteWebUsers             							theRemoteWebUsers_;


    ConfigurationManagerRW*								refreshUserSession(std::string username, uint64_t activeSessionIndex, bool refresh);
    std::map<std::string, ConfigurationManagerRW* > 	userConfigurationManagers_;
    std::map<std::string, time_t> 						userLastUseTime_;
};



} //end ots namespace

#endif
