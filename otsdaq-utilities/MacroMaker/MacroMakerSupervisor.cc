#include "otsdaq-utilities/MacroMaker/MacroMakerSupervisor.h"

//#include "otsdaq-core/MessageFacility/MessageFacility.h"
//#include "otsdaq-core/Macros/CoutMacros.h"
//#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
//#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"
//#include "otsdaq-core/SOAPUtilities/SOAPUtilities.h"
//#include "otsdaq-core/SOAPUtilities/SOAPParameters.h"
#include "otsdaq-core/ConfigurationInterface/ConfigurationManager.h"
//#include "otsdaq-core/Macros/CoutMacros.h"

#include "otsdaq-core/FECore/FEVInterface.h"

#include "otsdaq-core/CodeEditor/CodeEditor.h"

//#include <xdaq/NamespaceURI.h>
//#include <string>
//#include <vector>
//#include <iostream>
#include <fstream>
//#include <sstream>
#include <dirent.h>    //for DIR
#include <stdio.h>     //for file rename
#include <sys/stat.h>  //for mkdir
#include <cstdio>
#include <thread>  //for std::thread
#include "../../../otsdaq/otsdaq-core/TableCore/TableGroupKey.h"

#define MACROS_DB_PATH std::string(getenv("SERVICE_DATA_PATH")) + "/MacroData/"
#define MACROS_HIST_PATH std::string(getenv("SERVICE_DATA_PATH")) + "/MacroHistory/"
#define MACROS_EXPORT_PATH std::string(getenv("SERVICE_DATA_PATH")) + "/MacroExport/"

using namespace ots;

#undef __MF_SUBJECT__
#define __MF_SUBJECT__ "MacroMaker"

XDAQ_INSTANTIATOR_IMPL(MacroMakerSupervisor)

//========================================================================================================================
MacroMakerSupervisor::MacroMakerSupervisor(xdaq::ApplicationStub* stub)
    : CoreSupervisorBase(stub)
{
	INIT_MF("MacroMaker");

	// make macro directories in case they don't exist
	mkdir(((std::string)MACROS_DB_PATH).c_str(), 0755);
	mkdir(((std::string)MACROS_HIST_PATH).c_str(), 0755);
	mkdir(((std::string)MACROS_EXPORT_PATH).c_str(), 0755);

	xoap::bind(this,
	           &MacroMakerSupervisor::frontEndCommunicationRequest,
	           "FECommunication",
	           XDAQ_NS_URI);

	init();
}

//========================================================================================================================
MacroMakerSupervisor::~MacroMakerSupervisor(void) { destroy(); }

//========================================================================================================================
void MacroMakerSupervisor::init(void)
{
	// called by constructor

	// MacroMaker should consider all FE compatible types..
	allFESupervisorInfo_ = allSupervisorInfo_.getAllFETypeSupervisorInfo();
}

//========================================================================================================================
void MacroMakerSupervisor::destroy(void)
{
	// called by destructor
}

//========================================================================================================================
// forceSupervisorPropertyValues
//		override to force supervisor property values (and ignore user settings)
void MacroMakerSupervisor::forceSupervisorPropertyValues()
{
	//	CorePropertySupervisorBase::setSupervisorProperty(CorePropertySupervisorBase::SUPERVISOR_PROPERTIES.NeedUsernameRequestTypes,
	//			"getPermission");
}

//========================================================================================================================
void MacroMakerSupervisor::request(const std::string&               requestType,
                                   cgicc::Cgicc&                    cgiIn,
                                   HttpXmlDocument&                 xmlOut,
                                   const WebUsers::RequestUserInfo& userInfo) try
{
	__SUP_COUT__ << "User name is " << userInfo.username_ << "." << __E__;
	__SUP_COUT__ << "User permission level for request '" << requestType << "' is "
	             << unsigned(userInfo.permissionLevel_) << "." << __E__;

	// handle request per requestType
	if(requestType == "getPermission")
	{
		xmlOut.addTextElementToData("Permission",
		                            std::to_string(unsigned(userInfo.permissionLevel_)));

		// create macro maker folders for the user (the first time a user authenticates
		// with macro maker)
		std::string macroPath = (std::string)MACROS_DB_PATH + userInfo.username_ + "/";
		mkdir(macroPath.c_str(), 0755);
		std::string histPath = (std::string)MACROS_HIST_PATH + userInfo.username_ + "/";
		mkdir(histPath.c_str(), 0755);
		std::string publicPath = (std::string)MACROS_DB_PATH + "publicMacros/";
		mkdir(publicPath.c_str(), 0755);
		std::string exportPath =
		    (std::string)MACROS_EXPORT_PATH + userInfo.username_ + "/";
		mkdir(exportPath.c_str(), 0755);
	}
	else
		handleRequest(requestType, xmlOut, cgiIn, userInfo.username_);
}
catch(const std::runtime_error& e)
{
	__SS__ << "Error occurred handling request '" << requestType << "': " << e.what()
	       << __E__;
	__SUP_COUT__ << ss.str();
	xmlOut.addTextElementToData("Error", ss.str());
}
catch(...)
{
	__SS__ << "Unknown error occurred handling request '" << requestType << "!'" << __E__;
	__SUP_COUT__ << ss.str();
	xmlOut.addTextElementToData("Error", ss.str());
}

//========================================================================================================================
void MacroMakerSupervisor::handleRequest(const std::string  Command,
                                         HttpXmlDocument&   xmldoc,
                                         cgicc::Cgicc&      cgi,
                                         const std::string& username)
{
	if(Command == "FElist")
		getFElist(xmldoc);
	else if(Command == "writeData")
		writeData(xmldoc, cgi, username);
	else if(Command == "readData")
		readData(xmldoc, cgi, username);
	else if(Command == "createMacro")
		createMacro(xmldoc, cgi, username);
	else if(Command == "loadMacros")
		loadMacros(xmldoc, username);
	else if(Command == "loadHistory")
		loadHistory(xmldoc, username);
	else if(Command == "deleteMacro")
		deleteMacro(xmldoc, cgi, username);
	else if(Command == "editMacro")
		editMacro(xmldoc, cgi, username);
	else if(Command == "clearHistory")
		clearHistory(username);
	else if(Command == "exportMacro")
		exportMacro(xmldoc, cgi, username);
	else if(Command == "exportFEMacro")
		exportFEMacro(xmldoc, cgi, username);
	else if(Command == "getFEMacroList")
		getFEMacroList(xmldoc, username);
	else if(Command == "runFEMacro")
		runFEMacro(xmldoc, cgi);
	else
		xmldoc.addTextElementToData("Error", "Unrecognized command '" + Command + "'");
}

