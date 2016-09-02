#include "otsdaq-utilities/MacroMaker/MacroMakerSupervisor.h"

#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/Macros/CoutHeaderMacros.h"
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"
#include "otsdaq-core/SOAPUtilities/SOAPUtilities.h"
#include "otsdaq-core/SOAPUtilities/SOAPParameters.h"
#include "otsdaq-core/ConfigurationDataFormats/ConfigurationKey.h"
#include "otsdaq-core/ConfigurationInterface/ConfigurationManager.h"
#include "otsdaq-core/Macros/CoutHeaderMacros.h"

#include <xdaq/NamespaceURI.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <stdio.h> //for file rename
#include <dirent.h> //for DIR
#include <sys/stat.h> //for mkdir

#define MACROS_DB_PATH 					std::string(getenv("SERVICE_DATA")) + "/MacroData/"
#define MACROS_HIST_PATH 				std::string(getenv("SERVICE_DATA")) + "/MacroHistory/"
#define MACROS_EXPORT_PATH 				std::string(getenv("SERVICE_DATA")) + "/MacroExport/"

using namespace ots;

#define __MF_SUBJECT__ "MacroMaker"
#define __MF_HDR__		__COUT_HDR_FL__
#define __MOUT_ERR__  	mf::LogError	(__MF_SUBJECT__) << __MF_HDR__
#define __MOUT_WARN__  	mf::LogWarning	(__MF_SUBJECT__) << __MF_HDR__
#define __MOUT_INFO__  	mf::LogInfo		(__MF_SUBJECT__) << __COUT_HDR__
#define __MOUT__  		mf::LogDebug	(__MF_SUBJECT__) << __MF_HDR__

XDAQ_INSTANTIATOR_IMPL(MacroMakerSupervisor)

//========================================================================================================================
MacroMakerSupervisor::MacroMakerSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception):
xdaq::Application(s   ),
SOAPMessenger  (this),
theRemoteWebUsers_(this)
{
	INIT_MF("MacroMaker");
	xgi::bind (this, &MacroMakerSupervisor::Default,                	"Default" );
	xgi::bind (this, &MacroMakerSupervisor::MacroMakerRequest,          "MacroMakerRequest" );

	init();
	SupervisorDescriptors::const_iterator it;
	it = theSupervisorsConfiguration_.getFEDescriptors().begin();
	__MOUT__<< "PixelFESupervisor instance size " <<
			theSupervisorsConfiguration_.getFEDescriptors().size() << std::endl;
	for (; it != theSupervisorsConfiguration_.getFEDescriptors().end(); it++)
	{
		__MOUT__<< "PixelFESupervisor instance " << it->first <<
				"...and..." << it->second << std::endl;
		__MOUT__<< "Look! Here's a FE! @@@" << std::endl;
	}

	//make macro directories in case they don't exist
	mkdir(((std::string)MACROS_DB_PATH).c_str(), 0755);
	mkdir(((std::string)MACROS_HIST_PATH).c_str(), 0755);
	mkdir(((std::string)MACROS_EXPORT_PATH).c_str(), 0755);
	//	//getARTDAQFEDescriptors
	//	for (const auto& it: theSupervisorsConfiguration_.getFEDescriptors())
	//	{
	//		__MOUT__<< "PixelFESupervisor instance " << it.first << std::endl;
	//		__MOUT__<< "Look! Here's a FE! @@@" << std::endl;
	//	}
	//	for (const auto& it: theSupervisorsConfiguration_.getFEDataManagerDescriptors())
	//	{
	//		__MOUT__<< "PixelFEDataManagerSupervisor instance " << it.first << std::endl;
	//		__MOUT__<< "Look! Here's a FE! @@@" << std::endl;
	//	}

}

