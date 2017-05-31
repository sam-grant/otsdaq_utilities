#include "otsdaq-utilities/MacroMaker/MacroMakerSupervisor.h"

#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/Macros/CoutHeaderMacros.h"
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"
#include "otsdaq-core/SOAPUtilities/SOAPUtilities.h"
#include "otsdaq-core/SOAPUtilities/SOAPParameters.h"
#include "otsdaq-core/ConfigurationDataFormats/ConfigurationGroupKey.h"
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

#define MACROS_DB_PATH 					std::string(getenv("SERVICE_DATA_PATH")) + "/MacroData/"
#define MACROS_HIST_PATH 				std::string(getenv("SERVICE_DATA_PATH")) + "/MacroHistory/"
#define MACROS_EXPORT_PATH 				std::string(getenv("SERVICE_DATA_PATH")) + "/MacroExport/"

using namespace ots;

#undef 	__MF_SUBJECT__
#define __MF_SUBJECT__ "MacroMaker"

XDAQ_INSTANTIATOR_IMPL(MacroMakerSupervisor)

//========================================================================================================================
MacroMakerSupervisor::MacroMakerSupervisor(xdaq::ApplicationStub* stub)
throw (xdaq::exception::Exception)
: xdaq::Application (stub)
, SOAPMessenger     (this)
, theRemoteWebUsers_(this)
{
	INIT_MF("MacroMaker");
	xgi::bind (this, &MacroMakerSupervisor::Default,                	"Default" 			);
	xgi::bind (this, &MacroMakerSupervisor::MacroMakerRequest,          "MacroMakerRequest" );

	init();
	SupervisorDescriptors::const_iterator it;
	it = theSupervisorDescriptorInfo_.getFEDescriptors().begin();
	__MOUT__ << "PixelFESupervisor instance size " <<
			theSupervisorDescriptorInfo_.getFEDescriptors().size() << std::endl;
	for (; it != theSupervisorDescriptorInfo_.getFEDescriptors().end(); it++)
	{
		__MOUT__ << "PixelFESupervisor instance " << it->first <<
				"...and..." << it->second << std::endl;
		__MOUT__ << "Look! Here's a FE! @@@" << std::endl;
	}

	//make macro directories in case they don't exist
	mkdir(((std::string)MACROS_DB_PATH).c_str(), 0755);
	mkdir(((std::string)MACROS_HIST_PATH).c_str(), 0755);
	mkdir(((std::string)MACROS_EXPORT_PATH).c_str(), 0755);


	//Push the FE Supervisor types that MacroMaker cares about
	//	when scanninng for all FEs:
	//		- FEDescriptors
	//		- FEDataManagerDescriptors
	//		- ARTDAQFEDataManagerDescriptors
	FESupervisorLists_.insert(std::pair<std::string, const SupervisorDescriptors&>(
			"FEDescriptors",
			theSupervisorDescriptorInfo_.getFEDescriptors()));
	FESupervisorLists_.insert(std::pair<std::string, const SupervisorDescriptors&>(
			"FEDataManagerDescriptors",
			theSupervisorDescriptorInfo_.getFEDataManagerDescriptors()));
	FESupervisorLists_.insert(std::pair<std::string, const SupervisorDescriptors&>(
			"ARTDAQFEDataManagerDescriptors",
			theSupervisorDescriptorInfo_.getARTDAQFEDataManagerDescriptors()));


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
	theSupervisorDescriptorInfo_.init(getApplicationContext());
	__MOUT__ << "#######################################" << std::endl;
	__MOUT__ << "#######################################" << std::endl;

	__MOUT__ << "Running in MacroMaker Supervisor" << std::endl;

	__MOUT__ << "#######################################" << std::endl;
	__MOUT__ << "#######################################" << std::endl;


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
	__MOUT__<< "Command: " << Command << std::endl;

	//FIXME -- need to lock out MacroMaker vs State machine

	HttpXmlDocument xmldoc;
	uint64_t activeSessionIndex;
	uint8_t userPermissions;
	std::string username;

	//**** start LOGIN GATEWAY CODE ***//
	{
		bool automaticCommand = Command == "RefreshLogbook"; //automatic commands should not refresh cookie code.. only user initiated commands should!
		bool checkLock = true;
		bool getUser = (Command == "CreateExperiment") || (Command == "RemoveExperiment") ||
				(Command == "PreviewEntry") || (Command == "AdminRemoveRestoreEntry");
		bool requireLock = false;

		if(!theRemoteWebUsers_.xmlLoginGateway(
				cgi,
				out,
				&xmldoc,
				theSupervisorDescriptorInfo_,
				&userPermissions,  		//acquire user's access level (optionally null pointer)
				!automaticCommand,			//true/false refresh cookie code
				1, //set access level requirement to pass gateway
				checkLock,					//true/false enable check that system is unlocked or this user has the lock
				requireLock,				//true/false requires this user has the lock to proceed
				0,//&userWithLock,			//acquire username with lock (optionally null pointer)
				//(getUser?&user:0),				//acquire username of this user (optionally null pointer)
				&username,
				0,						//acquire user's Display Name
				&activeSessionIndex		//acquire user's session index associated with the cookieCode
		))
		{	//failure
			__MOUT__<< "Failed Login Gateway: " <<
					out->str() << std::endl; //print out return string on failure
			return;
		}
	}
	//**** end LOGIN GATEWAY CODE ***//


	__MOUT__<< "User name is " << username << "!!!" << std::endl;
	__MOUT__<< "User Permission is " << unsigned(userPermissions) << "!!!" << std::endl;

	//create macro maker folders for the user (the first time a user authenticates with macro maker)
	std::string macroPath = (std::string)MACROS_DB_PATH + username + "/";
	mkdir(macroPath.c_str(), 0755);
	std::string histPath = (std::string)MACROS_HIST_PATH + username + "/";
	mkdir(histPath.c_str(), 0755);
	std::string publicPath = (std::string)MACROS_DB_PATH + "publicMacros/";
	mkdir(publicPath.c_str(), 0755);
	std::string exportPath = (std::string)MACROS_EXPORT_PATH + username + "/";
	mkdir(exportPath.c_str(), 0755);

	//hanle request per Command
	if(Command == "getPermission")
		xmldoc.addTextElementToData("Permission", std::to_string(unsigned(userPermissions)));
	else
		handleRequest(Command,xmldoc,cgi,username,userPermissions);

	//return xml doc holding server response
	xmldoc.outputXmlDocument((std::ostringstream*) out, false);
}

