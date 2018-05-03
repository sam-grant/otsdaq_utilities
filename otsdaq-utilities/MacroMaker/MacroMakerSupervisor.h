#ifndef _ots_MacroMakerSupervisor_h_
#define _ots_MacroMakerSupervisor_h_

#include "otsdaq-core/CoreSupervisors/CoreSupervisorBase.h"

namespace ots
{

class MacroMakerSupervisor: public CoreSupervisorBase
{

public:

    XDAQ_INSTANTIATOR();


    						MacroMakerSupervisor            (xdaq::ApplicationStub* s) throw (xdaq::exception::Exception);
	virtual 				~MacroMakerSupervisor   		(void);

	void 					init	              			(void);
	void 					destroy              			(void);

    virtual void 			defaultPage      				(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception) override;
    virtual void			request         	 			(const std::string& requestType, cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut, 	const WebUsers::RequestUserInfo& userInfo) throw (xgi::exception::Exception) override;

    virtual void			forceSupervisorPropertyValues	(void) override; //override to force supervisor property values (and ignore user settings)


    //void Default               		(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    //void MacroMakerRequest          (xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);

    //xoap GetMacroList (username) //give macro list for user's and public
    //xoap RunMacro(macropath)		//get back unique id?
    //xoap getProgressOfRunningMacro(uid) //100%

private:

    //	void printStatus();

	void handleRequest				(const std::string Command, HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username);
	void getFElist					(HttpXmlDocument& xmldoc);
	void getFEMacroList				(HttpXmlDocument& xmldoc, const std::string &username);

	void writeData					(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username);
	void readData					(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username);
	void createMacro				(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username);
	void loadMacros					(HttpXmlDocument& xmldoc, const std::string &username);
	void appendCommandToHistory		(std::string command, std::string Format, std::string time, std::string interfaces, const std::string &username);
	void loadHistory				(HttpXmlDocument& xmldoc, const std::string &username);
	void deleteMacro				(HttpXmlDocument& xmldoc,cgicc::Cgicc& cgi, const std::string &username);
	void editMacro					(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username);
	void clearHistory				(const std::string &username);
	void exportMacro				(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username);
	void runFEMacro					(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi);


	std::string generateHexArray	(const std::string& sourceHexString,int &numOfBytes);

    //AllSupervisorInfo 						allSupervisorInfo_;
    //RemoteWebUsers							theRemoteWebUsers_;
    //FESupervisor*			 				theFESupervisor_;
	//ConfigurationManager*   				theConfigurationManager_;

	SupervisorInfoMap						allFESupervisorInfo_;

};

}

#endif