//========================================================================================================================
xoap::MessageReference MacroMakerSupervisor::frontEndCommunicationRequest(
    xoap::MessageReference message) try
{
	__SUP_COUT__ << "FE Request received: " << SOAPUtilities::translate(message) << __E__;

	SOAPParameters typeParameter, rxParameters;  // params for xoap to recv
	typeParameter.addParameter("type");
	SOAPUtilities::receive(message, typeParameter);

	std::string type = typeParameter.getValue("type");

	std::string error = "";

	if(type == "initFElist")  // gateway initializes during configure
	{
		__SUP_COUTV__(type);

		rxParameters.addParameter("groupName");
		rxParameters.addParameter("groupKey");
		SOAPUtilities::receive(message, rxParameters);

		std::string groupName = rxParameters.getValue("groupName");
		std::string groupKey  = rxParameters.getValue("groupKey");

		__SUP_COUTV__(groupName);
		__SUP_COUTV__(groupKey);

		ConfigurationManager cfgMgr;
		cfgMgr.loadConfigurationGroup(groupName, TableGroupKey(groupKey), true);

		// for each FESupervisor
		// get all front end children

		const SupervisorInfoMap& feTypeSupervisors =
		    CorePropertySupervisorBase::allSupervisorInfo_.getAllFETypeSupervisorInfo();

		ConfigurationTree appsNode = cfgMgr.getNode("XDAQApplicationConfiguration");

		for(auto& feApp : feTypeSupervisors)
		{
			__SUP_COUT__ << "FEs for app " << feApp.first << ":" << feApp.second.getName()
			             << __E__;

			std::vector<std::string> feChildren = appsNode.getNode(feApp.second.getName())
			                                          .getNode("LinkToSupervisorTable")
			                                          .getNode("LinkToFEInterfaceTable")
			                                          .getChildrenNames();

			for(auto& fe : feChildren)
			{
				__COUTV__(fe);
				FEtoSupervisorMap_[fe] = feApp.first;
			}
		}

		__SUP_COUTV__(StringMacros::mapToString(FEtoSupervisorMap_));
	}
	else if(type == "feSend" ||                        // from front-ends
	        type == "feMacro" ||                       // from front-ends
	        type == "feMacroMultiDimensionalStart" ||  // from iterator
	        type == "feMacroMultiDimensionalCheck" ||  // from iterator
	        type == "macroMultiDimensionalStart" ||    // from iterator
	        type == "macroMultiDimensionalCheck")      // from iterator
	{
		__SUP_COUTV__(type);

		rxParameters.addParameter("targetInterfaceID");
		SOAPUtilities::receive(message, rxParameters);

		std::string targetInterfaceID = rxParameters.getValue("targetInterfaceID");

		__SUP_COUTV__(targetInterfaceID);

		auto feIt = FEtoSupervisorMap_.find(targetInterfaceID);
		if(feIt == FEtoSupervisorMap_.end())
		{
			__SUP_SS__ << "Destination front end interface ID '" << targetInterfaceID
			           << "' was not found in the list of front ends." << __E__;
			__SUP_SS_THROW__;
		}

		unsigned int FESupervisorIndex = feIt->second;
		__SUP_COUT__ << "Found supervisor index: " << FESupervisorIndex << __E__;

		SupervisorInfoMap::iterator it = allFESupervisorInfo_.find(FESupervisorIndex);
		if(it == allFESupervisorInfo_.end())
		{
			__SUP_SS__ << "Error transmitting request to FE Supervisor '"
			           << targetInterfaceID << ":" << FESupervisorIndex << ".' \n\n"
			           << "The FE Supervisor Index does not exist. Have you configured "
			              "the state machine properly?"
			           << __E__;
			__SUP_SS_THROW__;
		}

		if(type == "macroMultiDimensionalStart")
		{
			// add Macro sequence (and check macro exists)

			SOAPParameters rxParameters;
			rxParameters.addParameter("macroName");
			SOAPUtilities::receive(message, rxParameters);
			std::string macroName = rxParameters.getValue("macroName");
			__SUP_COUTV__(macroName);

			// MacroMakerSupervisor::macroStruct_t macro;
			std::string macroString;
			loadMacro(macroName, macroString);

			SOAPParameters parameters;
			parameters.addParameter("macroString", macroString);
			SOAPUtilities::addParameters(message, parameters);
		}

		try
		{
			__SUP_COUT__ << "Forwarding request: " << SOAPUtilities::translate(message)
			             << __E__;

			xoap::MessageReference replyMessage =
			    SOAPMessenger::sendWithSOAPReply(it->second.getDescriptor(), message);

			if(type != "feSend")
			{
				__SUP_COUT__ << "Forwarding FE Macro response: "
				             << SOAPUtilities::translate(replyMessage) << __E__;

				return replyMessage;
			}
		}
		catch(const xdaq::exception::Exception& e)
		{
			__SUP_SS__ << "Error forwarding FE Communication request to FE Supervisor '"
			           << targetInterfaceID << ":" << FESupervisorIndex << ".' "
			           << "Have you configured the state machine properly?\n\n"
			           << e.what() << __E__;
			__SUP_SS_THROW__;
		}
	}
	else
	{
		__SUP_SS__ << "Unrecognized FE Communication type: " << type << __E__;
		__SUP_SS_THROW__;
	}

	return SOAPUtilities::makeSOAPMessageReference("Received");
}  // end frontEndCommunicationRequest()
catch(const std::runtime_error& e)
{
	xoap::MessageReference returnMessage =
	    SOAPUtilities::makeSOAPMessageReference("Error");

	SOAPParameters parameters;
	parameters.addParameter("Error", e.what());
	SOAPUtilities::addParameters(returnMessage, parameters);
	return returnMessage;
}
catch(...)
{
	xoap::MessageReference returnMessage =
	    SOAPUtilities::makeSOAPMessageReference("Error");

	__SUP_SS__ << "Unknown error processing FE communication request." << __E__;
	__SUP_COUT_ERR__ << ss.str();

	SOAPParameters parameters;
	parameters.addParameter("Error", ss.str());
	SOAPUtilities::addParameters(returnMessage, parameters);
	return returnMessage;
}  // end frontEndCommunicationRequest() catch

//========================================================================================================================
void MacroMakerSupervisor::getFElist(HttpXmlDocument& xmldoc)
{
	__SUP_COUT__ << "Getting FE list!!!!!!!!!" << __E__;
	FEtoSupervisorMap_.clear();

	SOAPParameters txParameters;  // params for xoap to send
	txParameters.addParameter("Request", "GetInterfaces");

	SOAPParameters rxParameters;  // params for xoap to recv
	rxParameters.addParameter("FEList");

	SupervisorInfoMap::const_iterator it;
	std::string                       oneInterface;
	std::string                       rxFEList;

	size_t lastColonIndex;

	// for each list of FE Supervisors,
	//	loop through each FE Supervisors and get FE interfaces list
	for(auto& appInfo : allFESupervisorInfo_)
	{
		//		__SUP_COUT__ << "Number of " << listPair.first << " = " <<
		//				listPair.second.size() << __E__;
		//
		//		for (it = listPair.second.begin(); it != listPair.second.end(); it++)
		//		{

		__SUP_COUT__ << "FESupervisor LID = " << appInfo.second.getId()
		             << " name = " << appInfo.second.getName() << __E__;

		try
		{
			xoap::MessageReference retMsg =
			    SOAPMessenger::sendWithSOAPReply(appInfo.second.getDescriptor(),
			                                     "MacroMakerSupervisorRequest",
			                                     txParameters);
			SOAPUtilities::receive(retMsg, rxParameters);
		}
		catch(const xdaq::exception::Exception& e)
		{
			__SS__ << "Error transmitting request to FE Supervisor LID = "
			       << appInfo.second.getId() << " name = " << appInfo.second.getName()
			       << ". \n\n"
			       << e.what() << __E__;
			__SUP_COUT_ERR__ << ss.str();
			return;
		}

		rxFEList = rxParameters.getValue("FEList");

		__SUP_COUT__ << "FE List received: \n" << rxFEList << __E__;

		std::istringstream allInterfaces(rxFEList);
		while(std::getline(allInterfaces, oneInterface))
		{
			__SUP_COUTV__(oneInterface);
			xmldoc.addTextElementToData("FE", oneInterface);

			lastColonIndex = oneInterface.rfind(':');
			if(lastColonIndex == std::string::npos)
			{
				__SUP_SS__ << "Last colon could not be found in " << oneInterface
				           << __E__;
				__SS_THROW__;
			}
			oneInterface = oneInterface.substr(lastColonIndex);

			__SUP_COUTV__(oneInterface);

			FEtoSupervisorMap_[oneInterface] = appInfo.second.getId();
		}  // end FE extract loop

	}  // end ask Supervisors for their FE list loop

}  // end getFEList()