//========================================================================================================================
void MacroMakerSupervisor::handleRequest(const std::string Command,
		HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi,
		const std::string &username,
		const uint8_t userPermissions)
{
	if(Command 				== "FElist")
		getFElist(xmldoc);
	else if(Command 		== "writeData")
		writeData(xmldoc,cgi,username);
	else if(Command 		== "readData")
		readData(xmldoc,cgi,username);
	else if(Command 		== "createMacro")
		createMacro(xmldoc,cgi,username);
	else if(Command 		== "loadMacros")
		loadMacros(xmldoc,username);
	else if(Command 		== "loadHistory")
		loadHistory(xmldoc,username);
	else if(Command 		== "deleteMacro")
		deleteMacro(xmldoc,cgi,username);
	else if(Command 		== "editMacro")
		editMacro(xmldoc,cgi,username);
	else if(Command 		== "clearHistory")
		clearHistory(username);
	else if(Command 		== "exportMacro")
		exportMacro(xmldoc,cgi,username);
	else if(Command 		== "getFEMacroList")
		getFEMacroList(xmldoc,username);
	else if(Command 		== "runFEMacro")
		runFEMacro(xmldoc,cgi);
	else
		xmldoc.addTextElementToData("Error","Unrecognized command '" + Command + "'");
}

//========================================================================================================================
void MacroMakerSupervisor::getFElist(HttpXmlDocument& xmldoc)
{
	__MOUT__<< "Getting FE list!!!!!!!!!" << std::endl;

	SOAPParameters txParameters; //params for xoap to send
	txParameters.addParameter("Request", "GetInterfaces");

	SOAPParameters rxParameters;  //params for xoap to recv
	rxParameters.addParameter("FEList");

	SupervisorDescriptors::const_iterator it;
	std::string oneInterface;
	std::string rxFEList;

	//for each list of FE Supervisors,
	//	loop through each FE Supervisors and get FE interfaces list
	for(auto &listPair:FESupervisorLists_)
	{
		__MOUT__ << "Number of " << listPair.first << " = " <<
				listPair.second.size() << std::endl;

		for (it = listPair.second.begin(); it != listPair.second.end(); it++)
		{
			__MOUT__ << "FESupervisor LID " << it->first << std::endl;

			xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
					it->second,
					"MacroMakerSupervisorRequest",
					txParameters);
			receive(retMsg, rxParameters);
			rxFEList = rxParameters.getValue("FEList");

			__MOUT__ << "FE List received: \n" << rxFEList << std::endl;

			std::istringstream allInterfaces(rxFEList);
			while (std::getline(allInterfaces, oneInterface))
			{
				//interfaceList.push_back(oneInterface);
				xmldoc.addTextElementToData("FE",oneInterface);
			}
		}
	}

	return;
}


