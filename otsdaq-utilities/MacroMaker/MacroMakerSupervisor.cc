#include "otsdaq-utilities/MacroMaker/MacroMakerSupervisor.h"

#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/Macros/CoutMacros.h"
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"
#include "otsdaq-core/SOAPUtilities/SOAPUtilities.h"
#include "otsdaq-core/SOAPUtilities/SOAPParameters.h"
#include "otsdaq-core/ConfigurationDataFormats/ConfigurationGroupKey.h"
#include "otsdaq-core/ConfigurationInterface/ConfigurationManager.h"
#include "otsdaq-core/Macros/CoutMacros.h"


#include "otsdaq-core/CodeEditor/CodeEditor.h"

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
: CoreSupervisorBase(stub)
{
	INIT_MF("MacroMaker");

	//make macro directories in case they don't exist
	mkdir(((std::string)MACROS_DB_PATH).c_str(), 0755);
	mkdir(((std::string)MACROS_HIST_PATH).c_str(), 0755);
	mkdir(((std::string)MACROS_EXPORT_PATH).c_str(), 0755);


	init();
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

	//MacroMaker should consider all FE compatible types..
	allFESupervisorInfo_ = allSupervisorInfo_.getAllFETypeSupervisorInfo();
}

//========================================================================================================================
void MacroMakerSupervisor::destroy(void)
{
	//called by destructor
}

//========================================================================================================================
//forceSupervisorPropertyValues
//		override to force supervisor property values (and ignore user settings)
void MacroMakerSupervisor::forceSupervisorPropertyValues()
{
//	CorePropertySupervisorBase::setSupervisorProperty(CorePropertySupervisorBase::SUPERVISOR_PROPERTIES.NeedUsernameRequestTypes,
//			"getPermission");
}

//========================================================================================================================
void MacroMakerSupervisor::request(const std::string& requestType, cgicc::Cgicc& cgiIn,
		HttpXmlDocument& xmlOut, const WebUsers::RequestUserInfo& userInfo)
try
{
	__COUT__ << "User name is " << userInfo.username_ << "." << std::endl;
	__COUT__ << "User permission level for request '" << requestType << "' is " <<
			unsigned(userInfo.permissionLevel_) << "." << std::endl;


	//handle request per requestType
	if(requestType == "getPermission")
	{
		xmlOut.addTextElementToData("Permission", std::to_string(unsigned(userInfo.permissionLevel_)));

		//create macro maker folders for the user (the first time a user authenticates with macro maker)
		std::string macroPath = (std::string)MACROS_DB_PATH + userInfo.username_ + "/";
		mkdir(macroPath.c_str(), 0755);
		std::string histPath = (std::string)MACROS_HIST_PATH + userInfo.username_ + "/";
		mkdir(histPath.c_str(), 0755);
		std::string publicPath = (std::string)MACROS_DB_PATH + "publicMacros/";
		mkdir(publicPath.c_str(), 0755);
		std::string exportPath = (std::string)MACROS_EXPORT_PATH + userInfo.username_ + "/";
		mkdir(exportPath.c_str(), 0755);
	}
	else
		handleRequest(requestType,xmlOut,cgiIn,userInfo.username_);
}
catch(const std::runtime_error& e)
{
	__SS__ << "Error occurred handling request '" << requestType <<
			"': " << e.what() << __E__;
	__COUT__ << ss.str();
	xmlOut.addTextElementToData("Error",ss.str());
}
catch(...)
{
	__SS__ << "Unknown error occurred handling request '" << requestType <<
			"!'" << __E__;
	__COUT__ << ss.str();
	xmlOut.addTextElementToData("Error",ss.str());
}