//========================================================================================================================
void MacroMakerSupervisor::writeData(HttpXmlDocument&   xmldoc,
                                     cgicc::Cgicc&      cgi,
                                     const std::string& username)
{
	__SUP_COUT__ << "MacroMaker writing..." << __E__;

	std::string Address              = CgiDataUtilities::getData(cgi, "Address");
	std::string Data                 = CgiDataUtilities::getData(cgi, "Data");
	std::string interfaceIndexArray  = CgiDataUtilities::getData(cgi, "interfaceIndex");
	std::string supervisorIndexArray = CgiDataUtilities::getData(cgi, "supervisorIndex");
	std::string time =
	    CgiDataUtilities::decodeURIComponent(CgiDataUtilities::getData(cgi, "time"));
	std::string addressFormatStr = CgiDataUtilities::getData(cgi, "addressFormatStr");
	std::string dataFormatStr    = CgiDataUtilities::getData(cgi, "dataFormatStr");

	std::string interfaces = CgiDataUtilities::postData(cgi, "interfaces");

	__SUP_COUT__ << "Write Address: " << Address << " Data: " << Data << __E__;
	__SUP_COUTV__(interfaces);

	std::string command = "w:" + Address + ":" + Data;
	std::string format  = addressFormatStr + ":" + dataFormatStr;
	appendCommandToHistory(command, format, time, interfaces, username);

	SOAPParameters txParameters;  // params for xoap to send
	txParameters.addParameter("Request", "UniversalWrite");
	txParameters.addParameter("Address", Address);
	txParameters.addParameter("Data", Data);

	__SUP_COUT__ << "Here comes the array from multiselect box for WRITE, behold: \n"
	             << supervisorIndexArray << "\n"
	             << interfaceIndexArray << __E__;

	////////////////////////////////Store cgi arrays into
	/// vectors/////////////////////////////
	std::vector<std::string> interfaceIndices;
	std::istringstream       f(interfaceIndexArray);
	std::string              s;
	while(getline(f, s, ','))
		interfaceIndices.push_back(s);
	std::vector<int>   supervisorIndices;
	std::istringstream g(supervisorIndexArray);
	std::string        t;
	while(getline(g, t, ','))
		supervisorIndices.push_back(std::stoi(t));

	for(unsigned int i = 0; i < supervisorIndices.size(); i++)
	{
		unsigned int FESupervisorIndex = supervisorIndices[i];
		std::string  interfaceIndex    = interfaceIndices[i];

		txParameters.addParameter("InterfaceID", interfaceIndex);

		__SUP_COUT__ << "The index of the supervisor instance is: " << FESupervisorIndex
		             << __E__;
		__SUP_COUT__ << "...and the interface ID is: " << interfaceIndex << __E__;

		SupervisorInfoMap::iterator it = allFESupervisorInfo_.find(FESupervisorIndex);
		if(it == allFESupervisorInfo_.end())
		{
			__SUP_SS__ << "Error transmitting request to FE Supervisor '"
			           << interfaceIndex << ":" << FESupervisorIndex << ".' \n\n"
			           << "The FE Index doesn't exist. Have you configured the state "
			              "machine properly?"
			           << __E__;
			__SUP_SS_THROW__;
		}

		try
		{
			xoap::MessageReference replyMessage = SOAPMessenger::sendWithSOAPReply(
			    it->second.getDescriptor(), "MacroMakerSupervisorRequest", txParameters);

			__SUP_COUT__ << "Response received: "
			             << SOAPUtilities::translate(replyMessage) << __E__;

			SOAPParameters rxParameters;
			rxParameters.addParameter("Error");
			SOAPUtilities::receive(replyMessage, rxParameters);

			std::string error = rxParameters.getValue("Error");
			__SUP_COUTV__(error);

			if(error != "")
			{
				// error occurred!
				__SUP_SS__ << "Error transmitting request to FE Supervisor '"
				           << interfaceIndex << ":" << FESupervisorIndex << ".' "
				           << "Have you configured the state machine properly?\n\n"
				           << error << __E__;
				__SUP_SS_THROW__;
			}
		}
		catch(const xdaq::exception::Exception& e)
		{
			__SUP_SS__ << "Error transmitting request to FE Supervisor '"
			           << interfaceIndex << ":" << FESupervisorIndex << ".' "
			           << "Have you configured the state machine properly?\n\n"
			           << e.what() << __E__;
			__SUP_SS_THROW__;
		}

	}  // end FE Supervisor loop
}  // end writeData()

//========================================================================================================================
void MacroMakerSupervisor::readData(HttpXmlDocument&   xmldoc,
                                    cgicc::Cgicc&      cgi,
                                    const std::string& username)
{
	__SUP_COUT__ << "@@@@@@@ MacroMaker wants to read data @@@@@@@@" << __E__;
	std::string Address              = CgiDataUtilities::getData(cgi, "Address");
	std::string interfaceIndexArray  = CgiDataUtilities::getData(cgi, "interfaceIndex");
	std::string supervisorIndexArray = CgiDataUtilities::getData(cgi, "supervisorIndex");
	std::string time =
	    CgiDataUtilities::decodeURIComponent(CgiDataUtilities::getData(cgi, "time"));
	std::string addressFormatStr = CgiDataUtilities::getData(cgi, "addressFormatStr");
	std::string dataFormatStr    = CgiDataUtilities::getData(cgi, "dataFormatStr");

	std::string interfaces = CgiDataUtilities::postData(cgi, "interfaces");

	__SUP_COUT__ << "Read Address: " << Address << __E__;
	__SUP_COUTV__(interfaces);

	SOAPParameters txParameters;  // params for xoap to send
	txParameters.addParameter("Request", "UniversalRead");
	txParameters.addParameter("Address", Address);

	SOAPParameters rxParameters;
	rxParameters.addParameter("dataResult");
	rxParameters.addParameter("Error");
	__SUP_COUT__ << "Here comes the array from multiselect box for READ, behold: "
	             << supervisorIndexArray << "," << interfaceIndexArray << __E__;

	////////////////////////////////Store cgi arrays into
	/// vectors/////////////////////////////
	std::vector<std::string> interfaceIndices;
	std::istringstream       f(interfaceIndexArray);
	std::string              s;
	while(getline(f, s, ','))
		interfaceIndices.push_back(s);
	std::vector<int>   supervisorIndices;
	std::istringstream g(supervisorIndexArray);
	std::string        t;
	while(getline(g, t, ','))
		supervisorIndices.push_back(std::stoi(t));

	for(unsigned int i = 0; i < supervisorIndices.size(); i++)
	{
		unsigned int FESupervisorIndex = supervisorIndices[i];
		std::string  interfaceIndex    = interfaceIndices[i];

		txParameters.addParameter("InterfaceID", interfaceIndex);

		__SUP_COUT__ << "The index of the supervisor instance is: " << FESupervisorIndex
		             << __E__;
		__SUP_COUT__ << "...and the interface ID is: " << interfaceIndex << __E__;

		SupervisorInfoMap::iterator it = allFESupervisorInfo_.find(FESupervisorIndex);
		if(it == allFESupervisorInfo_.end())
		{
			__SUP_SS__ << "Error transmitting request to FE Supervisor '"
			           << interfaceIndex << ":" << FESupervisorIndex << ".' \n\n"
			           << "The FE Index doesn't exist. Have you configured the state "
			              "machine properly?"
			           << __E__;
			__SUP_SS_THROW__;
		}

		try
		{
			xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
			    it->second.getDescriptor(), "MacroMakerSupervisorRequest", txParameters);

			__SUP_COUT__ << "Response received: " << SOAPUtilities::translate(retMsg)
			             << __E__;

			// SOAPParameters rxParameters;
			// rxParameters.addParameter("Error");
			SOAPUtilities::receive(retMsg, rxParameters);

			std::string error = rxParameters.getValue("Error");
			__SUP_COUTV__(error);

			if(error != "")
			{
				// error occurred!
				__SUP_SS__ << "Error transmitting request to FE Supervisor '"
				           << interfaceIndex << ":" << FESupervisorIndex << ".' "
				           << "Have you configured the state machine properly?\n\n"
				           << error << __E__;
				__SUP_SS_THROW__;
			}
		}
		catch(const xdaq::exception::Exception& e)
		{
			__SUP_SS__ << "Error transmitting request to FE Supervisor '"
			           << interfaceIndex << ":" << FESupervisorIndex << ".' "
			           << "Have you configured the state machine properly?\n\n"
			           << e.what() << __E__;
			__SUP_SS_THROW__;
		}

		std::string dataReadResult = rxParameters.getValue("dataResult");
		__SUP_COUT__ << "Data reading result received: " << dataReadResult << __E__;
		xmldoc.addTextElementToData("readData", dataReadResult);
		std::string command = "r:" + Address + ":" + dataReadResult;
		std::string format  = addressFormatStr + ":" + dataFormatStr;
		appendCommandToHistory(command, format, time, interfaces, username);
	}
}