//========================================================================================================================
void MacroMakerSupervisor::writeData(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username)
{
	__MOUT__<< "¡¡¡¡¡¡MacroMaker wants to write data!!!!!!!!!" << std::endl;
	std::string Address = CgiDataUtilities::getData(cgi, "Address");
	std::string Data = CgiDataUtilities::getData(cgi, "Data");
	std::string interfaceIndexArray = CgiDataUtilities::getData(cgi, "interfaceIndex");
	std::string supervisorIndexArray = CgiDataUtilities::getData(cgi, "supervisorIndex");
	std::string time = CgiDataUtilities::decodeURIComponent(CgiDataUtilities::getData(cgi, "time"));
	std::string interfaces = CgiDataUtilities::getData(cgi, "interfaces");
	std::string addressFormatStr = CgiDataUtilities::getData(cgi, "addressFormatStr");
	std::string dataFormatStr = CgiDataUtilities::getData(cgi, "dataFormatStr");

	std::string command = "w:" + Address + ":" + Data;
	std::string format = addressFormatStr + ":" + dataFormatStr;
	appendCommandToHistory(command,format,time,interfaces,username);

	SOAPParameters txParameters; //params for xoap to send
	txParameters.addParameter("Request", "UniversalWrite");
	txParameters.addParameter("Address",Address);
	txParameters.addParameter("Data",Data);

	__MOUT__<< "Address: " << Address << " Data: " << Data << std::endl;

	__MOUT__<< "Here comes the array from multiselect box for WRITE, behold: \n"
			<< supervisorIndexArray << "\n" << interfaceIndexArray << std::endl;

	SupervisorDescriptors FESupervisors = theSupervisorDescriptorInfo_.getFEDescriptors();

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

		txParameters.addParameter("InterfaceID",interfaceIndex);

		__MOUT__<<"The index of the supervisor instance is: " << FEIndex << std::endl;
		__MOUT__<<"...and the interface ID is: " << interfaceIndex << std::endl;

		SupervisorDescriptors::iterator it = FESupervisors.find(FEIndex);
		if (it == FESupervisors.end())
		{
			__MOUT__<< "ERROR!? FE Index doesn't exist" << std::endl;
			return;
		}

		xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
				it->second,
				"MacroMakerSupervisorRequest",
				txParameters);
		receive(retMsg);
	}
}

//========================================================================================================================
void MacroMakerSupervisor::readData(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username)
{
	__MOUT__<< "@@@@@@@ MacroMaker wants to read data @@@@@@@@" << std::endl;
	std::string Address = CgiDataUtilities::getData(cgi, "Address");
	std::string interfaceIndexArray = CgiDataUtilities::getData(cgi, "interfaceIndex");
	std::string supervisorIndexArray = CgiDataUtilities::getData(cgi, "supervisorIndex");
	std::string time = CgiDataUtilities::decodeURIComponent(CgiDataUtilities::getData(cgi, "time"));
	std::string interfaces = CgiDataUtilities::getData(cgi, "interfaces");
	std::string addressFormatStr = CgiDataUtilities::getData(cgi, "addressFormatStr");
	std::string dataFormatStr = CgiDataUtilities::getData(cgi, "dataFormatStr");

	SOAPParameters txParameters; //params for xoap to send
	txParameters.addParameter("Request", "UniversalRead");
	txParameters.addParameter("Address",Address);

	SOAPParameters rxParameters;
	rxParameters.addParameter("dataResult");
	__MOUT__<<"Here comes the array from multiselect box for READ, behold: "
			<< supervisorIndexArray << "," << interfaceIndexArray << std::endl;

	SupervisorDescriptors FESupervisors = theSupervisorDescriptorInfo_.getFEDescriptors();

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

		txParameters.addParameter("InterfaceID",interfaceIndex);


		__MOUT__<<"The index of the supervisor instance is: " << FEIndex << std::endl;
		__MOUT__<<"...and the interface ID is: " << interfaceIndexArray << std::endl;

		SupervisorDescriptors::iterator it = FESupervisors.find(FEIndex);
		if (it == FESupervisors.end())
		{
			__MOUT__<< "ERROR!? FE Index doesn't exist" << std::endl;
			return;
		}

		xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
				it->second,
				"MacroMakerSupervisorRequest",
				txParameters);

		receive(retMsg,rxParameters);
		std::string dataReadResult = rxParameters.getValue("dataResult");
		__MOUT__<< "Data reading result received: " << dataReadResult << std::endl;
		xmldoc.addTextElementToData("readData",dataReadResult);
		std::string command = "r:" + Address + ":" + dataReadResult;
		std::string format = addressFormatStr + ":" + dataFormatStr;
		appendCommandToHistory(command,format,time,interfaces,username);
	}
}

