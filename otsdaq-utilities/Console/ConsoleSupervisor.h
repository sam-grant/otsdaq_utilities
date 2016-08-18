#ifndef _ots_ConsoleSupervisor_h
#define _ots_ConsoleSupervisor_h

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

struct ConsoleMessageStruct
{
	ConsoleMessageStruct()
	{
		buffer.resize(BUFFER_SZ);
	}

	void set(std::string msg)
	{
		buffer = msg.substr(0,BUFFER_SZ);
	}

	const int BUFFER_SZ = 5000;
//	const int MF_MARKER_MSG = 11;
//	const int MF_MARKER_MSG = 11;
	std::string buffer;
};


class ConsoleSupervisor: public xdaq::Application, public SOAPMessenger
{

public:

    XDAQ_INSTANTIATOR();

    ConsoleSupervisor            	(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);
    virtual ~ConsoleSupervisor   	(void);

    void init                  		(void);
    void destroy              		(void);

    void Default               		(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    void Request               		(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);

private:
    enum {
    	ADMIN_PERMISSIONS_THRESHOLD = 255,
    };

    static void						MFReceiverWorkLoop			();
    
    SupervisorConfiguration         theSupervisorsConfiguration_;
    RemoteWebUsers					theRemoteWebUsers_;

    
    ConsoleMessageStruct			messages[100];

};

}

#endif