//========================================================================================================================
void MacroMakerSupervisor::createMacro(HttpXmlDocument&   xmldoc,
                                       cgicc::Cgicc&      cgi,
                                       const std::string& username)
{
	__SUP_COUT__ << "MacroMaker wants to create a macro!!!!!!!!!" << __E__;
	std::string Name     = CgiDataUtilities::postData(cgi, "Name");
	std::string Sequence = CgiDataUtilities::postData(cgi, "Sequence");
	std::string Time     = CgiDataUtilities::postData(cgi, "Time");
	std::string Notes =
	    CgiDataUtilities::decodeURIComponent(CgiDataUtilities::postData(cgi, "Notes"));
	std::string isMacroPublic = CgiDataUtilities::getData(cgi, "isPublic");
	std::string isMacroLSBF   = CgiDataUtilities::getData(cgi, "isLSBF");

	__SUP_COUTV__(Name);
	__SUP_COUTV__(Sequence);
	__SUP_COUTV__(Notes);
	__SUP_COUTV__(Time);
	__SUP_COUTV__(isMacroPublic);
	__SUP_COUTV__(isMacroLSBF);

	__SUP_COUTV__(MACROS_DB_PATH);

	std::string fileName = Name + ".dat";
	std::string fullPath;
	if(isMacroPublic == "true")
		fullPath = (std::string)MACROS_DB_PATH + "publicMacros/" + fileName;
	else
		fullPath = (std::string)MACROS_DB_PATH + username + "/" + fileName;

	__SUP_COUTV__(fullPath);

	std::ofstream macrofile(fullPath.c_str());
	if(macrofile.is_open())
	{
		macrofile << "{\n";
		macrofile << "\"name\":\"" << Name << "\",\n";
		macrofile << "\"sequence\":\"" << Sequence << "\",\n";
		macrofile << "\"time\":\"" << Time << "\",\n";
		macrofile << "\"notes\":\"" << Notes << "\",\n";
		macrofile << "\"LSBF\":\"" << isMacroLSBF << "\"\n";
		macrofile << "}@" << __E__;
		macrofile.close();
	}
	else
		__SUP_COUT__ << "Unable to open file" << __E__;
}  // end createMacro()

//========================================================================================================================
// loadMacro
//	Load macro string from file.
//	look in public macros and username (if given)
//	for the macroName.
//
//	If found, return by reference
//	Else, throw exception
void MacroMakerSupervisor::loadMacro(const std::string& macroName,
                                     std::string&       macroString,
                                     const std::string& username /*=""*/)
{
	__SUP_COUTV__(macroName);

	// first check public folder, then user
	std::string fullPath, line;
	macroString = "";
	for(unsigned int i = 0; i < 2; ++i)
	{
		if(i == 1)
			fullPath = (std::string)MACROS_DB_PATH + username + "/";
		else
			fullPath = (std::string)MACROS_DB_PATH + "publicMacros/";

		fullPath += macroName + ".dat";
		__SUP_COUTV__(fullPath);

		std::ifstream read(fullPath.c_str());  // reading a file
		if(read.is_open())
		{
			while(!read.eof())
			{
				getline(read, line);
				macroString += line;
			}

			read.close();
		}
		else  // file does not exist
		{
			__SUP_COUT__ << "Unable to open file: " << fullPath << __E__;
			continue;
		}

		if(macroString != "")
			break;  // macro has been found!
	}               // end load from path loop

	if(macroString == "")
	{
		__SUP_SS__ << "Unable to locate file for macro '" << macroName
		           << "'... does it exist?" << __E__;
		if(username != "")
			ss << " Attempted username was '" << username << ".'" << __E__;
		__SUP_SS_THROW__;
	}

	__SUP_COUTV__(macroString);
}  // end loadMacro()

//========================================================================================================================
void MacroMakerSupervisor::loadMacros(HttpXmlDocument&   xmldoc,
                                      const std::string& username)
{
	DIR*           dir;
	struct dirent* ent;
	std::string    returnStr = "";
	std::string    fullPath  = (std::string)MACROS_DB_PATH + username + "/";
	if((dir = opendir(fullPath.c_str())) != NULL)
	{
		/* print all the files and directories within directory */
		while((ent = readdir(dir)) != NULL)
		{
			/* File name validation check */
			if((unsigned)strlen(ent->d_name) > 4)
			{
				std::string   line;
				std::ifstream read(
				    ((fullPath + (std::string)ent->d_name)).c_str());  // reading a file
				if(read.is_open())
				{
					std::stringstream buffer;
					while(!read.eof())
					{
						getline(read, line);
						buffer << line;
						//__SUP_COUT__ << line << __E__;
					}
					returnStr += buffer.str();

					read.close();
				}
				else
					__SUP_COUT__ << "Unable to open file" << __E__;
			}
		}
		std::string returnMacroStr = returnStr.substr(0, returnStr.size() - 1);

		__SUP_COUT__ << "Loading existing macros! " << returnMacroStr << __E__;

		closedir(dir);
		xmldoc.addTextElementToData("returnMacroStr", returnMacroStr);
	}
	else
	{
		__SUP_COUT__ << "Looping through privateMacros folder failed! Wrong directory"
		             << __E__;
	}
	fullPath  = (std::string)MACROS_DB_PATH + "publicMacros/";
	returnStr = "";
	if((dir = opendir(fullPath.c_str())) != NULL)
	{
		/* print all the files and directories within directory */
		while((ent = readdir(dir)) != NULL)
		{
			/* File name validation check */
			if((unsigned)strlen(ent->d_name) > 4)
			{
				std::string   line;
				std::ifstream read(
				    ((fullPath + (std::string)ent->d_name)).c_str());  // reading a file
				if(read.is_open())
				{
					std::stringstream buffer;
					while(!read.eof())
					{
						getline(read, line);
						buffer << line;
						//__SUP_COUT__ << line << __E__;
					}
					returnStr += buffer.str();
					read.close();
				}
				else
					__SUP_COUT__ << "Unable to open file" << __E__;
			}
		}
		std::string returnPublicStr = returnStr.substr(0, returnStr.size() - 1);
		__SUP_COUT__ << "Loading existing public macros: " << returnPublicStr << __E__;
		closedir(dir);
		xmldoc.addTextElementToData("returnPublicStr", returnPublicStr);
	}
	else
	{
		__SUP_COUT__ << fullPath << __E__;
		__SUP_COUT__ << "Looping through MacroData folder failed! Wrong directory"
		             << __E__;
	}
}  // end loadMacros()

//========================================================================================================================
void MacroMakerSupervisor::appendCommandToHistory(std::string        Command,
                                                  std::string        Format,
                                                  std::string        Time,
                                                  std::string        Interfaces,
                                                  const std::string& username)
{
	std::string fileName = "history.hist";
	std::string fullPath = (std::string)MACROS_HIST_PATH + username + "/" + fileName;
	__SUP_COUT__ << fullPath << __E__;
	std::ofstream histfile(fullPath.c_str(), std::ios::app);
	if(histfile.is_open())
	{
		histfile << "{\n";
		histfile << "\"Command\":\"" << Command << "\",\n";
		histfile << "\"Format\":\"" << Format << "\",\n";
		histfile << "\"Time\":\"" << Time << "\",\n";
		histfile << "\"Interfaces\":\"" << Interfaces << "\"\n";
		histfile << "}#" << __E__;
		histfile.close();
	}
	else
		__SUP_COUT__ << "Unable to open history.hist" << __E__;
}

//========================================================================================================================
void MacroMakerSupervisor::loadHistory(HttpXmlDocument&   xmldoc,
                                       const std::string& username)
{
	std::string fileName = MACROS_HIST_PATH + username + "/" + "history.hist";

	std::ifstream read(fileName.c_str());  // reading a file
	__SUP_COUT__ << fileName << __E__;

	if(read.is_open())
	{
		std::string        line;
		char*              returnStr;
		unsigned long long fileSz, i = 0, MAX_HISTORY_SIZE = 100000;

		// get length of file to reserve the string size
		//	and to cap history size
		read.seekg(0, std::ios::end);
		fileSz            = read.tellg();
		returnStr         = new char[fileSz + 1];
		returnStr[fileSz] = '\0';
		read.seekg(0, std::ios::beg);

		// read data as a block:
		read.read(returnStr, fileSz);
		read.close();

		// find i such that new string size is less than
		if(fileSz > MAX_HISTORY_SIZE)
		{
			i = fileSz - MAX_HISTORY_SIZE;
			for(; i < fileSz; ++i)
				if(returnStr[i] == '#')
				{
					i += 2;
					break;  // skip new line character also to get to next record
				}
			if(i > fileSz)
				i = fileSz;

			// write back to file truncated history
			FILE* fp = fopen(fileName.c_str(), "w");
			if(!fp)
			{
				__SS__ << "Big problem with macromaker history file: " << fileName
				       << __E__;
				__SS_THROW__;
			}
			fwrite(&returnStr[i], fileSz - i, 1, fp);
			fclose(fp);
		}

		__SUP_COUT__ << "Loading user history! " << __E__;

		if(fileSz > 1)
			returnStr[fileSz - 2] = '\0';  // remove final newline and last #

		xmldoc.addTextElementToData("returnHistStr", &returnStr[i]);

		delete[] returnStr;
	}
	else

		__SUP_COUT__ << "Unable to open history.hist" << __E__;
}