//========================================================================================================================
void MacroMakerSupervisor::handleRequest(const std::string Command,
		HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi,
		const std::string &username)
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
	else if(Command 		== "exportFEMacro")
		exportFEMacro(xmldoc,cgi,username);
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
	__COUT__<< "Getting FE list!!!!!!!!!" << std::endl;

	SOAPParameters txParameters; //params for xoap to send
	txParameters.addParameter("Request", "GetInterfaces");

	SOAPParameters rxParameters;  //params for xoap to recv
	rxParameters.addParameter("FEList");

	SupervisorInfoMap::const_iterator it;
	std::string oneInterface;
	std::string rxFEList;

	//for each list of FE Supervisors,
	//	loop through each FE Supervisors and get FE interfaces list
	for(auto &appInfo:allFESupervisorInfo_)
	{
		//		__COUT__ << "Number of " << listPair.first << " = " <<
		//				listPair.second.size() << std::endl;
		//
		//		for (it = listPair.second.begin(); it != listPair.second.end(); it++)
		//		{

		__COUT__ << "FESupervisor LID = " << appInfo.second.getId() <<
				" name = " << appInfo.second.getName() << __E__;

		try
		{
			xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
					appInfo.second.getDescriptor(),
					"MacroMakerSupervisorRequest",
					txParameters);
			receive(retMsg, rxParameters);
		}
		catch(const xdaq::exception::Exception& e)
		{
			__SS__ << "Error transmitting request to FE Supervisor LID = " << appInfo.second.getId() <<
				" name = " << appInfo.second.getName() << ". \n\n" << e.what() << __E__;
			__COUT_ERR__ << ss.str();
			return;
		}

		rxFEList = rxParameters.getValue("FEList");

		__COUT__ << "FE List received: \n" << rxFEList << std::endl;

		std::istringstream allInterfaces(rxFEList);
		while (std::getline(allInterfaces, oneInterface))
		{
			//interfaceList.push_back(oneInterface);
			xmldoc.addTextElementToData("FE",oneInterface);
		}

	}
}


//========================================================================================================================
void MacroMakerSupervisor::writeData(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username)
{
	__COUT__<< "MacroMaker writing..." << std::endl;

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

	__COUT__<< "Address: " << Address << " Data: " << Data << std::endl;

	__COUT__<< "Here comes the array from multiselect box for WRITE, behold: \n"
			<< supervisorIndexArray << "\n" << interfaceIndexArray << std::endl;

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
		unsigned int FESupervisorIndex = supervisorIndices[i];
		std::string interfaceIndex = interfaceIndices[i];

		txParameters.addParameter("InterfaceID",interfaceIndex);

		__COUT__<<"The index of the supervisor instance is: " << FESupervisorIndex << std::endl;
		__COUT__<<"...and the interface ID is: " << interfaceIndex << std::endl;

		SupervisorInfoMap::iterator it = allFESupervisorInfo_.find(FESupervisorIndex);
		if (it == allFESupervisorInfo_.end())
		{
			__COUT__<< "ERROR!? FE Index doesn't exist" << std::endl;
			return;
		}

		xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
				it->second.getDescriptor(),
				"MacroMakerSupervisorRequest",
				txParameters);
		receive(retMsg);
	}
}

//========================================================================================================================
void MacroMakerSupervisor::readData(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username)
{
	__COUT__<< "@@@@@@@ MacroMaker wants to read data @@@@@@@@" << std::endl;
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
	__COUT__<< "Here comes the array from multiselect box for READ, behold: "
			<< supervisorIndexArray << "," << interfaceIndexArray << std::endl;


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


		__COUT__ << "The index of the supervisor instance is: " << FEIndex << std::endl;
		__COUT__ << "...and the interface ID is: " << interfaceIndexArray << std::endl;

		SupervisorInfoMap::iterator it = allFESupervisorInfo_.find(FEIndex);
		if (it == allFESupervisorInfo_.end())
		{
			__COUT__<< "ERROR!? FE Index doesn't exist" << std::endl;
			xmldoc.addTextElementToData("readData","MissingFrontEnd");
			return;
		}

		try
		{
			xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
					it->second.getDescriptor(),
					"MacroMakerSupervisorRequest",
					txParameters);

			receive(retMsg,rxParameters);
		}
		catch(const xdaq::exception::Exception& e)
		{
			__SS__ << "Error transmitting request to FE Supervisor. \n\n" << e.what() << __E__;
			__COUT_ERR__ << ss.str();
			xmldoc.addTextElementToData("readData","NotConfigured");
			return;
		}

		std::string dataReadResult = rxParameters.getValue("dataResult");
		__COUT__<< "Data reading result received: " << dataReadResult << std::endl;
		xmldoc.addTextElementToData("readData",dataReadResult);
		std::string command = "r:" + Address + ":" + dataReadResult;
		std::string format = addressFormatStr + ":" + dataFormatStr;
		appendCommandToHistory(command,format,time,interfaces,username);
	}
}