//========================================================================================================================
void MacroMakerSupervisor::createMacro(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username)
{
	__MOUT__<< "¡¡¡¡¡¡MacroMaker wants to create a macro!!!!!!!!!" << std::endl;
	std::string Name = CgiDataUtilities::getData(cgi, "Name");
	std::string Sequence = CgiDataUtilities::getData(cgi, "Sequence");
	std::string Time = CgiDataUtilities::decodeURIComponent(CgiDataUtilities::getData(cgi, "Time"));
	std::string Notes = CgiDataUtilities::decodeURIComponent(CgiDataUtilities::getData(cgi, "Notes"));
	std::string isMacroPublic = CgiDataUtilities::getData(cgi, "isPublic");
	std::string isMacroLSBF = CgiDataUtilities::getData(cgi, "isLSBF");


	__MOUT__<< MACROS_DB_PATH << std::endl;

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
		macrofile << "\"notes\":\"" << Notes << "\",\n";
		macrofile << "\"LSBF\":\"" << isMacroLSBF << "\"\n";
		macrofile << "}@" << std::endl;
		macrofile.close();
	}
	else
		__MOUT__<<  "Unable to open file" << std::endl;
}

//========================================================================================================================
void MacroMakerSupervisor::loadMacros(HttpXmlDocument& xmldoc, const std::string &username)
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
						//__MOUT__ << line << std::endl;
					}
					returnStr += buffer.str();

					read.close();
				}
				else
					__MOUT__<< "Unable to open file" << std::endl;
			}
		}
		std::string returnMacroStr = returnStr.substr(0, returnStr.size()-1);

		__MOUT__<<  "Loading existing macros! " << returnMacroStr << std::endl;

		closedir (dir);
		xmldoc.addTextElementToData("returnMacroStr",returnMacroStr);
	}
	else
	{
		__MOUT__<<  "Looping through privateMacros folder failed! Wrong directory" << std::endl;
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
						//__MOUT__ << line << std::endl;
					}
					returnStr += buffer.str();
					read.close();
				}
				else
					__MOUT__<< "Unable to open file" << std::endl;
			}
		}
		std::string returnPublicStr = returnStr.substr(0, returnStr.size()-1);
		__MOUT__<<  "Loading existing public macros: " << returnPublicStr << std::endl;
		closedir (dir);
		xmldoc.addTextElementToData("returnPublicStr",returnPublicStr);
	}
	else
	{
		std::cout << fullPath << std::endl;
		__MOUT__<<  "Looping through MacroData folder failed! Wrong directory" << std::endl;

	}
}

//========================================================================================================================
void MacroMakerSupervisor::appendCommandToHistory(std::string Command,
		std::string Format, std::string Time, std::string Interfaces, const std::string &username)
{
	std::string fileName = "history.hist";
	std::string fullPath = (std::string)MACROS_HIST_PATH + username + "/" + fileName;
	__MOUT__ << fullPath << std::endl;
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
		__MOUT__<< "Unable to open history.hist" << std::endl;
}

//========================================================================================================================
void MacroMakerSupervisor::loadHistory(HttpXmlDocument& xmldoc, const std::string &username)
{
	std::string fileName = MACROS_HIST_PATH + username + "/" + "history.hist";

	std::ifstream read (fileName.c_str());//reading a file
	__MOUT__<<  fileName << std::endl;

	if (read.is_open())
	{
		std::string line;
		char * returnStr;
		unsigned long long fileSz, i = 0, MAX_HISTORY_SIZE = 100;


		//get length of file to reserve the string size
		//	and to cap history size
		read.seekg(0, std::ios::end);
		fileSz = read.tellg();
		returnStr = new char[fileSz];
		read.seekg(0, std::ios::beg);


	    // read data as a block:
	    read.read(returnStr,fileSz);
		read.close();


		//find i such that new string size is less than
		if(fileSz > MAX_HISTORY_SIZE)
		{
			i = fileSz - MAX_HISTORY_SIZE;
			for(;i<fileSz;++i)
				if(returnStr[i] == '#')
				{
					i += 2; break; //skip new line character also to get to next record
				}

			//write back to file truncated history
			FILE *fp = fopen(fileName.c_str(),"w");
			if(!fp)
			{
				__SS__ << "Big problem with macromaker history file: " << fileName << std::endl;
				throw std::runtime_error(ss.str());
			}
			fwrite(&returnStr[i],fileSz-i,1,fp);
			fclose(fp);
		}

		__MOUT__<<  "Loading user history! " << std::endl;

		if(fileSz > 1)
			returnStr[fileSz-2] = '\0'; //remove final newline and last #

		xmldoc.addTextElementToData("returnHistStr",&returnStr[i]);


		delete[] returnStr;
	}
	else

		__MOUT__<< "Unable to open history.hist" << std::endl;

}