//========================================================================================================================
MacroMakerSupervisor::~MacroMakerSupervisor(void)
{
	destroy();
}
//========================================================================================================================
void MacroMakerSupervisor::init(void)
{
	//called by constructor
	theSupervisorsConfiguration_.init(getApplicationContext());
	__MOUT__<< "#######################################" << std::endl;
	__MOUT__<< "#######################################" << std::endl;

	__MOUT__<< "Running in MacroMaker Supervisor" << std::endl;

	__MOUT__<< "#######################################" << std::endl;
	__MOUT__<< "#######################################" << std::endl;
}

//========================================================================================================================
void MacroMakerSupervisor::destroy(void)
{
	//called by destructor

}

//========================================================================================================================
void MacroMakerSupervisor::Default(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
}

//========================================================================================================================
void MacroMakerSupervisor::MacroMakerRequest(xgi::Input* in, xgi::Output* out)
throw (xgi::exception::Exception)
{

	cgicc::Cgicc cgi(in);
	std::string Command = CgiDataUtilities::getData(cgi, "RequestType");
	__MOUT__ << "Command: " << Command << std::endl;

	//FIXME -- need to lock out MacroMaker vs State machine

	HttpXmlDocument xmldoc;
	uint64_t activeSessionIndex;
	std::string user;
	uint8_t userPermissions;

	//**** start LOGIN GATEWAY CODE ***//
	{
		bool automaticCommand = Command == "RefreshLogbook"; //automatic commands should not refresh cookie code.. only user initiated commands should!
		bool checkLock = true;
		bool getUser = (Command == "CreateExperiment") || (Command == "RemoveExperiment") ||
				(Command == "PreviewEntry") || (Command == "AdminRemoveRestoreEntry");

		if(!theRemoteWebUsers_.xmlLoginGateway(
				cgi,out,&xmldoc,theSupervisorsConfiguration_,
				0,//&userPermissions,  		//acquire user's access level (optionally null pointer)
				"0",						//report user's ip address, if known
				!automaticCommand,			//true/false refresh cookie code
				1, //set access level requirement to pass gateway
				checkLock,					//true/false enable check that system is unlocked or this user has the lock
				0,//&userWithLock,			//acquire username with lock (optionally null pointer)
				(getUser?&user:0),				//acquire username of this user (optionally null pointer)
				0,//,&displayName			//acquire user's Display Name
				&activeSessionIndex		//acquire user's session index associated with the cookieCode
		))
		{	//failure
			__MOUT__ << "Failed Login Gateway: " <<
					out->str() << std::endl; //print out return string on failure
			return;
		}
	}
	//**** end LOGIN GATEWAY CODE ***//


	username = user;


	theRemoteWebUsers_.cookieCodeIsActiveForRequest(theSupervisorsConfiguration_.getSupervisorDescriptor(),
				cookieCode, &userPermissions);
	retParameters.addParameter("UserPermissions", userPermissions);
	std::cout << __COUT_HDR_FL__ << "User Permission is " << unsigned(userPermissions) << "!!!";


	DIR *dir;
	std::string macroPath = (std::string)MACROS_DB_PATH + username + "/";
	if ((dir = opendir (macroPath.c_str())) == NULL)
		mkdir(macroPath.c_str(), 0755);

	std::string histPath = (std::string)MACROS_HIST_PATH + username + "/";
	if ((dir = opendir (histPath.c_str())) == NULL)
		mkdir(histPath.c_str(), 0755);

	std::string publicPath = (std::string)MACROS_DB_PATH + "publicMacros/";
		if ((dir = opendir (publicPath.c_str())) == NULL)
			mkdir(publicPath.c_str(), 0755);

	std::string exportPath = (std::string)MACROS_EXPORT_PATH + username + "/";
	if ((dir = opendir (exportPath.c_str())) == NULL)
		mkdir(exportPath.c_str(), 0755);


	handleRequest(Command,xmldoc,cgi);
	if(Command == "getPermission")
		xmldoc.addTextElementToData("Permission", std::to_string(unsigned(userPermissions)));
	//return xml doc holding server response
	xmldoc.outputXmlDocument((std::ostringstream*) out, false);
}