//========================================================================================================================
void MacroMakerSupervisor::createMacro(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username)
{
	__COUT__<< "¡¡¡¡¡¡MacroMaker wants to create a macro!!!!!!!!!" << std::endl;
	std::string Name = CgiDataUtilities::getData(cgi, "Name");
	std::string Sequence = CgiDataUtilities::getData(cgi, "Sequence");
	std::string Time = CgiDataUtilities::decodeURIComponent(CgiDataUtilities::getData(cgi, "Time"));
	std::string Notes = CgiDataUtilities::decodeURIComponent(CgiDataUtilities::getData(cgi, "Notes"));
	std::string isMacroPublic = CgiDataUtilities::getData(cgi, "isPublic");
	std::string isMacroLSBF = CgiDataUtilities::getData(cgi, "isLSBF");


	__COUT__<< MACROS_DB_PATH << std::endl;

	std::string fileName = Name + ".dat";
	std::string fullPath;
	if (isMacroPublic == "true")  fullPath = (std::string)MACROS_DB_PATH + "publicMacros/" + fileName;
	else fullPath = (std::string)MACROS_DB_PATH + username + "/" + fileName;
	__COUT__ << fullPath << std::endl;

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
		__COUT__<<  "Unable to open file" << std::endl;
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
						//__COUT__ << line << std::endl;
					}
					returnStr += buffer.str();

					read.close();
				}
				else
					__COUT__<< "Unable to open file" << std::endl;
			}
		}
		std::string returnMacroStr = returnStr.substr(0, returnStr.size()-1);

		__COUT__<<  "Loading existing macros! " << returnMacroStr << std::endl;

		closedir (dir);
		xmldoc.addTextElementToData("returnMacroStr",returnMacroStr);
	}
	else
	{
		__COUT__<<  "Looping through privateMacros folder failed! Wrong directory" << std::endl;
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
						//__COUT__ << line << std::endl;
					}
					returnStr += buffer.str();
					read.close();
				}
				else
					__COUT__<< "Unable to open file" << std::endl;
			}
		}
		std::string returnPublicStr = returnStr.substr(0, returnStr.size()-1);
		__COUT__<<  "Loading existing public macros: " << returnPublicStr << std::endl;
		closedir (dir);
		xmldoc.addTextElementToData("returnPublicStr",returnPublicStr);
	}
	else
	{
		__COUT__ << fullPath << std::endl;
		__COUT__<<  "Looping through MacroData folder failed! Wrong directory" << std::endl;

	}
}

//========================================================================================================================
void MacroMakerSupervisor::appendCommandToHistory(std::string Command,
		std::string Format, std::string Time, std::string Interfaces, const std::string &username)
{
	std::string fileName = "history.hist";
	std::string fullPath = (std::string)MACROS_HIST_PATH + username + "/" + fileName;
	__COUT__ << fullPath << std::endl;
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
		__COUT__<< "Unable to open history.hist" << std::endl;
}

