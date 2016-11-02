#ifndef _ots_ConfigurationGUISupervisor_h
#define _ots_ConfigurationGUISupervisor_h

#include "otsdaq-core/SOAPUtilities/SOAPMessenger.h"

#include "otsdaq-core/SupervisorConfigurations/SupervisorConfiguration.h"
#include "otsdaq-core/WebUsersUtilities/RemoteWebUsers.h"
#include "otsdaq-core/ConfigurationInterface/ConfigurationManagerRW.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"
#include "xdaq/Application.h"
#include "xgi/Method.h"

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPBody.h"
#include "xoap/domutils.h"
#include "xoap/Method.h"


#include "cgicc/HTMLClasses.h"
#include <cgicc/HTTPCookie.h>
#include "cgicc/HTMLDoctype.h"
#include <cgicc/HTTPHeader.h>

#include <string>
#include <map>

namespace ots
{

class ConfigurationGUISupervisor: public xdaq::Application, public SOAPMessenger
{

public:

    XDAQ_INSTANTIATOR();

    ConfigurationGUISupervisor            (xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);
    virtual ~ConfigurationGUISupervisor   (void);

    void init                  (void);
    void destroy               (void);
    void Default               (xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);

    void 						request                      			(xgi::Input* in, xgi::Output* out )  	throw (xgi::exception::Exception);

private:
    void 			handleSaveConfigurationInfoXML		(HttpXmlDocument &xmldoc, ConfigurationManagerRW *cfgMgr, std::string &configName, const std::string& columnCSV, bool allowOverwrite=false);
    void 			handleDeleteConfigurationInfoXML	(HttpXmlDocument &xmldoc, ConfigurationManagerRW *cfgMgr, std::string &configName);

    void 			handleGroupAliasesXML				(HttpXmlDocument &xmldoc, ConfigurationManagerRW *cfgMgr);
    void 			handleSetGroupAliasInBackboneXML	(HttpXmlDocument &xmldoc, ConfigurationManagerRW *cfgMgr, const std::string &groupAlias, const std::string& groupName, ConfigurationGroupKey groupKey, const std::string &author);
    void 			handleVersionAliasesXML				(HttpXmlDocument &xmldoc, ConfigurationManagerRW *cfgMgr);
    void 			handleConfigurationGroupsXML		(HttpXmlDocument &xmldoc, ConfigurationManagerRW *cfgMgr);
    void 			handleGetConfigurationGroupXML		(HttpXmlDocument &xmldoc, ConfigurationManagerRW *cfgMgr, const std::string &groupName, ConfigurationGroupKey groupKey);
    void			handleCreateConfigurationGroupXML	(HttpXmlDocument &xmldoc, ConfigurationManagerRW *cfgMgr, const std::string &groupName, const std::string &configList, bool allowDuplicates=false);

    void 			handleConfigurationsXML				(HttpXmlDocument &xmldoc, ConfigurationManagerRW *cfgMgr, bool allowIllegalColumns);
    void 			handleGetConfigurationXML			(HttpXmlDocument &xmldoc, ConfigurationManagerRW *cfgMgr, const std::string &configName, ConfigurationVersion version, bool allowIllegalColumns=false);
    void 			handleCreateConfigurationXML		(HttpXmlDocument &xmldoc, ConfigurationManagerRW *cfgMgr, const std::string &configName, ConfigurationVersion version, bool makeTemporary, const std::string &data, const int &dataOffset, const std::string &author);

    void 			handleFillTreeViewXML				(HttpXmlDocument &xmldoc, ConfigurationManagerRW *cfgMgr, const std::string &groupName, const ConfigurationGroupKey &groupKey, const std::string &startPath, int depth);
    static void		recursiveTreeToXML					(const ConfigurationTree &t, unsigned int depth, HttpXmlDocument &xmldoc, DOMElement* parentEl);

    void testXDAQContext();

    enum {
        USER_PERMISSIONS_THRESHOLD = 10,
        CONFIGURATION_MANAGER_EXPIRATION_TIME = 60*60*1, //1 hour, in seconds
        CONFIGURATION_MANAGER_REFRESH_THRESHOLD = 60*1, //1 minute, in seconds
    };

    SupervisorConfiguration    							theSupervisorsConfiguration_;
    RemoteWebUsers             							theRemoteWebUsers_;


    ConfigurationManagerRW*								refreshUserSession(std::string username, uint64_t activeSessionIndex, bool refresh);
    std::map<std::string, ConfigurationManagerRW *> 	userConfigurationManagers_;
    std::map<std::string, time_t> 						userLastUseTime_;
};

}

#endif