void MacroMakerSupervisor::handleRequest(const std::string Command, HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi)
{
	if(Command == "FElist")
		getFElist(xmldoc);
	else if(Command == "writeData")
		writeData(xmldoc,cgi);
	else if(Command == "readData")
		readData(xmldoc,cgi);
	else if(Command == "createMacro")
		createMacro(xmldoc,cgi);
	else if(Command == "loadMacros")
		loadMacros(xmldoc);
	else if(Command == "loadHistory")
		loadHistory(xmldoc);
	else if(Command == "deleteMacro")
		deleteMacro(xmldoc,cgi);
	else if(Command == "editMacro")
		editMacro(xmldoc,cgi);
	else if(Command == "clearHistory")
		clearHistory();
	else if(Command == "exportMacro")
		exportMacro(xmldoc,cgi);
}

void MacroMakerSupervisor::getFElist(HttpXmlDocument& xmldoc)
{
	__MOUT__ << "Getting FE list!!!!!!!!!" << std::endl;

	SOAPParameters parameters; //params for xoap to send
	parameters.addParameter("Request", "GetInterfaces");

	SOAPParameters retParameters;  //params for xoap to recv
	retParameters.addParameter("FEList");

	SupervisorDescriptors::const_iterator it;
	it = theSupervisorsConfiguration_.getFEDescriptors().begin();

	__MOUT__<< "PixelFESupervisor instance size " <<
			theSupervisorsConfiguration_.getFEDescriptors().size() << std::endl;

	//loop through each front end, and send xoap request for front end list
	for (; it != theSupervisorsConfiguration_.getFEDescriptors().end(); it++)
	{
		__MOUT__<< "PixelFESupervisor instance " << it->first <<
				"...and..." << it->second << std::endl;
		__MOUT__<< "Look! Here's a FE! @@@" << std::endl;

		xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
				it->second,
				"MacroMakerSupervisorRequest",
				parameters);

		receive(retMsg, retParameters);
		std::string retMsgFEList = retParameters.getValue("FEList");

		__MOUT__<< "FE List received : " <<
				retMsgFEList << std::endl;

		std::istringstream f(retMsgFEList);
		std::string oneInterface;
		while (std::getline(f, oneInterface)){
			std::stringstream buffer;
			buffer << oneInterface.substr(0,oneInterface.rfind(":")+1)
														   << std::to_string(it->first) << oneInterface.substr(oneInterface.rfind(":"),oneInterface.length())<< std::endl;
			interfaceList.push_back(buffer.str());
			xmldoc.addTextElementToData("FE", buffer.str());
		}
	}
	return;
}