//========================================================================================================================
void MacroMakerSupervisor::loadHistory(HttpXmlDocument& xmldoc, const std::string &username)
{
	std::string fileName = MACROS_HIST_PATH + username + "/" + "history.hist";

	std::ifstream read (fileName.c_str());//reading a file
	__COUT__<<  fileName << std::endl;

	if (read.is_open())
	{
		std::string line;
		char * returnStr;
		unsigned long long fileSz, i = 0, MAX_HISTORY_SIZE = 100000;


		//get length of file to reserve the string size
		//	and to cap history size
		read.seekg(0, std::ios::end);
		fileSz = read.tellg();
		returnStr = new char[fileSz+1];
		returnStr[fileSz] = '\0';
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
			if(i > fileSz) i = fileSz;

			//write back to file truncated history
			FILE *fp = fopen(fileName.c_str(),"w");
			if(!fp)
			{
				__SS__ << "Big problem with macromaker history file: " << fileName << std::endl;
				__SS_THROW__;
			}
			fwrite(&returnStr[i],fileSz-i,1,fp);
			fclose(fp);
		}

		__COUT__<<  "Loading user history! " << std::endl;

		if(fileSz > 1)
			returnStr[fileSz-2] = '\0'; //remove final newline and last #

		xmldoc.addTextElementToData("returnHistStr",&returnStr[i]);


		delete[] returnStr;
	}
	else

		__COUT__<< "Unable to open history.hist" << std::endl;

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

	__COUT__<< fullPath << std::endl;

	std::remove(fullPath.c_str());
	__COUT__ << "Successfully deleted " << MacroName;
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

	__COUT__<<  fullPath << std::endl;

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
		__COUT__<<  "Unable to open file" << std::endl;

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
	__COUT__<< "Successfully deleted " << fullPath;
}

