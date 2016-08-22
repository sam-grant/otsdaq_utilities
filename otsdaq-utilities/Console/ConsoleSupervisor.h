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

    static void						MFReceiverWorkLoop			(ConsoleSupervisor *cs);
    
    SupervisorConfiguration         theSupervisorsConfiguration_;
    RemoteWebUsers					theRemoteWebUsers_;



    struct ConsoleMessageStruct
    {
    	ConsoleMessageStruct()
    	{
    		buffer.resize(BUFFER_SZ);

    		//init fields to position -1 (for unknown)
    		//NOTE: must be in order of appearance in buffer
    		fields[LEVEL].set("Level",4,-1);
    		fields[LABEL].set("Label",5,-1);
    		fields[SOURCE].set("Source",6,-1);
    		fields[MSG].set("Msg",11,-1);
    	}

    	void set(std::string msg)
    	{
    		buffer = msg.substr(0,BUFFER_SZ); //clip to BUFFER_SZ

    		//find fields
    		int i=0, m=0;
    		size_t p;
    		//loop until no more markers
    		while( (p = buffer.find('|',p))
    				!= std::string::npos)
    		{
    			++m; //found next marker
    			if(i < (int)fields.size() &&
    					m == fields[i].markerCount) //found marker for field
    				fields[i++].posInString = p+1; //set position in string and move on to next field

    			//change all | to \0 so strings are terminated
    			buffer[p] = '\0';
    		}

    		//debug
			//    		for(auto &f: fields)
			//    		{
			//    			std::cout << f.fieldName << ": " ;
			//    			std::cout << (char *)&buffer[f.posInString] << std::endl;
			//    		}
    	}

    	const char * getMsg() {return  (char *)&buffer[fields[MSG].posInString];}
    	const char * getLabel() {return  (char *)&buffer[fields[LABEL].posInString];}
    	const char * getLevel() {return  (char *)&buffer[fields[LEVEL].posInString];}
    	const char * getSource() {return  (char *)&buffer[fields[SOURCE].posInString];}

    	const int BUFFER_SZ = 5000;


    	//define field structure
    	struct FieldStruct {
    		void set(std::string fn, int mc, int ps)
    		{ fieldName=fn; markerCount=mc; posInString=ps; }
    		std::string fieldName;
    		int markerCount;
    		int posInString;
    	};

    	//define field index enum alias
    	enum {		//must be in order of appearance in buffer
    		LEVEL,	//aka SEVERITY
    		LABEL,
    		SOURCE,
    		MSG
    	};
    	std::array<FieldStruct,4> fields;
    	std::string buffer;
    };
    
    ConsoleMessageStruct			messages[100];
};


}

#endif