void MacroMakerSupervisor::writeData(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi)
{
	__MOUT__ << "¡¡¡¡¡¡MacroMaker wants to write data!!!!!!!!!" << std::endl;
	std::string Address = CgiDataUtilities::getData(cgi, "Address");
	std::string Data = CgiDataUtilities::getData(cgi, "Data");
	std::string interfaceIndexArray = CgiDataUtilities::getData(cgi, "interfaceIndex");
	std::string supervisorIndexArray = CgiDataUtilities::getData(cgi, "supervisorIndex");
	std::string time = CgiDataUtilities::getData(cgi, "time");
	std::string interfaces = CgiDataUtilities::getData(cgi, "interfaces");
	std::string addressFormatStr = CgiDataUtilities::getData(cgi, "addressFormatStr");
	std::string dataFormatStr = CgiDataUtilities::getData(cgi, "dataFormatStr");

	std::string command = "w:" + Address + ":" + Data;
	std::string format = addressFormatStr + ":" + dataFormatStr;
	appendCommandToHistory(command,format,time,interfaces);

	SOAPParameters parameters; //params for xoap to send
	parameters.addParameter("Request", "UniversalWrite");
	parameters.addParameter("Address",Address);
	parameters.addParameter("Data",Data);

	__MOUT__ << "Address: " << Address << " Data: " << Data << std::endl;

	__MOUT__ <<"Here comes the array from multiselect box for WRITE, behold: "
			<< supervisorIndexArray << interfaceIndexArray << std::endl;

	SupervisorDescriptors FESupervisors = theSupervisorsConfiguration_.getFEDescriptors();

	////////////////////////////////Store cgi arrays into vectors/////////////////////////////
	std::vector<std::string> interfaceIndices;
	std::istringstream f(interfaceIndexArray);
	std::string s;
	while (getline(f, s, ',')) interfaceIndices.push_back(s);
	std::vector<int> supervisorIndices;
	std::istringstream g(supervisorIndexArray);
	std::string t;
	while (getline(g, t, ',')) supervisorIndices.push_back(std::stoi(t));


	for(unsigned int i=0; i < supervisorIndices.size(); i++)
	{
		unsigned int FEIndex = supervisorIndices[i];
		std::string interfaceIndex = interfaceIndices[i];

		parameters.addParameter("InterfaceID",interfaceIndex);

		__MOUT__ <<"The index of the supervisor instance is: " << FEIndex << std::endl;
		__MOUT__ <<"...and the interface ID is: " << interfaceIndex << std::endl;

		SupervisorDescriptors::iterator it = FESupervisors.find(FEIndex);
		if (it == FESupervisors.end())
		{
			__MOUT__ << "ERROR!? FE Index doesn't exist" << std::endl;
			return;
		}

		xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
				it->second,
				"MacroMakerSupervisorRequest",
				parameters);
		receive(retMsg);
	}
}

void MacroMakerSupervisor::readData(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi)
{
	__MOUT__ << "@@@@@@@ MacroMaker wants to read data @@@@@@@@" << std::endl;
	std::string Address = CgiDataUtilities::getData(cgi, "Address");
	std::string interfaceIndexArray = CgiDataUtilities::getData(cgi, "interfaceIndex");
	std::string supervisorIndexArray = CgiDataUtilities::getData(cgi, "supervisorIndex");
	std::string time = CgiDataUtilities::getData(cgi, "time");
	std::string interfaces = CgiDataUtilities::getData(cgi, "interfaces");
	std::string addressFormatStr = CgiDataUtilities::getData(cgi, "addressFormatStr");
	std::string dataFormatStr = CgiDataUtilities::getData(cgi, "dataFormatStr");

	SOAPParameters parameters; //params for xoap to send
	parameters.addParameter("Request", "UniversalRead");
	parameters.addParameter("Address",Address);

	SOAPParameters retParameters;
	retParameters.addParameter("dataResult");
	__MOUT__ <<"Here comes the array from multiselect box for READ, behold: "
			<< supervisorIndexArray << "," << interfaceIndexArray << std::endl;

	SupervisorDescriptors FESupervisors = theSupervisorsConfiguration_.getFEDescriptors();

	////////////////////////////////Store cgi arrays into vectors/////////////////////////////
	std::vector<std::string> interfaceIndices;
	std::istringstream f(interfaceIndexArray);
	std::string s;
	while (getline(f, s, ',')) interfaceIndices.push_back(s);
	std::vector<int> supervisorIndices;
	std::istringstream g(supervisorIndexArray);
	std::string t;
	while (getline(g, t, ',')) supervisorIndices.push_back(std::stoi(t));

	for(unsigned int i=0; i < supervisorIndices.size(); i++)
	{
		unsigned int FEIndex = supervisorIndices[i];
		std::string interfaceIndex = interfaceIndices[i];

		parameters.addParameter("InterfaceID",interfaceIndex);


		__MOUT__ <<"The index of the supervisor instance is: " << FEIndex << std::endl;
		__MOUT__ <<"...and the interface ID is: " << interfaceIndexArray << std::endl;

		SupervisorDescriptors::iterator it = FESupervisors.find(FEIndex);
		if (it == FESupervisors.end())
		{
			__MOUT__ << "ERROR!? FE Index doesn't exist" << std::endl;
			return;
		}

		xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
				it->second,
				"MacroMakerSupervisorRequest",
				parameters);

		receive(retMsg,retParameters);
		std::string dataReadResult = retParameters.getValue("dataResult");
		__MOUT__ << "Data reading result received: " << dataReadResult << std::endl;
		xmldoc.addTextElementToData("readData",dataReadResult);
		std::string command = "r:" + Address + ":" + dataReadResult;
		std::string format = addressFormatStr + ":" + dataFormatStr;
		appendCommandToHistory(command,format,time,interfaces);
	}
}