//========================================================================================================================
void MacroMakerSupervisor::exportFEMacro(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi,
		const std::string &username)
{
	std::string macroName = CgiDataUtilities::getData(cgi, "MacroName");
	std::string pluginName = CgiDataUtilities::getData(cgi, "PluginName");
	std::string macroSequence = CgiDataUtilities::postData(cgi, "MacroSequence");
	__COUTV__(pluginName);
	__COUTV__(macroName);
	__COUTV__(macroSequence);


	std::stringstream ss(macroSequence);
	std::string command;
	std::vector<std::string> commands;

	while (getline(ss, command, ','))  commands.push_back(command);

	__COUTV__(StringMacros::vectorToString(commands));

	std::map<std::string /*special type*/,std::set<std::string> /*special file paths*/>
	specialsCodeMap = CodeEditor::getSpecialsMap();

	//__COUTV__(StringMacros::mapToString(specialsCodeMap));
	auto specialsCodeMapIt = specialsCodeMap.find(CodeEditor::SPECIAL_TYPE_FEInterface);
	if(specialsCodeMapIt == specialsCodeMap.end())
	{
		__SS__ << "Could not find any FE Interface plugins in source code. Does MacroMaker " <<
				"have access to the source code? Check that the Supervisor context places MacroMaker in a " <<
				"location with access to the source code." << __E__;
		__SS_THROW__;
	}

	//find first .h and .cc with the plugin name
	std::string headerFile = pluginName + ".h";
	std::string sourceFile = pluginName + "_interface.cc";
	bool foundHeaderFile = false;
	bool foundSourceFile = false;
	for(const auto& filePath : specialsCodeMapIt->second)
	{
		if(!foundHeaderFile &&
				filePath.find(headerFile) != std::string::npos)
		{
			foundHeaderFile = true;
			headerFile = filePath;
			__COUT__ << "found headerFile=" << filePath << __E__;
		}
		if(!foundSourceFile &&
				filePath.find(sourceFile) != std::string::npos)
		{
			foundSourceFile = true;
			sourceFile = filePath;
			__COUT__ << "found sourceFile=" << filePath << __E__;
		}

		if(foundSourceFile && foundHeaderFile) break;
	} //end file search loop

	if(!foundHeaderFile)
	{
		__SS__ << "Could not find the header file for the FE Interface plugins at '" <<
				headerFile << ".' Does MacroMaker " <<
				"have access to the source code? Check that the Supervisor context places MacroMaker in a " <<
				"location with access to the source code." << __E__;
		__SS_THROW__;
	}
	if(!foundSourceFile)
	{
		__SS__ << "Could not find the source file for the FE Interface plugins at '" <<
				sourceFile << ".' Does MacroMaker " <<
				"have access to the source code? Check that the Supervisor context places MacroMaker in a " <<
				"location with access to the source code." << __E__;
		__SS_THROW__;
	}

	//at this point have header and source file, now add FE Macro
	//Steps for each file:
	//	- read current file
	//	- find insert point
	//	- open file for writing
	//		- write original file up to insert point
	//		- insert new code
	//		- write remaining original file

	char timeBuffer[100];
	{ //get time string
		time_t rawtime;
		struct tm * timeinfo;

		time (&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(timeBuffer,100,"%b-%d-%Y %I:%M:%S",timeinfo);
	}

	std::string contents;
	CodeEditor::readFile(headerFile,contents);
	//__COUTV__(contents);

	std::string insert;

	//find end of class by looking for last };
	{
		auto insertPos = contents.rfind("};");
		if(insertPos == std::string::npos)
		{
			__SS__ << "Could not find the code insert position in the header file '" <<
					headerFile << ".' The FE plugin class must end with a '};' - is this the case?" << __E__;
			__SS_THROW__;
		}

		__COUTV__(insertPos);

		insert = "\npublic: // FEMacro '" + macroName + "' generated, " +
				timeBuffer + ", by '" + username + "' using MacroMaker.\n\t" +
				"void " + macroName +
				"\t(frontEndMacroInArgs_t argsIn, frontEndMacroOutArgs_t argsOut);\n";

		__COUTV__(insert);
		CodeEditor::writeFile(headerFile,contents,insertPos,insert);
	}

	xmldoc.addTextElementToData("headerFile",headerFile);

	CodeEditor::readFile(sourceFile,contents);
	//__COUTV__(contents);

	//find start of constructor and register macro
	{
		auto insertPos = contents.find(pluginName + "::" + pluginName);
		if(insertPos == std::string::npos)
		{
			__SS__ << "Could not find the code insert position in the source file '" <<
					sourceFile << ".' The FE plugin class constructor must be '" <<
					pluginName << ":" << pluginName <<"' - is this the case?" << __E__;
			__SS_THROW__;
		}
		__COUTV__(insertPos);
		//find opening bracket after constructor name
		insertPos = contents.find("{",insertPos);
		if(insertPos == std::string::npos)
		{
			__SS__ << "Could not find the code insert position in the source file '" <<
					sourceFile << ".' The FE plugin class constructor must begin with '{" <<
					"' - is this the case?" << __E__;
			__SS_THROW__;
		}
		++insertPos; //go past {
		__COUTV__(insertPos);

		insert = "\n\t//registration of FEMacro '" + macroName + "' generated, " +
				timeBuffer + ", by '" + username + "' using MacroMaker.\n\t" +
				"registerFEMacroFunction(\"" + macroName + "\",//feMacroName \n\t\t" +
				"static_cast<FEVInterface::frontEndMacroFunction_t>(&" +
				pluginName + "::" + macroName + "), //feMacroFunction \n\t\t" +
				"std::vector<std::string>{}, //namesOfInputArgs \n\t\t" +
				"std::vector<std::string>{}, //namesOfOutputArgs \n\t\t" +
				"1); //requiredUserPermissions \n\n";

		__COUTV__(insert);
		contents = contents.substr(0,insertPos) + insert + contents.substr(insertPos);
	}

	//find end of source to append FE Macro function
	{
		auto insertPos = contents.rfind("DEFINE_OTS_INTERFACE");
		if(insertPos == std::string::npos)
		{
			__SS__ << "Could not find the code insert position in the source file '" <<
					sourceFile << ".' The FE plugin class must end with a 'DEFINE_OTS_INTERFACE(" <<
					pluginName << ")' - is this the case?" << __E__;
			__SS_THROW__;
		}
		__COUTV__(insertPos);

		std::stringstream codess;
		createCode(codess,commands,"\t");

		insert = "\n//========================================================================================================================\n//" +
				macroName + "\n" +
				"//\tFEMacro '" + macroName + "' generated, " +
				timeBuffer + ", by '" + username + "' using MacroMaker.\n" +
				"void " + pluginName + "::" + macroName + "(__ARGS__)\n{\n\t" +
				"__CFG_COUT__ << \"# of input args = \" << argsIn.size() << __E__; \n\t" +
				"__CFG_COUT__ << \"# of output args = \" << argsOut.size() << __E__; \n\t" +
				"for(auto &argIn:argsIn) \n\t\t" +
				"__CFG_COUT__ << argIn.first << \": \" << argIn.second << __E__; \n\n\t" +
				"//macro commands section \n" +
				codess.str() +
				"\n\n\t" +
				"for(auto &argOut:argsOut) \n\t\t" +
				"__CFG_COUT__ << argOut.first << \": \" << argOut.second << __E__; \n\n" +
				"} \n\n";

		//__COUTV__(insert);
		CodeEditor::writeFile(sourceFile,contents,insertPos,insert);
	}

	xmldoc.addTextElementToData("sourceFile",sourceFile);

} //end exportFEMacro ()

//========================================================================================================================
void MacroMakerSupervisor::exportMacro(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi, const std::string &username)
{
	std::string macroName = CgiDataUtilities::getData(cgi, "MacroName");
	std::string macroSequence = CgiDataUtilities::postData(cgi, "MacroSequence");
	std::stringstream ss(macroSequence);
	std::string command;
	std::vector<std::string> commands;

	while (getline(ss, command, ','))  commands.push_back(command);

	std::string fileName = macroName + ".cc";


	std::string fullPath = (std::string)MACROS_EXPORT_PATH + username + "/" + fileName;
	__COUT__ << fullPath << std::endl;
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

		createCode(exportFile,commands);

		exportFile.close();

		xmldoc.addTextElementToData("ExportFile",fullPath);
	}
	else
		__COUT__ << "Unable to open file" << std::endl;
}


