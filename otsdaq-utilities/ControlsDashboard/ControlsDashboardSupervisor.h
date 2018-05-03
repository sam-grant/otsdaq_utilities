#ifndef _ots_ControlsDashboardSupervisor_h_
#define _ots_ControlsDashboardSupervisor_h_

#include "otsdaq-core/CoreSupervisors/CoreSupervisorBase.h"

//#include "otsdaq-utilities/SlowControlsInterfacePlugins/EpicsInterface.h"
//#include "EpicsInterface.h.bkup"



namespace ots
{

	class ControlsVInterface;
	class ConfigurationManager;

class ControlsDashboardSupervisor: public CoreSupervisorBase
{

public:

    XDAQ_INSTANTIATOR();

    						ControlsDashboardSupervisor     (xdaq::ApplicationStub* s);
	virtual 				~ControlsDashboardSupervisor	(void);

	void 					init                  			(void);
	void 					destroy              			(void);


	virtual void			request         	 						(const std::string& requestType, cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut, const WebUsers::RequestUserInfo& userInfo) override;

    virtual void			setSupervisorPropertyDefaults				(void) override;
    virtual void			forceSupervisorPropertyValues				(void) override; //override to force supervisor property values (and ignore user settings)


    void Poll                                    (cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut, std::string UID) 	throw (xgi::exception::Exception);
    void GetPVSettings                           (cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut, std::string pvList);
    void GenerateUID                             (cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut, std::string pvlist);
    void GetList                                 (cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut) 				 	throw (xgi::exception::Exception);
    void GetPages                                (cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut) 				 	throw (xgi::exception::Exception);
    void loadPage                                (cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut, std::string page)	throw (xgi::exception::Exception);
    void Subscribe                               (cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut) 					throw (xgi::exception::Exception);
    void Unsubscribe                             (cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut) 					throw (xgi::exception::Exception);



    //Utilities, eventually to be moved
    bool isDir									 (std::string dir                    );
    void listFiles								 (std::string baseDir, bool recursive, std::vector<std::string> * pages );


private:
	//SlowControlsInterface
    //AllSupervisorInfo 						allSupervisorInfo_;
	//EpicsInterface                        * interface_;
    ControlsVInterface*                     interface_;
//    ConfigurationManager*          			theConfigurationManager_;
//    RemoteWebUsers							theRemoteWebUsers_;
//	std::string                             username;
	std::map<int, std::set<std::string>> 	pvDependencyLookupMap_;
	int										UID_;


};

}

#endif