//========================================================================================================================
void MacroMakerSupervisor::deleteMacro(HttpXmlDocument& xmldoc,cgicc::Cgicc& cgi, const std::string &username)
{
	std::string MacroName = CgiDataUtilities::getData(cgi, "MacroName");
	std::string isMacroPublic = CgiDataUtilities::getData(cgi, "isPublic");


	std::string fileName = MacroName + ".dat";
	std::string fullPath;
	if (isMacroPublic == "true")  fullPath = (std::string)MACROS_DB_PATH + "publicMacros/" + fileName;
	else fullPath = (std::string)MACROS_DB_PATH + username + "/" + fileName;

	__MOUT__<< fullPath << std::endl;

	std::remove(fullPath.c_str());
	std::cout << "Successfully deleted " << MacroName;
	xmldoc.addTextElementToData("deletedMacroName",MacroName);
}

//========================================================================================================================
void MacroMakerSupervisor::editMacro(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username)
{
	std::string oldMacroName = CgiDataUtilities::getData(cgi, "oldMacroName");
	std::string newMacroName = CgiDataUtilities::getData(cgi, "newMacroName");
	std::string Sequence = CgiDataUtilities::getData(cgi, "Sequence");
	std::string Time = CgiDataUtilities::decodeURIComponent(CgiDataUtilities::getData(cgi, "Time"));
	std::string Notes = CgiDataUtilities::decodeURIComponent(CgiDataUtilities::getData(cgi, "Notes"));
	std::string isMacroPublic = CgiDataUtilities::getData(cgi, "isPublic");
	std::string isMacroLSBF = CgiDataUtilities::getData(cgi, "isLSBF");

	std::string fileName = oldMacroName + ".dat";
	std::string fullPath;
	if (isMacroPublic == "true")  fullPath = (std::string)MACROS_DB_PATH + "publicMacros/" + fileName;
	else fullPath = (std::string)MACROS_DB_PATH + username + "/" + fileName;

	__MOUT__<<  fullPath << std::endl;

	std::ofstream macrofile (fullPath.c_str());
	if (macrofile.is_open())
	{
		macrofile << "{\n";
		macrofile << "\"name\":\"" << newMacroName << "\",\n";
		macrofile << "\"sequence\":\"" << Sequence << "\",\n";
		macrofile << "\"time\":\"" << Time << "\",\n";
		macrofile << "\"notes\":\"" << Notes << "\",\n";
		macrofile << "\"LSBF\":\"" << isMacroLSBF << "\"\n";
		macrofile << "}@" << std::endl;
		macrofile.close();
	}
	else
		__MOUT__<<  "Unable to open file" << std::endl;

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

//========================================================================================================================
void MacroMakerSupervisor::clearHistory(const std::string &username)
{
	std::string fileName = "history.hist";
	std::string fullPath = (std::string)MACROS_HIST_PATH + username + "/" + fileName;

	std::remove(fullPath.c_str());
	__MOUT__<< "Successfully deleted " << fullPath;
}

//========================================================================================================================
void MacroMakerSupervisor::exportMacro(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username)
{
	std::string macroName = CgiDataUtilities::getData(cgi, "MacroName");
	std::string macroSequence = CgiDataUtilities::getData(cgi, "MacroSequence");
	std::stringstream ss(macroSequence);
	std::string command;
	std::vector<std::string> Commands;

	while (getline(ss, command, ','))  Commands.push_back(command);

	std::string fileName = macroName + ".cc";

	int numOfHexBytes;
	std::string hexInitStr;

	std::string fullPath = (std::string)MACROS_EXPORT_PATH + username + "/" + fileName;
	std::cout << fullPath << std::endl;
	std::ofstream exportFile (fullPath.c_str(),std::ios::trunc);
	if (exportFile.is_open())
	{
		exportFile << "//Generated Macro Name:\t" << macroName << "\n";

		{
			time_t rawtime;
			struct tm * timeinfo;
			char buffer[100];

			time (&rawtime);
			timeinfo = localtime(&rawtime);

			strftime(buffer,100,"%b-%d-%Y %I:%M:%S",timeinfo);
			exportFile << "//Generated Time: \t\t" << buffer << "\n";
		}

		exportFile << "//Paste this whole file into an interface to transfer Macro functionality.\n";
		exportFile << "{\n";

		exportFile << "\n\tuint8_t addrs[universalAddressSize_];	//create address buffer of interface size";
		exportFile << "\n\tuint8_t data[universalDataSize_];		//create data buffer of interface size";

		for(unsigned int i = 0; i < Commands.size(); i++)
		{
			std::stringstream sst(Commands[i]);
			std::string tokens;
			std::vector<std::string> oneCommand;
			while (getline(sst, tokens, ':'))  oneCommand.push_back(tokens);
			std::cout << oneCommand[1] << oneCommand[2] << std::endl;

			//make this:
			//			{
			//				uint8_t addrs[universalAddressSize_];	//create address buffer of interface size
			//				uint8_t macroAddrs[8] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x09}; //create macro address buffer
			//				for(unsigned int i=0;i<universalAddressSize_;++i) //fill with macro address and 0 fill
			//					addrs[i] = (i < 8)?macroAddrs[i]:0;
			//				uint8_t data[universalDataSize_];		//create data buffer of interface size
			//				uint8_t macroData[8] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x09}; //create macro data buffer
			//				for(unsigned int i=0;i<universalDataSize_;++i) //fill with macro data and 0 fill
			//					data[i] = (i < 8)?macroData[i]:0;
			//
			//				universalWrite(addrs,data);
			//			}
			//
			//			//if variable
			//			{
			//				uint8_t addrs[universalAddressSize_];
			//				uint64_t macroAddrs = theXDAQContextConfigTree_.getNode(theConfigurationPath_).getNode("variableName").getValue<uint64_t>();
			//				for(unsigned int i=0;i<universalAddressSize_;++i) //fill with macro address and 0 fill
			//					addrs[i] = (i < 8)?((uint8_t *)(&macroAddrs))[i]:0;
			//			}

			exportFile << "\n\n\t// ";
			if (oneCommand[1] == "w")
				exportFile << "universalWrite(0x" << oneCommand[2] << ",0x" << oneCommand[3] << ");\n";
			else if (oneCommand[1] == "r")
				exportFile << "universalRead(0x" << oneCommand[2] << ",data);\n";
			else if (oneCommand[1] == "d")
			{
				exportFile << "delay(" << oneCommand[2] << ");\n";
				exportFile << "sleep(" << oneCommand[2] << ");\n";
				continue;
			}
			else
			{
				__MOUT_ERR__<< "FATAL ERROR: command is not w, r or d" << std::endl;
				continue;
			}

			hexInitStr = generateHexArray(oneCommand[2],numOfHexBytes);

			exportFile << "\t{";
			if(numOfHexBytes == -1) //handle as variable
			{
				exportFile << "\n\t\tuint64_t macroAddrs = theXDAQContextConfigTree_.getNode(theConfigurationPath_).getNode(" <<
						"\n\t\t\t\"" <<
						oneCommand[2] << "\").getValue<uint64_t>();";
				exportFile << "\t//create macro address buffer";
				exportFile << "\n\t\tfor(unsigned int i=0;i<universalAddressSize_;++i) //fill with macro address and 0 fill";
				exportFile << "\n\t\t\t\taddrs[i] = (i < 8)?((uint8_t *)(&macroAddrs))[i]:0;";
			}
			else	//handle as literal
			{
				exportFile << "\n\t\tuint8_t macroAddrs" <<
						hexInitStr <<
						"\t//create macro address buffer";
				exportFile << "\n\t\tfor(unsigned int i=0;i<universalAddressSize_;++i) //fill with macro address and 0 fill";
				exportFile << "\n\t\t\t\taddrs[i] = (i < " << numOfHexBytes <<
						")?macroAddrs[i]:0;";
			}
			exportFile << "\n";

			if (oneCommand[1] == "w") //if write, handle data too
			{

				hexInitStr = generateHexArray(oneCommand[3],numOfHexBytes);

				if(numOfHexBytes == -1) //handle as variable
				{
					exportFile << "\n\t\tuint64_t macroData = theXDAQContextConfigTree_.getNode(theConfigurationPath_).getNode(" <<
							"\n\t\t\t\"" <<
							oneCommand[3] << "\").getValue<uint64_t>();";
					exportFile << "\t//create macro data buffer";
					exportFile << "\n\t\tfor(unsigned int i=0;i<universalDataSize_;++i) //fill with macro address and 0 fill";
					exportFile << "\n\t\t\t\tdata[i] = (i < 8)?((uint8_t *)(&macroData))[i]:0;";
				}
				else //handle as literal
				{
					exportFile << "\n\t\tuint8_t macroData" <<
							hexInitStr <<
							"\t//create macro data buffer";
					exportFile << "\n\t\tfor(unsigned int i=0;i<universalDataSize_;++i) //fill with macro address and 0 fill";
					exportFile << "\n\t\t\t\tdata[i] = (i < " << numOfHexBytes <<
							")?macroData[i]:0;";
				}
				exportFile << "\n\t\tuniversalWrite((char *)addrs,(char *)data);";
			}
			else
				exportFile << "\n\t\tuniversalRead((char *)addrs,(char *)data);";
			exportFile << "\n\t}";
		}

		exportFile << "\n}";
		exportFile.close();

		xmldoc.addTextElementToData("ExportFile",fullPath);
	}
	else
		__MOUT__ << "Unable to open file" << std::endl;
}


//========================================================================================================================
//generateHexArray
//	returns a char array initializer
//	something like this
//	"[8] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x09};"
//		..depending a size of source string
//
//FIXME -- identify variables in a better way from macromaker...!
//	for now just assume a non hex is a variable name
//	return -1 size
std::string MacroMakerSupervisor::generateHexArray(const std::string &sourceHexString,
		int &numOfBytes)
{
	std::stringstream retSs;

	std::string srcHexStr = sourceHexString;
	__MOUT__<< "Translating: \n";
	__MOUT__ << srcHexStr << std::endl;

	if(srcHexStr.size()%2) //if odd, make even
		srcHexStr = "0" + srcHexStr;

	numOfBytes = srcHexStr.size()/2;
	retSs << "[" << numOfBytes << "] = {";

	for(int i=0; i<numOfBytes*2; i+=2)
	{
		//detect non-hex
		if(!((srcHexStr[i] >= '0' && srcHexStr[i] <= '9') ||
				(srcHexStr[i] >= 'a' && srcHexStr[i] <= 'f')||
				(srcHexStr[i] >= 'A' && srcHexStr[i] <= 'F')) ||
				!((srcHexStr[i+1] >= '0' && srcHexStr[i+1] <= '9') ||
						(srcHexStr[i+1] >= 'a' && srcHexStr[i+1] <= 'f')||
						(srcHexStr[i+1] >= 'A' && srcHexStr[i+1] <= 'F'))
		)
		{
			numOfBytes = -1;
			return srcHexStr;
		}

		if(i != 0) retSs << ", ";
		retSs << "0x" <<
				srcHexStr[srcHexStr.size()-1-i-1] <<
				srcHexStr[srcHexStr.size()-1-i];
	}
	retSs << "};";

	__MOUT__ << retSs.str() << std::endl;

	return retSs.str();
}

//========================================================================================================================
void MacroMakerSupervisor::runFEMacro(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi)
{
	__MOUT__<< __COUT_HDR_P__ << std::endl;

	std::string feSupervisorType = CgiDataUtilities::getData(cgi, "feSupervisorType");
	unsigned int supervisorLID = CgiDataUtilities::getDataAsInt(cgi, "supervisorLID");
	std::string feUID = CgiDataUtilities::getData(cgi, "feUID");
	std::string macroName = CgiDataUtilities::getData(cgi, "macroName");
	std::string inputArgs = CgiDataUtilities::postData(cgi, "inputArgs");
	std::string outputArgs = CgiDataUtilities::postData(cgi, "outputArgs");

	__MOUT__ << "feSupervisorType = " << feSupervisorType << std::endl;
	__MOUT__ << "supervisorLID = " << supervisorLID << std::endl;
	__MOUT__ << "feUID = " << feUID << std::endl;
	__MOUT__ << "macroName = " << macroName << std::endl;
	__MOUT__ << "inputArgs = " << inputArgs << std::endl;
	__MOUT__ << "outputArgs = " << outputArgs << std::endl;

	//send command to chosen FE and await response
	SOAPParameters txParameters; //params for xoap to send
	txParameters.addParameter("Request", "RunInterfaceMacro");
	txParameters.addParameter("InterfaceID", feUID);
	txParameters.addParameter("feMacroName", macroName);
	txParameters.addParameter("inputArgs", inputArgs);
	txParameters.addParameter("outputArgs", outputArgs);

	SOAPParameters rxParameters;  //params for xoap to recv
	rxParameters.addParameter("success");
	rxParameters.addParameter("outputArgs");

	//find feSupervisorType in lists
	auto supervisorListPairIt = FESupervisorLists_.find(feSupervisorType);
	if(supervisorListPairIt == FESupervisorLists_.end())
	{
		__SS__ << "Targeted Supervisor Descriptor was not found. Attempted target " <<
				"was feSupervisorType=" << feSupervisorType << std::endl;
		__MOUT_ERR__ << "\n" << ss.str();
		xmldoc.addTextElementToData("Error",ss.str());
		return;
	}

	//find supervisorLID in target list
	auto supervisorDescriptorPairIt = supervisorListPairIt->second.find(supervisorLID);
	if(supervisorDescriptorPairIt == supervisorListPairIt->second.end())
	{
		__SS__ << "Targeted Supervisor Descriptor was not found. Attempted target" <<
				"was feSupervisorType=" << feSupervisorType << " and " <<
				"supervisorLID=" << supervisorLID << std::endl;
		__MOUT_ERR__ << "\n" << ss.str();
		xmldoc.addTextElementToData("Error",ss.str());
		return;
	}

	//have FE supervisor descriptor, so send
	xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
			supervisorDescriptorPairIt->second, //supervisor descriptor
			"MacroMakerSupervisorRequest",
			txParameters);
	SOAPMessenger::receive(retMsg, rxParameters);

	__MOUT__ << "Received it " << std::endl;

	bool success = rxParameters.getValue("success") == "1";
	outputArgs = rxParameters.getValue("outputArgs");

	__MOUT__ << "rx success = " << success << std::endl;
	__MOUT__ << "outputArgs = " << outputArgs << std::endl;

	if(!success)
	{
		__SS__ << "Attempted FE Macro Failed. Attempted target" <<
				"was feSupervisorType=" << feSupervisorType << " and " <<
				"supervisorLID=" << supervisorLID << std::endl;
		ss << ".\n\n The error was:\n\n" << outputArgs << std::endl;
		__MOUT_ERR__ << "\n" << ss.str();
		xmldoc.addTextElementToData("Error",ss.str());
		return;
	}


	//build output arguments
	//	parse args, colon-separated pairs, and then comma-separated
	{
		std::istringstream inputStream(outputArgs);
		std::string splitVal, argName, argValue;
		while (getline(inputStream, splitVal, ';'))
		{
			std::istringstream pairInputStream(splitVal);
			getline(pairInputStream, argName, ',');
			getline(pairInputStream, argValue, ',');
			xmldoc.addTextElementToData("outputArgs_name",argName);
			xmldoc.addTextElementToData("outputArgs_value",argValue);
			__MOUT__ << argName << ": " << argValue << std::endl;
		}
	}

}