//========================================================================================================================
//createCode
void MacroMakerSupervisor::createCode(std::ostream& out,
		const std::vector<std::string>& commands,
		const std::string& tabOffset)
{
	int numOfHexBytes;
	std::string hexInitStr;

	out << tabOffset << "{";

	out << "\n" << tabOffset << "\t" << "char *address \t= new char[universalAddressSize_]{0};	//create address buffer of interface size and init to all 0";
	out << "\n" << tabOffset << "\t" << "char *data \t\t= new char[universalDataSize_]{0};		//create data buffer of interface size and init to all 0";

	out << "\n" << tabOffset << "\t" << "uint64_t macroAddress;		//create macro address buffer (size 8 bytes)";
	out << "\n" << tabOffset << "\t" << "uint64_t macroData;			//create macro address buffer (size 8 bytes)";

	//loop through each macro command
	for(unsigned int i = 0; i < commands.size(); i++)
	{
		std::stringstream sst(commands[i]);
		std::string tokens;
		std::vector<std::string> oneCommand;
		while (getline(sst, tokens, ':'))  oneCommand.push_back(tokens);

		__COUT__ << oneCommand[1] << oneCommand[2] << std::endl;

		//make this:
		//			{
		//				uint64_t addrs = 0x1001;	//create address buffer
		//				uint64_t data = 0x100203; 	//create data buffer
		//
		//				universalWrite(addrs,data);
		//			}
		//
		//			//if variable
		//			{
		//				char addrs[universalAddressSize_];
		//				uint64_t macroAddrs = theXDAQContextConfigTree_.getNode(theConfigurationPath_).getNode("variableName").getValue<uint64_t>();
		//				for(unsigned int i=0;i<universalAddressSize_;++i) //fill with macro address and 0 fill
		//					addrs[i] = (i < 8)?((char *)(&macroAddrs))[i]:0;
		//			}

		out << "\n\n" << tabOffset << "\t// ";
		if (oneCommand[1] == "w")
			out << "universalWrite(0x" << oneCommand[2] << ",0x" << oneCommand[3] << ");\n";
		else if (oneCommand[1] == "r")
			out << "universalRead(0x" << oneCommand[2] << ",data);\n";
		else if (oneCommand[1] == "d")
		{
			out << "delay(" << oneCommand[2] << ");\n";
			out << "sleep(" << oneCommand[2] << ");\n";
			continue;
		}
		else
		{
			__COUT_ERR__<< "FATAL ERROR: command is not w, r or d" << std::endl;
			continue;
		}

		//interpret address
		hexInitStr = generateHexArray(oneCommand[2],numOfHexBytes); //interpret address

		//create address
		if(numOfHexBytes == -1) //handle address as variable
		{
			out << tabOffset << "\t" <<
					"macroAddress = theXDAQContextConfigTree_.getNode(theConfigurationPath_).getNode(" <<
					"\n" << tabOffset << "\t\t\"" <<
					oneCommand[2] << "\").getValue<uint64_t>();";
			out << "\t//get macro address from configuration tree";
			out << "\n" << tabOffset << "\tmemcpy(address,&macroAddress,8); //copy macro address to buffer";
		}
		else	//handle address as literal
		{
			out << tabOffset << "\t" <<
					"macroAddress = 0x" << oneCommand[2] << "; memcpy(address,&macroAddress,8);" <<
					"\t//copy macro address to buffer";
		}

		if (oneCommand[1] == "w") //if write, handle data too
		{

			//interpret data
			hexInitStr = generateHexArray(oneCommand[3],numOfHexBytes); //interpret data

			if(numOfHexBytes == -1) //handle data as variable
			{
				out << "\n" << tabOffset << "\t" <<
						"macroData = theXDAQContextConfigTree_.getNode(theConfigurationPath_).getNode(" <<
						"\n" << tabOffset << "\t\t\"" <<
						oneCommand[3] << "\").getValue<uint64_t>();";
				out << "\t//get macro data from configuration tree";
				out << "\n" << tabOffset << "\tmemcpy(data,&macroData,8); //copy macro data to buffer";
			}
			else //handle data as literal
			{
				out << "\n" << tabOffset << "\t" <<
						"macroData = 0x" << oneCommand[3] << "; memcpy(data,&macroData,8);" <<
						"\t//copy macro data to buffer";
			}
			out << "\n" << tabOffset << "\t" <<
					"universalWrite(address,data);";
		}
		else
			out << "\n" << tabOffset << "\t" <<
				"universalRead(address,data);";
	}

	out << "\n\n" << tabOffset << "\tdelete[] address; //free the memory";
	out << "\n" << tabOffset << "\tdelete[] data; //free the memory";
	out << "\n" << tabOffset << "}";
} // end createCode()

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
	__COUT__<< "Translating: \n";
	__COUT__ << srcHexStr << std::endl;

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

	__COUT__ << retSs.str() << std::endl;

	return retSs.str();
}

