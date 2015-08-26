#ifndef _ots_LogbookSupervisor_h
#define _ots_LogbookSupervisor_h

#include "otsdaq-core/SOAPUtilities/SOAPMessenger.h"
#include "otsdaq-core/SupervisorConfigurations/SupervisorConfiguration.h"
#include "otsdaq-core/WebUsersUtilities/RemoteWebUsers.h"

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

class HttpXmlDocument;

class LogbookSupervisor: public xdaq::Application, public SOAPMessenger
{

public:

    XDAQ_INSTANTIATOR();

    LogbookSupervisor            	(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);
    virtual ~LogbookSupervisor   	(void);

    void init                  		(void);
    void destroy              		(void);

    void Default               		(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    void Log	               		(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    void LogImage              		(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    void LogReport             		(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);

    //External Supervisor XOAP handlers
    xoap::MessageReference 		MakeSystemLogbookEntry 		 (xoap::MessageReference msg) 			throw (xoap::exception::Exception);

private:

    bool 			validateExperimentName		(std::string &experiment);
    std::string		getActiveExperiment			();
    void 			setActiveExperiment			(std::string experiment = "");
    void			createExperiment			(std::string experiment, std::string creator, HttpXmlDocument *xmldoc = 0);
    void			removeExperiment			(std::string experiment, std::string remover, HttpXmlDocument *xmldoc = 0);
    void			getExperiments				(HttpXmlDocument *xmldoc = 0, std::ostringstream *out = 0);
    void			webUserSetActiveExperiment	(std::string experiment, HttpXmlDocument *xmldoc = 0);
    void			refreshLogbook				(time_t date, unsigned char duration, HttpXmlDocument *xmldoc = 0, std::ostringstream *out = 0, std::string experiment = "");
    void            cleanUpPreviews             ();
    void            savePostPreview             (std::string &subject, std::string &text, const std::vector<cgicc::FormFile> &files, std::string creator, HttpXmlDocument *xmldoc = 0);
	void 			escapeLogbookEntry          (std::string &entry);
	std::string     validateUploadFileType      (const std::string fileType);
    void            movePreviewEntry            (std::string previewNumber, bool approve, HttpXmlDocument *xmldoc = 0);
    void 			hideLogbookEntry	        (const std::string &entryId, bool hide, const std::string &hider);
    
    SupervisorConfiguration              	theSupervisorsConfiguration_;
    RemoteWebUsers						theRemoteWebUsers_;

    enum {
    	ADMIN_PERMISSIONS_THRESHOLD = 255,
    	EXPERIMENT_NAME_MIN_LENTH = 3,
    	EXPERIMENT_NAME_MAX_LENTH = 25,
    	LOGBOOK_PREVIEW_EXPIRATION_TIME = 60*20, //20 minutes
    };
    std::vector<std::string>                allowedFileUploadTypes_, matchingFileUploadTypes_;
    
    std::string                             activeExperiment_;
    unsigned int							mostRecentDayIndex_;
};

}

#endif