void MacroMakerSupervisor::createMacro(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi)
{
	__MOUT__ << "¡¡¡¡¡¡MacroMaker wants to create a macro!!!!!!!!!" << std::endl;
	std::string Name = CgiDataUtilities::getData(cgi, "Name");
	std::string Sequence = CgiDataUtilities::getData(cgi, "Sequence");
	std::string Time = CgiDataUtilities::getData(cgi, "Time");
	std::string Notes = CgiDataUtilities::getData(cgi, "Notes");
	std::string isMacroPublic = CgiDataUtilities::getData(cgi, "isPublic");

	__MOUT__ << MACROS_DB_PATH << std::endl;

	std::string fileName = Name + ".dat";
	std::string fullPath;
    if (isMacroPublic == "true")  fullPath = (std::string)MACROS_DB_PATH + "publicMacros/" + fileName;
    else fullPath = (std::string)MACROS_DB_PATH + username + "/" + fileName;
	std::cout << fullPath << std::endl;

	std::ofstream macrofile (fullPath.c_str());
	if (macrofile.is_open())
	{
		macrofile << "{\n";
		macrofile << "\"name\":\"" << Name << "\",\n";
		macrofile << "\"sequence\":\"" << Sequence << "\",\n";
		macrofile << "\"time\":\"" << Time << "\",\n";
		macrofile << "\"notes\":\"" << Notes << "\"\n";
		macrofile << "}@" << std::endl;
		macrofile.close();
	}
	else
		__MOUT__ <<  "Unable to open file" << std::endl;
}

void MacroMakerSupervisor::loadMacros(HttpXmlDocument& xmldoc)
{
	DIR *dir;
	struct dirent *ent;
	std::string returnStr = "";
	std::string fullPath = (std::string)MACROS_DB_PATH + username + "/";
	if ((dir = opendir (fullPath.c_str())) != NULL)
	{
		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL)
		{
			/* File name validation check */
			if ((unsigned)strlen(ent->d_name) > 4)
			{
				std::string line;
				std::ifstream read (((fullPath + (std::string)ent->d_name)).c_str());//reading a file
				if (read.is_open())
				{
					std::stringstream buffer;
					while (! read.eof() )
					{
						getline (read,line);
						buffer << line;
					}
					returnStr += buffer.str();
					read.close();
				}
				else
					__MOUT__ << "Unable to open file" << std::endl;
			}
		}
		std::string returnMacroStr = returnStr.substr(0, returnStr.size()-1);

		__MOUT__ <<  "Loading existing macros: " << returnMacroStr << std::endl;

		closedir (dir);
		xmldoc.addTextElementToData("returnMacroStr",returnMacroStr);
	}
	else
	{
		__MOUT__ <<  "Looping through privateMacros folder failed! Wrong directory" << std::endl;
	}
	fullPath = (std::string)MACROS_DB_PATH + "publicMacros/";
	returnStr = "";
	if ((dir = opendir (fullPath.c_str())) != NULL)
	{
	  /* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL)
		{
		/* File name validation check */
			if ((unsigned)strlen(ent->d_name) > 4)
			{
				std::string line;
				std::ifstream read (((fullPath + (std::string)ent->d_name)).c_str());//reading a file
				  if (read.is_open())
				  {
					  std::stringstream buffer;
					  while (! read.eof() )
					  {
						  getline (read,line);
						  buffer << line;
					  }
					  returnStr += buffer.str();
					  read.close();
				  }
				  else
					  std::cout << __COUT_HDR_FL__ << "Unable to open file" << std::endl;
			}
		}
		std::string returnPublicStr = returnStr.substr(0, returnStr.size()-1);
		std::cout << __COUT_HDR_FL__ <<  "Loading existing public macros: " << returnPublicStr << std::endl;
		closedir (dir);
		xmldoc.addTextElementToData("returnPublicStr",returnPublicStr);
	}
	else
	{
		std::cout << fullPath << std::endl;
		__MOUT__ <<  "Looping through MacroData folder failed! Wrong directory" << std::endl;

	}
}