//========================================================================================================================
void MacroMakerSupervisor::getFEMacroList(HttpXmlDocument& xmldoc, const std::string &username)
{
	__MOUT__<< "Getting FE Macro list!!!!!!!!!" << std::endl;

	SOAPParameters txParameters; //params for xoap to send
	txParameters.addParameter("Request", "GetInterfaceMacros");

	SOAPParameters rxParameters;  //params for xoap to recv
	rxParameters.addParameter("FEMacros");

	SupervisorDescriptors::const_iterator it;
	std::string oneInterface;
	std::string rxFEMacros;

	//get all FE specific macros
	//		for each list of FE Supervisors,
	//			loop through each FE Supervisors and get FE interfaces list
	for(auto &listPair:FESupervisorLists_)
	{
		__MOUT__ << "===== Number of " << listPair.first << " = " <<
				listPair.second.size() << std::endl;

		for (it = listPair.second.begin(); it != listPair.second.end(); it++)
		{
			__MOUT__ << "FESupervisor LID " << it->first << std::endl;

			xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
					it->second,
					"MacroMakerSupervisorRequest",
					txParameters);
			SOAPMessenger::receive(retMsg, rxParameters);
			rxFEMacros = rxParameters.getValue("FEMacros");

			__MOUT__ << "FE Macros received: \n" << rxFEMacros << std::endl;

			std::istringstream allInterfaces(rxFEMacros);
			while (std::getline(allInterfaces, oneInterface))
				xmldoc.addTextElementToData("FEMacros",listPair.first + ":" + oneInterface);
		}
	}

	//add macros to response
	loadMacros(xmldoc, username);

	return;
}