//========================================================================================================================
void MacroMakerSupervisor::deleteMacro(HttpXmlDocument&   xmldoc,
                                       cgicc::Cgicc&      cgi,
                                       const std::string& username)
{
	std::string MacroName     = CgiDataUtilities::getData(cgi, "MacroName");
	std::string isMacroPublic = CgiDataUtilities::getData(cgi, "isPublic");

	std::string fileName = MacroName + ".dat";
	std::string fullPath;
	if(isMacroPublic == "true")
		fullPath = (std::string)MACROS_DB_PATH + "publicMacros/" + fileName;
	else
		fullPath = (std::string)MACROS_DB_PATH + username + "/" + fileName;

	__SUP_COUT__ << fullPath << __E__;

	std::remove(fullPath.c_str());
	__SUP_COUT__ << "Successfully deleted " << MacroName;
	xmldoc.addTextElementToData("deletedMacroName", MacroName);
}

//========================================================================================================================
void MacroMakerSupervisor::editMacro(HttpXmlDocument&   xmldoc,
                                     cgicc::Cgicc&      cgi,
                                     const std::string& username)
{
	std::string oldMacroName = CgiDataUtilities::postData(cgi, "oldMacroName");
	std::string newMacroName = CgiDataUtilities::postData(cgi, "newMacroName");
	std::string Sequence     = CgiDataUtilities::postData(cgi, "Sequence");
	std::string Time         = CgiDataUtilities::postData(cgi, "Time");
	std::string Notes =
	    CgiDataUtilities::decodeURIComponent(CgiDataUtilities::postData(cgi, "Notes"));

	std::string isMacroPublic = CgiDataUtilities::getData(cgi, "isPublic");
	std::string isMacroLSBF   = CgiDataUtilities::getData(cgi, "isLSBF");

	__SUP_COUTV__(oldMacroName);
	__SUP_COUTV__(newMacroName);
	__SUP_COUTV__(Sequence);
	__SUP_COUTV__(Notes);
	__SUP_COUTV__(Time);
	__SUP_COUTV__(isMacroPublic);
	__SUP_COUTV__(isMacroLSBF);

	__SUP_COUTV__(MACROS_DB_PATH);

	std::string fileName = oldMacroName + ".dat";
	std::string fullPath;
	if(isMacroPublic == "true")
		fullPath = (std::string)MACROS_DB_PATH + "publicMacros/" + fileName;
	else
		fullPath = (std::string)MACROS_DB_PATH + username + "/" + fileName;

	__SUP_COUTV__(fullPath);

	std::ofstream macrofile(fullPath.c_str());
	if(macrofile.is_open())
	{
		macrofile << "{\n";
		macrofile << "\"name\":\"" << newMacroName << "\",\n";
		macrofile << "\"sequence\":\"" << Sequence << "\",\n";
		macrofile << "\"time\":\"" << Time << "\",\n";
		macrofile << "\"notes\":\"" << Notes << "\",\n";
		macrofile << "\"LSBF\":\"" << isMacroLSBF << "\"\n";
		macrofile << "}@" << __E__;
		macrofile.close();
	}
	else
		__SUP_COUT__ << "Unable to open file" << __E__;

	if(oldMacroName != newMacroName)  // renaming macro
	{
		int result;
		result =
		    rename((MACROS_DB_PATH + username + "/" + oldMacroName + ".dat").c_str(),
		           (MACROS_DB_PATH + username + "/" + newMacroName + ".dat").c_str());
		if(result == 0)
			xmldoc.addTextElementToData("newMacroName", newMacroName);
		else
			xmldoc.addTextElementToData("newMacroName", "ERROR");
	}
}

//========================================================================================================================
void MacroMakerSupervisor::clearHistory(const std::string& username)
{
	std::string fileName = "history.hist";
	std::string fullPath = (std::string)MACROS_HIST_PATH + username + "/" + fileName;

	std::remove(fullPath.c_str());
	__SUP_COUT__ << "Successfully deleted " << fullPath;
}

