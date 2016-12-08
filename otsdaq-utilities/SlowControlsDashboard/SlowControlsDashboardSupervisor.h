#ifndef _ots_SlowControlsDashboardSupervisor_h
#define _ots_SlowControlsDashboardSupervisor_h

#include "otsdaq-core/SOAPUtilities/SOAPMessenger.h"
#include "otsdaq-core/WebUsersUtilities/RemoteWebUsers.h"
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
#include <vector>
#include <set>

#include "otsdaq-core/SupervisorDescriptorInfo/SupervisorDescriptorInfo.h"
//#include "EpicsInterface.h.bkup"



namespace ots
{

	class SlowControlsInterface;
	class ConfigurationManager;

class SlowControlsDashboardSupervisor: public xdaq::Application, public SOAPMessenger
{

public:

    XDAQ_INSTANTIATOR();

    SlowControlsDashboardSupervisor              (xdaq::ApplicationStub *        ) throw (xdaq::exception::Exception);
    virtual ~SlowControlsDashboardSupervisor     (void                                                              );
    void init                  		  	 		 (void                                                              );
    void destroy                   		 		 (void                                                              );
	void requestHandler	            		 	 (xgi::Input* in, xgi::Output* out) 											throw (xgi::exception::Exception);	
	void Default                      		 	 (xgi::Input* in, xgi::Output* out)							 					throw (xgi::exception::Exception);
    void Poll                                    (xgi::Input* in, xgi::Output* out, HttpXmlDocument *xmldoc, std::string UID) 	throw (xgi::exception::Exception); 
    void GetPVSettings                           (xgi::Input * in, xgi::Output * out, HttpXmlDocument *xmldoc, std::string pvList ) throw (xgi::exception::Exception);
    void GenerateUID                             (xgi::Input* in, xgi::Output* out, HttpXmlDocument *xmldoc, std::string pvlist)throw (xgi::exception::Exception); 
    void GetList                                 (xgi::Input* in, xgi::Output* out, HttpXmlDocument *xmldoc) 				 	throw (xgi::exception::Exception); 
    void GetPages                                (xgi::Input* in, xgi::Output* out, HttpXmlDocument *xmldoc) 				 	throw (xgi::exception::Exception); 
    void loadPage                                (xgi::Input* in, xgi::Output* out, HttpXmlDocument *xmldoc, std::string page)	throw (xgi::exception::Exception); 
    void Subscribe                               (xgi::Input* in, xgi::Output* out, HttpXmlDocument *xmldoc) 					throw (xgi::exception::Exception);
    void Unsubscribe                             (xgi::Input* in, xgi::Output* out, HttpXmlDocument *xmldoc) 					throw (xgi::exception::Exception);
 
    
    //Utilities, eventually to be moved
    bool isDir									 (std::string dir                    );
    void listFiles								 (std::string baseDir, bool recursive, std::vector<std::string> * pages );
    

private:
	//SlowControlsInterface 
    SupervisorDescriptorInfo              	theSupervisorDescriptorInfo_;
	//EpicsInterface                        * interface_;
    ConfigurationManager*          			theConfigurationManager_;
    RemoteWebUsers							theRemoteWebUsers_;
	std::string                             username;
	std::map<int, std::set<std::string>> 	pvDependencyLookupMap_;
	int										UID_;


};

}

#endif