void MacroMakerSupervisor::appendCommandToHistory(std::string Command, std::string Format, std::string Time, std::string Interfaces)
{
	std::string fileName = "history.hist";
	std::string fullPath = (std::string)MACROS_HIST_PATH + username + "/" + fileName;
	std::cout << fullPath << std::endl;
	std::ofstream histfile (fullPath.c_str(),std::ios::app);
	if (histfile.is_open())
	{
		histfile << "{\n";
		histfile << "\"Command\":\"" << Command << "\",\n";
		histfile << "\"Format\":\"" << Format << "\",\n";
		histfile << "\"Time\":\"" << Time << "\",\n";
		histfile << "\"Interfaces\":\"" << Interfaces << "\"\n";
		histfile << "}#" << std::endl;
		histfile.close();
	}
	else
		std::cout << "Unable to open history.hist";
}

void MacroMakerSupervisor::loadHistory(HttpXmlDocument& xmldoc)
{
	std::string line;
	std::string returnStr = "";
	std::string fileName = "history.hist";

	std::ifstream read ((MACROS_HIST_PATH + username + "/" + fileName).c_str());//reading a file
	std::cout << MACROS_HIST_PATH + username + "/" + fileName << std::endl;
	if (read.is_open())
	{
		std::stringstream buffer;
		while (! read.eof() )
		{
			getline (read,line);
			buffer << line;
		}
		returnStr += buffer.str();
		read.close();
		if (returnStr.size() != 0)
		{
			std::string returnHistStr = returnStr.substr(0, returnStr.size()-1);
			__MOUT__ <<  "Loading user history: " << returnHistStr << std::endl;
			xmldoc.addTextElementToData("returnHistStr",returnHistStr);
		}
	}
	else

		__MOUT__ << "Unable to open history.hist" << std::endl;

}

void MacroMakerSupervisor::deleteMacro(HttpXmlDocument& xmldoc,cgicc::Cgicc& cgi)
{
	std::string MacroName = CgiDataUtilities::getData(cgi, "MacroName");
	std::string isMacroPublic = CgiDataUtilities::getData(cgi, "isPublic");

	std::cout << __COUT_HDR_FL__ << MACROS_DB_PATH << std::endl;

	std::string fileName = MacroName + ".dat";
	std::string fullPath;
	if (isMacroPublic == "true")  fullPath = (std::string)MACROS_DB_PATH + "publicMacros/" + fileName;
	else fullPath = (std::string)MACROS_DB_PATH + username + "/" + fileName;
	std::cout << fullPath << std::endl;

	std::remove(fullPath.c_str());
	std::cout << "Successfully deleted " << MacroName;
	xmldoc.addTextElementToData("deletedMacroName",MacroName);
}