//========================================================================================================================
void MacroMakerSupervisor::runFEMacro(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi)
{
	__COUT__<< __COUT_HDR_P__ << std::endl;

	//std::string feSupervisorType = CgiDataUtilities::getData(cgi, "feSupervisorType");
	unsigned int feSupervisorID = CgiDataUtilities::getDataAsInt(cgi, "feSupervisorID");
	std::string feUID = CgiDataUtilities::getData(cgi, "feUID");
	std::string macroName = CgiDataUtilities::getData(cgi, "macroName");
	std::string inputArgs = CgiDataUtilities::postData(cgi, "inputArgs");
	std::string outputArgs = CgiDataUtilities::postData(cgi, "outputArgs");

	//__COUT__ << "feSupervisorType = " << feSupervisorType << std::endl;
	__COUT__ << "feSupervisorID = " << feSupervisorID << std::endl;
	__COUT__ << "feUID = " << feUID << std::endl;
	__COUT__ << "macroName = " << macroName << std::endl;
	__COUT__ << "inputArgs = " << inputArgs << std::endl;
	__COUT__ << "outputArgs = " << outputArgs << std::endl;

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



	//find feSupervisorID in target list
	auto supervisorDescriptorPairIt = allFESupervisorInfo_.find(feSupervisorID);
	if(supervisorDescriptorPairIt == allFESupervisorInfo_.end())
	{
		__SS__ << "Targeted Supervisor Descriptor was not found. Attempted target " <<
				"was UID=" << feUID << " at feSupervisorID=" << feSupervisorID << "." << std::endl;
		__COUT_ERR__ << "\n" << ss.str();
		xmldoc.addTextElementToData("Error",ss.str());
		return;
	}

	//have FE supervisor descriptor, so send
	xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
			supervisorDescriptorPairIt->second.getDescriptor(), //supervisor descriptor
			"MacroMakerSupervisorRequest",
			txParameters);
	SOAPMessenger::receive(retMsg, rxParameters);

	__COUT__ << "Received it " << std::endl;

	bool success = rxParameters.getValue("success") == "1";
	outputArgs = rxParameters.getValue("outputArgs");

	__COUT__ << "rx success = " << success << std::endl;
	__COUT__ << "outputArgs = " << outputArgs << std::endl;

	if(!success)
	{
		__SS__ << "Attempted FE Macro Failed. Attempted target " <<
				"was UID=" << feUID << " at feSupervisorID=" << feSupervisorID << "." << std::endl;
		ss << "\n\n The error was:\n\n" << outputArgs << std::endl;
		__COUT_ERR__ << "\n" << ss.str();
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
			__COUT__ << argName << ": " << argValue << std::endl;
		}
	}

}