//========================================================================================================================
void MacroMakerSupervisor::exportFEMacro(HttpXmlDocument&   xmldoc,
                                         cgicc::Cgicc&      cgi,
                                         const std::string& username)
{
	std::string macroName     = CgiDataUtilities::getData(cgi, "MacroName");
	std::string pluginName    = CgiDataUtilities::getData(cgi, "PluginName");
	std::string macroSequence = CgiDataUtilities::postData(cgi, "MacroSequence");
	std::string macroNotes    = CgiDataUtilities::decodeURIComponent(
        CgiDataUtilities::postData(cgi, "MacroNotes"));

	__SUP_COUTV__(pluginName);
	__SUP_COUTV__(macroName);
	__SUP_COUTV__(macroSequence);

	// replace all special characters with white space
	for(unsigned int i = 0; i < macroNotes.length(); ++i)
		if(macroNotes[i] == '\r' || macroNotes[i] == '\n')
			macroNotes[i] = ' ';
	__SUP_COUTV__(macroNotes);

	std::stringstream        ss(macroSequence);
	std::string              command;
	std::vector<std::string> commands;

	while(getline(ss, command, ','))
		commands.push_back(command);

	__SUP_COUTV__(StringMacros::vectorToString(commands));

	std::map<std::string /*special type*/, std::set<std::string> /*special file paths*/>
	    specialsCodeMap = CodeEditor::getSpecialsMap();

	//__SUP_COUTV__(StringMacros::mapToString(specialsCodeMap));
	auto specialsCodeMapIt = specialsCodeMap.find(CodeEditor::SPECIAL_TYPE_FEInterface);
	if(specialsCodeMapIt == specialsCodeMap.end())
	{
		__SS__
		    << "Could not find any FE Interface plugins in source code. Does MacroMaker "
		    << "have access to the source code? Check that the Supervisor context places "
		       "MacroMaker in a "
		    << "location with access to the source code." << __E__;
		__SS_THROW__;
	}

	// find first .h and .cc with the plugin name
	std::string headerFile      = pluginName + ".h";
	std::string sourceFile      = pluginName + "_interface.cc";
	bool        foundHeaderFile = false;
	bool        foundSourceFile = false;
	for(const auto& filePath : specialsCodeMapIt->second)
	{
		if(!foundHeaderFile && filePath.find(headerFile) != std::string::npos)
		{
			foundHeaderFile = true;
			headerFile      = filePath;
			__SUP_COUT__ << "found headerFile=" << filePath << __E__;
		}
		if(!foundSourceFile && filePath.find(sourceFile) != std::string::npos)
		{
			foundSourceFile = true;
			sourceFile      = filePath;
			__SUP_COUT__ << "found sourceFile=" << filePath << __E__;
		}

		if(foundSourceFile && foundHeaderFile)
			break;
	}  // end file search loop

	if(!foundHeaderFile)
	{
		__SS__ << "Could not find the header file for the FE Interface plugins at '"
		       << headerFile << ".' Does MacroMaker "
		       << "have access to the source code? Check that the Supervisor context "
		          "places MacroMaker in a "
		       << "location with access to the source code." << __E__;
		__SS_THROW__;
	}
	if(!foundSourceFile)
	{
		__SS__ << "Could not find the source file for the FE Interface plugins at '"
		       << sourceFile << ".' Does MacroMaker "
		       << "have access to the source code? Check that the Supervisor context "
		          "places MacroMaker in a "
		       << "location with access to the source code." << __E__;
		__SS_THROW__;
	}

	// at this point have header and source file, now add FE Macro
	// Steps for each file:
	//	- read current file
	//	- find insert point
	//	- open file for writing
	//		- write original file up to insert point
	//		- insert new code
	//		- write remaining original file

	char timeBuffer[100];
	{  // get time string
		time_t     rawtime;
		struct tm* timeinfo;

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(timeBuffer, 100, "%b-%d-%Y %I:%M:%S", timeinfo);
	}

	std::string contents;
	std::string insert;

	////////////////////////////
	// handle source file modifications
	CodeEditor::readFile(sourceFile, contents);
	//__SUP_COUTV__(contents);

	// return file locations, for the user to inspect on error
	xmldoc.addTextElementToData("sourceFile", sourceFile);
	xmldoc.addTextElementToData("headerFile", headerFile);

	// check for duplicate functions
	if(contents.find(pluginName + "::" + macroName) != std::string::npos)
	{
		__SS__ << "The function definition '" << (pluginName + "::" + macroName)
		       << "(...)' already exists in the source file '" << sourceFile
		       << ".' Duplicate functions are not allowed - please rename the macro or "
		          "modify the source file."
		       << __E__;
		__SS_THROW__;
	}

	std::stringstream     codess;
	std::set<std::string> inArgNames, outArgNames;
	createCode(codess,
	           commands,
	           "\t" /*tabOffset*/,
	           true /*forFeMacro*/,
	           &inArgNames,
	           &outArgNames);
	__SUP_COUTV__(StringMacros::setToString(inArgNames));
	__SUP_COUTV__(StringMacros::setToString(outArgNames));

	// find start of constructor and register macro
	{
		auto insertPos = contents.find(pluginName + "::" + pluginName);
		if(insertPos == std::string::npos)
		{
			__SS__ << "Could not find the code insert position in the source file '"
			       << sourceFile << ".' The FE plugin class constructor must be '"
			       << pluginName << ":" << pluginName << "' - is this the case?" << __E__;
			__SS_THROW__;
		}
		__SUP_COUTV__(insertPos);
		// find opening bracket after constructor name
		insertPos = contents.find("{", insertPos);
		if(insertPos == std::string::npos)
		{
			__SS__ << "Could not find the code insert position in the source file '"
			       << sourceFile
			       << ".' The FE plugin class constructor must begin with '{"
			       << "' - is this the case?" << __E__;
			__SS_THROW__;
		}
		++insertPos;  // go past {
		__SUP_COUTV__(insertPos);

		insert = "\n\t//registration of FEMacro '" + macroName + "' generated, " +
		         timeBuffer + ", by '" + username + "' using MacroMaker.\n\t" +
		         "FEVInterface::registerFEMacroFunction(\"" + macroName +
		         "\",//feMacroName \n\t\t" +
		         "static_cast<FEVInterface::frontEndMacroFunction_t>(&" + pluginName +
		         "::" + macroName + "), //feMacroFunction \n\t\t" +
		         "std::vector<std::string>{";
		{  // insert input argument names
			bool first = true;
			for(const auto& inArg : inArgNames)
			{
				if(first)
					first = false;
				else
					insert += ",";
				insert += "\"" + inArg + "\"";
			}
		}
		insert += "}, //namesOfInputArgs \n\t\t";
		insert += "std::vector<std::string>{";
		{  // insert output argument names
			bool first = true;
			for(const auto& outArg : outArgNames)
			{
				if(first)
					first = false;
				else
					insert += ",";
				insert += "\"" + outArg + "\"";
			}
		}
		insert += "}, //namesOfOutputArgs \n\t\t";
		insert += "1); //requiredUserPermissions \n\n";

		__SUP_COUTV__(insert);
		contents = contents.substr(0, insertPos) + insert + contents.substr(insertPos);
	}

	// find end of source to append FE Macro function
	{
		auto insertPos = contents.rfind("DEFINE_OTS_INTERFACE");
		if(insertPos == std::string::npos)
		{
			__SS__ << "Could not find the code insert position in the source file '"
			       << sourceFile
			       << ".' The FE plugin class must end with a 'DEFINE_OTS_INTERFACE("
			       << pluginName << ")' - is this the case?" << __E__;
			__SS_THROW__;
		}
		__SUP_COUTV__(insertPos);

		insert =
		    "\n//"
		    "============================================================================"
		    "============================================\n//" +
		    macroName + "\n" + "//\tFEMacro '" + macroName + "' generated, " +
		    timeBuffer + ", by '" + username + "' using MacroMaker.\n" +
		    "//\tMacro Notes: " + macroNotes + "\n" + "void " + pluginName +
		    "::" + macroName + "(__ARGS__)\n{\n\t" +
		    "__CFG_COUT__ << \"# of input args = \" << argsIn.size() << __E__; \n\t" +
		    "__CFG_COUT__ << \"# of output args = \" << argsOut.size() << __E__; \n\t" +
		    "for(auto &argIn:argsIn) \n\t\t" +
		    "__CFG_COUT__ << argIn.first << \": \" << argIn.second << __E__; \n\n\t" +
		    "//macro commands section \n" + codess.str() + "\n\n\t" +
		    "for(auto &argOut:argsOut) \n\t\t" +
		    "__CFG_COUT__ << argOut.first << \": \" << argOut.second << __E__; \n\n" +
		    "} //end " + macroName + "()\n\n";

		//__SUP_COUTV__(insert);
		CodeEditor::writeFile(sourceFile, contents, insertPos, insert);
	}

	////////////////////////////
	// handle include file insertions
	CodeEditor::readFile(headerFile, contents);
	//__SUP_COUTV__(contents);

	// find end of class by looking for last };
	{
		auto insertPos = contents.rfind("};");
		if(insertPos == std::string::npos)
		{
			__SS__ << "Could not find the code insert position in the header file '"
			       << headerFile
			       << ".' The FE plugin class must end with a '};' - is this the case?"
			       << __E__;
			__SS_THROW__;
		}

		__SUP_COUTV__(insertPos);

		insert = "\npublic: // FEMacro '" + macroName + "' generated, " + timeBuffer +
		         ", by '" + username + "' using MacroMaker.\n\t" + "void " + macroName +
		         "\t(__ARGS__);\n";

		__SUP_COUTV__(insert);
		CodeEditor::writeFile(headerFile, contents, insertPos, insert);
	}

}  // end exportFEMacro ()

//========================================================================================================================
void MacroMakerSupervisor::exportMacro(HttpXmlDocument&   xmldoc,
                                       cgicc::Cgicc&      cgi,
                                       const std::string& username)
{
	std::string macroName     = CgiDataUtilities::getData(cgi, "MacroName");
	std::string macroSequence = CgiDataUtilities::postData(cgi, "MacroSequence");
	std::string macroNotes    = CgiDataUtilities::decodeURIComponent(
        CgiDataUtilities::postData(cgi, "MacroNotes"));

	__SUP_COUTV__(macroName);
	__SUP_COUTV__(macroSequence);

	// replace all special characters with white space
	for(unsigned int i = 0; i < macroNotes.length(); ++i)
		if(macroNotes[i] == '\r' || macroNotes[i] == '\n')
			macroNotes[i] = ' ';
	__SUP_COUTV__(macroNotes);

	std::stringstream        ss(macroSequence);
	std::string              command;
	std::vector<std::string> commands;

	while(getline(ss, command, ','))
		commands.push_back(command);

	std::string fileName = macroName + ".cc";

	std::string fullPath = (std::string)MACROS_EXPORT_PATH + username + "/" + fileName;
	__SUP_COUT__ << fullPath << __E__;
	std::ofstream exportFile(fullPath.c_str(), std::ios::trunc);
	if(exportFile.is_open())
	{
		exportFile << "//Generated Macro Name:\t" << macroName << "\n";
		exportFile << "//Macro Notes: " << macroNotes << "\n";

		{
			time_t     rawtime;
			struct tm* timeinfo;
			char       buffer[100];

			time(&rawtime);
			timeinfo = localtime(&rawtime);

			strftime(buffer, 100, "%b-%d-%Y %I:%M:%S", timeinfo);
			exportFile << "//Generated Time: \t\t" << buffer << "\n";
		}

		exportFile << "//Paste this whole file into an interface to transfer Macro "
		              "functionality.\n";

		createCode(exportFile, commands);

		exportFile.close();

		xmldoc.addTextElementToData("ExportFile", fullPath);
	}
	else
		__SUP_COUT__ << "Unable to open file" << __E__;
}