void MacroMakerSupervisor::editMacro(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi)
{
	std::string oldMacroName = CgiDataUtilities::getData(cgi, "oldMacroName");
	std::string newMacroName = CgiDataUtilities::getData(cgi, "newMacroName");
	std::string Sequence = CgiDataUtilities::getData(cgi, "Sequence");
	std::string Time = CgiDataUtilities::getData(cgi, "Time");
	std::string Notes = CgiDataUtilities::getData(cgi, "Notes");
	std::string isMacroPublic = CgiDataUtilities::getData(cgi, "isPublic");

	__MOUT__ <<  MACROS_DB_PATH << std::endl;


	std::string fileName = oldMacroName + ".dat";
	std::string fullPath;
	if (isMacroPublic == "true")  fullPath = (std::string)MACROS_DB_PATH + "publicMacros/" + fileName;
	else fullPath = (std::string)MACROS_DB_PATH + username + "/" + fileName;
	std::cout << fullPath << std::endl;

	std::ofstream macrofile (fullPath.c_str());
	if (macrofile.is_open())
	{
		macrofile << "{\n";
		macrofile << "\"name\":\"" << newMacroName << "\",\n";
		macrofile << "\"sequence\":\"" << Sequence << "\",\n";
		macrofile << "\"time\":\"" << Time << "\",\n";
		macrofile << "\"notes\":\"" << Notes << "\"\n";
		macrofile << "}@" << std::endl;
		macrofile.close();
	}
	else
		__MOUT__ <<  "Unable to open file" << std::endl;

	if(oldMacroName != newMacroName) //renaming macro
	{
		int result;
		result = rename((MACROS_DB_PATH + username + "/" + oldMacroName + ".dat").c_str(), (MACROS_DB_PATH + username + "/" + newMacroName + ".dat").c_str());
		if (result == 0)
			xmldoc.addTextElementToData("newMacroName",newMacroName);
		else
			xmldoc.addTextElementToData("newMacroName","ERROR");
	}
}

void MacroMakerSupervisor::clearHistory()
{
	std::string fileName = "history.hist";
	std::string fullPath = (std::string)MACROS_HIST_PATH + username + "/" + fileName;

	std::remove(fullPath.c_str());
	std::cout << __COUT_HDR_FL__ << "Successfully deleted " << fullPath;
}

void MacroMakerSupervisor::exportMacro(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi)
{
	std::string macroName = CgiDataUtilities::getData(cgi, "MacroName");
	std::string macroSequence = CgiDataUtilities::getData(cgi, "MacroSequence");
	std::cout<< __COUT_HDR_FL__ << "Sequence: "<< macroSequence << std::endl;
	std::stringstream ss(macroSequence);
	std::string command;
	std::vector<std::string> Commands;

	while (getline(ss, command, ','))  Commands.push_back(command);

	std::string fileName = macroName + ".cc";

	std::string fullPath = (std::string)MACROS_EXPORT_PATH + username + "/" + fileName;
	std::cout << fullPath << std::endl;
	std::ofstream exportFile (fullPath.c_str(),std::ios::app);
	if (exportFile.is_open())
	{
		for(unsigned int i = 0; i < Commands.size(); i++)
		{
			std::stringstream sst(Commands[i]);
			std::string tokens;
			std::vector<std::string> oneCommand;
			while (getline(sst, tokens, ':'))  oneCommand.push_back(tokens);
			std::cout << oneCommand[1] << oneCommand[2] << std::endl;
            		if (oneCommand[1] == "w")
            			exportFile << "universalWrite(" << oneCommand[2] << "," << oneCommand[3] << ");\n";
           		else if (oneCommand[1] == "r")
            			exportFile << "universalRead(" << oneCommand[2] << ",data);\n";
            		else if (oneCommand[1] == "d")
            			exportFile << "delay(" << oneCommand[2] << ");\n";
            		else
            			std::cout << __COUT_HDR_FL__ << "FATAL ERROR: command is not w, r or d" << std::endl;
		}
		exportFile.close();
	}
	else
		std::cout << "Unable to open file";
}