//========================================================================================================================
void MacroMakerSupervisor::getFEMacroList(HttpXmlDocument& xmldoc, const std::string &username)
{
	__COUT__<< "Getting FE Macro list" << std::endl;

	SOAPParameters txParameters; //params for xoap to send
	txParameters.addParameter("Request", "GetInterfaceMacros");

	SOAPParameters rxParameters;  //params for xoap to recv
	rxParameters.addParameter("FEMacros");

	std::string oneInterface;
	std::string rxFEMacros;

	//get all FE specific macros
	//		for each list of FE Supervisors,
	//			loop through each FE Supervisors and get FE interfaces list
	for(auto &appInfo:allFESupervisorInfo_)
	{
		//		__COUT__ << "===== Number of " << listPair.first << " = " <<
		//				listPair.second.size() << std::endl;
		//
		//		for (it = listPair.second.begin(); it != listPair.second.end(); it++)
		//		{
		__COUT__ << "FESupervisor LID = " << appInfo.second.getId() <<
				" name = " << appInfo.second.getName() << std::endl;

		xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
				appInfo.second.getDescriptor(),
				"MacroMakerSupervisorRequest",
				txParameters);
		SOAPMessenger::receive(retMsg, rxParameters);

		rxFEMacros = rxParameters.getValue("FEMacros");

		__COUT__ << "FE Macros received: \n" << rxFEMacros << std::endl;

		std::istringstream allInterfaces(rxFEMacros);
		while (std::getline(allInterfaces, oneInterface))
			xmldoc.addTextElementToData("FEMacros",appInfo.second.getId() + ":" + oneInterface);
		//		}
	}

	//add macros to response
	loadMacros(xmldoc, username);

	return;
}