//========================================================================================================================
// createCode
void MacroMakerSupervisor::createCode(std::ostream&                   out,
                                      const std::vector<std::string>& commands,
                                      const std::string&              tabOffset,
                                      bool                            forFeMacro,
                                      std::set<std::string>*          inArgNames,
                                      std::set<std::string>*          outArgNames)
{
	int                                 numOfHexBytes;
	std::set<std::string /*argInName*/> argInHasBeenInitializedSet;
	bool                                addressIsVariable, dataIsVariable;

	out << tabOffset << "{";

	out << "\n"
	    << tabOffset << "\t"
	    << "char *address \t= new char[universalAddressSize_]{0};	//create address "
	       "buffer of interface size and init to all 0";
	out << "\n"
	    << tabOffset << "\t"
	    << "char *data \t\t= new char[universalDataSize_]{0};		//create data buffer "
	       "of interface size and init to all 0";

	out << "\n"
	    << tabOffset << "\t"
	    << "uint64_t macroAddress;		//create macro address buffer (size 8 bytes)";
	out << "\n"
	    << tabOffset << "\t"
	    << "uint64_t macroData;			//create macro address buffer (size 8 bytes)";

	out << "\n"
	    << tabOffset << "\t"
	    << "std::map<std::string /*arg name*/,uint64_t /*arg val*/> macroArgs; //create "
	       "map from arg name to 64-bit number";

	// loop through each macro command
	for(unsigned int i = 0; i < commands.size(); i++)
	{
		std::stringstream sst(commands[i]);
		std::string       tokens;
		std::vector<std::string>
		    oneCommand;  // 4 fields: cmd index | cmd type | addr | data
		while(getline(sst, tokens, ':'))
			oneCommand.push_back(tokens);
		while(oneCommand.size() < 4)
			oneCommand.push_back("");  // fill out the 4 fields

		__SUP_COUTV__(StringMacros::vectorToString(oneCommand));

		// make this:
		//			std::map<std::string,uint64_t> macroArgs;
		//			{
		//				uint64_t address = 0x1001;	//create address buffer
		//				uint64_t data = 0x100203; 	//create data buffer
		//
		//				universalWrite(address,data);
		//				universalRead(address,data);
		//			}
		//
		//			//if variable, first time init
		//			{
		//				address =
		// theXDAQContextConfigTree_.getNode(theConfigurationPath_).getNode("variableName").getValue<uint64_t>();
		//				or
		//				address = __GET_ARG_IN__("variableName",uint64_t);
		//			}
		//
		//			//if variable, second time use macroArgs
		//			{
		//				address = macroArgs["variableName"];
		//				data = macroArgs["variableName"];
		//			}

		addressIsVariable = isArgumentVariable(oneCommand[2]);
		dataIsVariable    = isArgumentVariable(oneCommand[3]);

		__SUP_COUTV__(addressIsVariable);
		__SUP_COUTV__(dataIsVariable);

		out << "\n\n" << tabOffset << "\t// command-#" << i << ": ";

		if(oneCommand[1][0] == 'w' || oneCommand[1][0] == 'r')
		{
			if(oneCommand[1][0] == 'w')
				out << "Write(";
			else if(oneCommand[1][0] == 'r')
				out << "Read(";

			if(addressIsVariable)
				out << oneCommand[2];
			else  // literal hex address
				out << "0x" << oneCommand[2];
			out << " /*address*/,";

			if(dataIsVariable)  // read or write can have variable data, sink or source
			                    // respectively
				out << oneCommand[3] << " /*data*/";
			else if(oneCommand[1][0] == 'w')  // literal hex data
				out << "0x" << oneCommand[3] << " /*data*/";
			else if(oneCommand[1][0] == 'r')  // just reading to buffer
				out << "data";
			out << ");\n";
		}
		else if(oneCommand[1][0] == 'd')
		{
			out << "delay(" << oneCommand[2] << ");\n";
			out << tabOffset << "\t"
			    << "__CFG_COUT__ << \"Sleeping for... \" << " << oneCommand[2]
			    << " << \" milliseconds \" << __E__;\n";
			out << tabOffset << "\t"
			    << "usleep(" << oneCommand[2] << "*1000 /* microseconds */);\n";
			continue;
		}
		else
		{
			__SS__ << "FATAL ERROR: Unknown command '" << oneCommand[1]
			       << "'... command is not w, r or d" << __E__;
			__SS_THROW__;
		}

		//////////
		// handle address
		if(addressIsVariable)  // handle address as variable
		{
			if(argInHasBeenInitializedSet.find(oneCommand[2]) ==
			   argInHasBeenInitializedSet.end())  // only initialize input argument once
			{
				argInHasBeenInitializedSet.emplace(oneCommand[2]);

				if(!forFeMacro)
				{
					// get address from configuration Tree
					out << tabOffset << "\t"
					    << "macroArgs[\"" << oneCommand[2]
					    << "\"] = "
					       "theXDAQContextConfigTree_.getNode(theConfigurationPath_)."
					       "getNode("
					    << "\n"
					    << tabOffset << "\t\t\"" << oneCommand[2]
					    << "\").getValue<uint64_t>();";
				}
				else
				{
					if(inArgNames)
						inArgNames->emplace(oneCommand[2]);

					// get address from arguments
					out << tabOffset << "\t"
					    << "macroArgs[\"" << oneCommand[2] << "\"] = __GET_ARG_IN__(\""
					    << oneCommand[2] << "\", uint64_t);";
				}
			}
			out << "\t//get macro address argument";
			out << "\n"
			    << tabOffset << "\tmemcpy(address,&macroArgs[\"" << oneCommand[2]
			    << "\"],8); //copy macro address argument to buffer";
		}
		else  // handle address as literal
		{
			out << tabOffset << "\t"
			    << "macroAddress = 0x" << oneCommand[2]
			    << "; memcpy(address,&macroAddress,8);"
			    << "\t//copy macro address to buffer";
		}

		//////////
		// handle data
		if(oneCommand[1] == "w")  // if write, handle data too
		{
			if(dataIsVariable)  // handle data as variable
			{
				if(argInHasBeenInitializedSet.find(oneCommand[3]) ==
				   argInHasBeenInitializedSet
				       .end())  // only initialize input argument once
				{
					argInHasBeenInitializedSet.emplace(oneCommand[3]);

					if(forFeMacro)
					{
						if(inArgNames)
							inArgNames->emplace(oneCommand[3]);

						// get data from arguments
						out << "\n"
						    << tabOffset << "\t"
						    << "macroArgs[\"" << oneCommand[3]
						    << "\"] = __GET_ARG_IN__(\"" << oneCommand[3]
						    << "\", uint64_t); //initialize from input arguments";
					}
					else
					{
						// get data from configuration Tree
						out << "\n"
						    << tabOffset << "\t"
						    << "macroArgs[\"" << oneCommand[3]
						    << "\"] = "
						       "theXDAQContextConfigTree_.getNode(theConfigurationPath_)."
						       "getNode("
						    << "\n"
						    << tabOffset << "\t\t\"" << oneCommand[3]
						    << "\").getValue<uint64_t>(); //initialize from "
						       "configuration tree";
					}
				}
				out << "\t//get macro data argument";
				out << "\n"
				    << tabOffset << "\tmemcpy(data,&macroArgs[\"" << oneCommand[3]
				    << "\"],8); //copy macro data argument to buffer";
			}
			else  // handle data as literal
			{
				out << "\n"
				    << tabOffset << "\t"
				    << "macroData = 0x" << oneCommand[3] << "; memcpy(data,&macroData,8);"
				    << "\t//copy macro data to buffer";
			}
			out << "\n"
			    << tabOffset << "\t"
			    << "universalWrite(address,data);";
		}
		else
		{
			out << "\n"
			    << tabOffset << "\t"
			    << "universalRead(address,data);";

			std::string outputArgName;

			if(dataIsVariable)  // handle data as variable
				outputArgName = oneCommand[3];
			else  // give each read data a unique argument name
			{
				char str[20];
				sprintf(str, "outArg%d", i);
				outputArgName = str;  // use command index for uniqueness
			}
			__SUP_COUTV__(outputArgName);

			out << tabOffset << "\t"
			    << "memcpy(&macroArgs[\"" << outputArgName
			    << "\"],data,8); //copy buffer to argument map";

			// copy read data to output args
			if(forFeMacro)
				out << "\n"
				    << tabOffset << "\t"
				    << "__SET_ARG_OUT__(\"" << outputArgName << "\",macroArgs[\""
				    << outputArgName << "\"]); //update output argument result";

			if(outArgNames)
				outArgNames->emplace(outputArgName);
			argInHasBeenInitializedSet.emplace(
			    outputArgName);  // mark initialized since value has been read
		}
	}  // end command loop

	out << "\n\n" << tabOffset << "\tdelete[] address; //free the memory";
	out << "\n" << tabOffset << "\tdelete[] data; //free the memory";
	out << "\n" << tabOffset << "}";

	__SUP_COUT__ << "Done with code generation." << __E__;
}  // end createCode()

