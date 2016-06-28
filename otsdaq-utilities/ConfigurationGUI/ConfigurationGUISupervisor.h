#ifndef _ots_ConfigurationGUISupervisor_h
#define _ots_ConfigurationGUISupervisor_h

#include "otsdaq-core/SOAPUtilities/SOAPMessenger.h"

#include "otsdaq-core/SupervisorConfigurations/SupervisorConfiguration.h"
#include "otsdaq-core/WebUsersUtilities/RemoteWebUsers.h"
#include "otsdaq-core/ConfigurationInterface/ConfigurationManagerWithWriteAccess.h"

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

    //modifying generic ConfigurationBase
	int 						saveNewConfiguration					(ConfigurationManagerWithWriteAccess *cfgMgr, std::string configurationName, int temporaryVersion = -1);

	//modifiers of backbone configuration members
    int							createTemporaryBackboneView				(ConfigurationManagerWithWriteAccess *cfgMgr, int sourceViewVersion = -1); //-1, from MockUp, else from valid backbone view version

    //modifiers of specific configuration based on alias, e.g. "Physics"
    void						setKOCVersionForSpecificConfiguration	(ConfigurationManagerWithWriteAccess *cfgMgr, int temporaryBackboneVersion, std::string configAlias, std::string KOCAlias, int newKOCVersion);
    void						deleteKOCForSpecificConfiguration		(ConfigurationManagerWithWriteAccess *cfgMgr, int temporaryBackboneVersion, std::string configAlias, std::string KOCAlias);
    void						addKOCForSpecificConfiguration			(ConfigurationManagerWithWriteAccess *cfgMgr, int temporaryBackboneVersion, std::string configAlias, std::string KOCAlias, int newKOCVersion);



    enum {
        USER_PERMISSIONS_THRESHOLD = 10,
        CONFIGURATION_MANAGER_EXPIRATION_TIME = 60*60*1, //1 hour, in seconds
    };

    SupervisorConfiguration    										theSupervisorsConfiguration_;
    RemoteWebUsers             										theRemoteWebUsers_;


    ConfigurationManagerWithWriteAccess*							refreshUserSession(std::string username, uint64_t activeSessionIndex, int &backboneVersion);
    std::map<std::string, ConfigurationManagerWithWriteAccess *> 	userConfigurationManagers_;
    std::map<std::string, time_t> 									userLastUseTime_;
};

}

#endif