//========================================================================================================================
// isArgumentVariable
//	returns true if string should be interpreted as a variable for MacroMaker
bool MacroMakerSupervisor::isArgumentVariable(const std::string& argumentString)
{
	for(unsigned int i = 0; i < argumentString.length(); ++i)
	{
		// detect non-hex
		if(!((argumentString[i] >= '0' && argumentString[i] <= '9') ||
		     (argumentString[i] >= 'a' && argumentString[i] <= 'f') ||
		     (argumentString[i] >= 'A' && argumentString[i] <= 'F')))
			return true;
	}
	return false;
}  // end isArgumentVariable()
//========================================================================================================================
// generateHexArray
//	returns a char array initializer
//	something like this
//	"[8] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x09};"
//		..depending a size of source string
//
// FIXME -- identify variables in a better way from macromaker...!
//	for now just assume a non hex is a variable name
//	return -1 size
std::string MacroMakerSupervisor::generateHexArray(const std::string& sourceHexString,
                                                   int&               numOfBytes)
{
	std::stringstream retSs;

	std::string srcHexStr = sourceHexString;
	__SUP_COUT__ << "Translating: \n";
	__SUP_COUT__ << srcHexStr << __E__;

	if(srcHexStr.size() % 2)  // if odd, make even
		srcHexStr = "0" + srcHexStr;

	numOfBytes = srcHexStr.size() / 2;
	retSs << "[" << numOfBytes << "] = {";

	for(int i = 0; i < numOfBytes * 2; i += 2)
	{
		// detect non-hex
		if(!((srcHexStr[i] >= '0' && srcHexStr[i] <= '9') ||
		     (srcHexStr[i] >= 'a' && srcHexStr[i] <= 'f') ||
		     (srcHexStr[i] >= 'A' && srcHexStr[i] <= 'F')) ||
		   !((srcHexStr[i + 1] >= '0' && srcHexStr[i + 1] <= '9') ||
		     (srcHexStr[i + 1] >= 'a' && srcHexStr[i + 1] <= 'f') ||
		     (srcHexStr[i + 1] >= 'A' && srcHexStr[i + 1] <= 'F')))
		{
			numOfBytes = -1;
			return srcHexStr;
		}

		if(i != 0)
			retSs << ", ";
		retSs << "0x" << srcHexStr[srcHexStr.size() - 1 - i - 1]
		      << srcHexStr[srcHexStr.size() - 1 - i];
	}
	retSs << "};";

	__SUP_COUT__ << retSs.str() << __E__;

	return retSs.str();
}

//========================================================================================================================
void MacroMakerSupervisor::runFEMacro(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi)
{
	__SUP_COUT__ << __E__;

	unsigned int feSupervisorID = CgiDataUtilities::getDataAsInt(cgi, "feSupervisorID");
	std::string  feUID          = CgiDataUtilities::getData(cgi, "feUID");
	std::string  macroName      = CgiDataUtilities::getData(cgi, "macroName");
	std::string  inputArgs      = CgiDataUtilities::postData(cgi, "inputArgs");
	std::string  outputArgs     = CgiDataUtilities::postData(cgi, "outputArgs");

	__SUP_COUTV__(feSupervisorID);
	__SUP_COUTV__(feUID);
	__SUP_COUTV__(macroName);
	__SUP_COUTV__(inputArgs);
	__SUP_COUTV__(outputArgs);

	// send command to chosen FE and await response
	SOAPParameters txParameters;  // params for xoap to send
	txParameters.addParameter("Request", "RunInterfaceMacro");
	txParameters.addParameter("InterfaceID", feUID);
	txParameters.addParameter("feMacroName", macroName);
	txParameters.addParameter("inputArgs", inputArgs);
	txParameters.addParameter("outputArgs", outputArgs);

	SOAPParameters rxParameters;  // params for xoap to recv
	rxParameters.addParameter("success");
	rxParameters.addParameter("outputArgs");

	// find feSupervisorID in target list
	auto supervisorDescriptorPairIt = allFESupervisorInfo_.find(feSupervisorID);
	if(supervisorDescriptorPairIt == allFESupervisorInfo_.end())
	{
		__SS__ << "Targeted Supervisor Descriptor was not found. Attempted target "
		       << "was UID=" << feUID << " at feSupervisorID=" << feSupervisorID << "."
		       << __E__;
		__SUP_COUT_ERR__ << "\n" << ss.str();
		xmldoc.addTextElementToData("Error", ss.str());
		return;
	}

	// have FE supervisor descriptor, so send
	xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
	    supervisorDescriptorPairIt->second.getDescriptor(),  // supervisor descriptor
	    "MacroMakerSupervisorRequest",
	    txParameters);
	SOAPUtilities::receive(retMsg, rxParameters);

	__SUP_COUT__ << "Received it " << __E__;

	bool success = rxParameters.getValue("success") == "1";
	outputArgs   = rxParameters.getValue("outputArgs");

	__SUP_COUT__ << "rx success = " << success << __E__;
	__SUP_COUT__ << "outputArgs = " << outputArgs << __E__;

	if(!success)
	{
		__SS__ << "Attempted FE Macro Failed. Attempted target "
		       << "was UID=" << feUID << " at feSupervisorID=" << feSupervisorID << "."
		       << __E__;
		ss << "\n\n The error was:\n\n" << outputArgs << __E__;
		__SUP_COUT_ERR__ << "\n" << ss.str();
		xmldoc.addTextElementToData("Error", ss.str());
		return;
	}

	// build output arguments
	//	parse args, colon-separated pairs, and then comma-separated
	{
		std::istringstream inputStream(outputArgs);
		std::string        splitVal, argName, argValue;
		while(getline(inputStream, splitVal, ';'))
		{
			std::istringstream pairInputStream(splitVal);
			getline(pairInputStream, argName, ',');
			getline(pairInputStream, argValue, ',');
			xmldoc.addTextElementToData("outputArgs_name", argName);
			xmldoc.addTextElementToData("outputArgs_value", argValue);
			__SUP_COUT__ << argName << ": " << argValue << __E__;
		}
	}
}

//========================================================================================================================
void MacroMakerSupervisor::getFEMacroList(HttpXmlDocument&   xmldoc,
                                          const std::string& username)
{
	__SUP_COUT__ << "Getting FE Macro list" << __E__;

	SOAPParameters txParameters;  // params for xoap to send
	txParameters.addParameter("Request", "GetInterfaceMacros");

	SOAPParameters rxParameters;  // params for xoap to recv
	rxParameters.addParameter("FEMacros");

	std::string oneInterface;
	std::string rxFEMacros;

	// for each list of FE Supervisors,
	//			get all FE specific macros
	for(auto& appInfo : allFESupervisorInfo_)
	{
		__SUP_COUT__ << "FESupervisor LID = " << appInfo.second.getId()
		             << " name = " << appInfo.second.getName() << __E__;

		xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
		    appInfo.second.getDescriptor(), "MacroMakerSupervisorRequest", txParameters);
		SOAPUtilities::receive(retMsg, rxParameters);

		rxFEMacros = rxParameters.getValue("FEMacros");

		__SUP_COUT__ << "FE Macros received: \n" << rxFEMacros << __E__;

		std::istringstream allInterfaces(rxFEMacros);
		while(std::getline(allInterfaces, oneInterface))
		{
			//__SUP_COUT__ << oneInterface << __E__;
			//__SUP_COUT__ << appInfo.second.getId() << __E__;
			xmldoc.addTextElementToData("FEMacros", oneInterface);
			// xmldoc.outputXmlDocument(0,true);
		}
	}

	// add macros to response
	loadMacros(xmldoc, username);

	return;
}
