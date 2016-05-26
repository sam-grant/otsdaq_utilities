#include "otsdaq-utilities/MacroMaker/MacroMakerSupervisor.h"

#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"
#include "otsdaq-core/SOAPUtilities/SOAPUtilities.h"
#include "otsdaq-core/SOAPUtilities/SOAPParameters.h"
#include "otsdaq-core/ConfigurationDataFormats/ConfigurationKey.h"
#include "otsdaq-core/ConfigurationInterface/ConfigurationManager.h"


#include <xdaq/NamespaceURI.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h> //for DIR
#include <sys/stat.h> //for mkdir

#define MACROS_DB_PATH 					"otsdaq_demo/NoGitData/ServiceData/LoginData/UsersData/MacroData/"

using namespace ots;


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
	mf::LogDebug(__FILE__)<< "PixelFESupervisor instance size " <<
			theSupervisorsConfiguration_.getFEDescriptors().size() << "     ";
	for (; it != theSupervisorsConfiguration_.getFEDescriptors().end(); it++)
	{
		mf::LogDebug(__FILE__)<< "PixelFESupervisor instance " << it->first <<
				"...and..." << it->second << "     ";
		mf::LogDebug(__FILE__)<< "Look! Here's a FE! @@@" << "     ";
	}
	mkdir(((std::string)MACROS_DB_PATH).c_str(), 0755);
//	//getARTDAQFEDescriptors
//	for (const auto& it: theSupervisorsConfiguration_.getFEDescriptors())
//	{
//		mf::LogDebug(__FILE__)<< "PixelFESupervisor instance " << it.first << "     ";
//		mf::LogDebug(__FILE__)<< "Look! Here's a FE! @@@" << "     ";
//	}
//	for (const auto& it: theSupervisorsConfiguration_.getFEDataManagerDescriptors())
//	{
//		mf::LogDebug(__FILE__)<< "PixelFEDataManagerSupervisor instance " << it.first << "     ";
//		mf::LogDebug(__FILE__)<< "Look! Here's a FE! @@@" << "     ";
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
	mf::LogDebug(__FILE__)<< "#######################################" << "     ";
	mf::LogDebug(__FILE__)<< "#######################################" << "     ";

	mf::LogDebug(__FILE__)<< "Running in MacroMaker Supervisor" << "     ";

	mf::LogDebug(__FILE__)<< "#######################################" << "     ";
	mf::LogDebug(__FILE__)<< "#######################################" << "     ";
}

//========================================================================================================================
void MacroMakerSupervisor::destroy(void)
{
	//called by destructor

}

//========================================================================================================================
void MacroMakerSupervisor::Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
}

//========================================================================================================================
void MacroMakerSupervisor::MacroMakerRequest(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception)
{
	

	cgicc::Cgicc cgi(in);
	std::string Command = CgiDataUtilities::getData(cgi, "RequestType");
	mf::LogDebug(__FILE__) << "Command: " << Command << "     ";

	//FIXME -- need to lock out MacroMaker vs State machine

	//**** start LOGIN GATEWAY CODE ***//
	//If TRUE, cookie code is good, and refreshed code is in cookieCode, also pointers optionally for UInt8 userPermissions
	//Else, error message is returned in cookieCode
	uint8_t userPermissions;
	std::string cookieCode = Command == "PreviewEntry"? cgi("CookieCode"):
			CgiDataUtilities::postData(cgi,"CookieCode");
	if(!theRemoteWebUsers_.cookieCodeIsActiveForRequest(theSupervisorsConfiguration_.getSupervisorDescriptor(),
			cookieCode, &userPermissions)) //only refresh cookie if not automatic refresh
	{
		*out << cookieCode;
		mf::LogDebug(__FILE__) << "Invalid Cookie Code" << "     ";
		return;
	}
	//**** end LOGIN GATEWAY CODE ***//

	


	HttpXmlDocument xmldoc(cookieCode);
	handleRequest(Command,xmldoc,cgi);
	//return xml doc holding server response
	xmldoc.outputXmlDocument((std::ostringstream*) out, false);
}

//
//
//#include "otsdaq-core/Supervisor/MacroMaker.h"
//#include "otsdaq-core/MessageFacility/MessageFacility.h"
//#include "otsdaq-core/SupervisorConfigurations/SupervisorConfiguration.h"
//#include "otsdaq-core/Supervisor/SupervisorsInfo.h"
//#include "otsdaq-core/ConfigurationInterface/ConfigurationManager.h"
//#include "otsdaq-demo/FEInterfaces/FEInterfacesManager.h"
////#include "otsdaq-demo/FEInterfaces/FEVInterface.h"
////#include "otsdaq-demo/FEInterfaces/FEZEDRyanInterface.h"
//
//#include <iostream>
//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//#include <sstream>
//
//
//
//
//using namespace ots;
//
//////
//
////========================================================================================================================
//MacroMaker::MacroMaker(SupervisorConfiguration* superConfig, SupervisorsInfo* superInfo)
//{
//	superConfiguration_ = superConfig;
//	superInfo_ = superInfo;
//
////	const int supervisorInstance_    = 1;
////	const int configurationKeyValue_ = 0;
////	const ConfigurationKey* theConfigurationKey_ = new ConfigurationKey(configurationKeyValue_);
////
////	theConfigurationManager_ = new ConfigurationManager;
////    theFEInterfacesManager_ = new FEInterfacesManager(theConfigurationManager_, supervisorInstance_);
////	theConfigurationManager_->setupFESupervisorConfiguration(theConfigurationKey_,supervisorInstance_);
////	theFEInterfacesManager_->createInterfaces();
//}
//
//MacroMaker::~MacroMaker()
//{
////	delete theFEInterfacesManager_;
////	delete theConfigurationManager_;
//}
////========================================================================================================================
//void MacroMaker::printStatus()
//{
//////	mf::LogDebug(__FILE__) << "\n\nGetting Supervisor Status\n\n" << "     ";
////	//super_->getSupervisorsStatus();
////	//super_->theSupervisorsConfiguration_;
//////	SupervisorDescriptors::const_iterator it =
//////			superConfiguration_->getFEDescriptors().begin();
//////	for (; it != superConfiguration_->getFEDescriptors().end();
//////			it++) {
//////		std::string state = "";
//////		//send(it->second,"StateMachineStateRequest");
//////
//////		superInfo_->getFESupervisorInfo(it->first).setStatus(
//////				state);
//////		mf::LogDebug(__FILE__)<< "PixelFESupervisor instance " << it->first << " is in FSM state " << state << "     ";
//////		//it->write(1,0);
//////		mf::LogDebug(__FILE__)<< "Look! Here's a FE! @@@" << "     ";
//////
//////	}
//////	it = superConfiguration_->getARTDAQFEDescriptors().begin();
//////	for (; it != superConfiguration_->getARTDAQFEDescriptors().end();
//////			it++) {
//////		std::string state = "";
//////		//		send(it->second,"StateMachineStateRequest");
//////		superInfo_->getARTDAQFESupervisorInfo(it->first).setStatus(
//////				state);
//////		mf::LogDebug(__FILE__)<< "PixelARTDAQFESupervisor instance " << it->first << " is in FSM state " << state << "     ";
//////		mf::LogDebug(__FILE__)<< "Look! Here's a ARTDAQFE! @@@" << "     ";
//////	}
////
//////	//example
//////	// read registers
////
////
////
//////Writing a "1"
////	std::string writeValue(8,0);
////	//writeValue.resize(8);
////	//writeValue += (char)0;
////	//writeValue[0] = (char)9;
////	uint64_t mywriteval = 0xABCDEF;
////	memcpy(&writeValue[0],&mywriteval,8);
//////End of formating
////
////	std::string readValue = "";
////	for(unsigned int i=0;i<theFEInterfacesManager_->theFEInterfaces_.size();++i)
////	{
////		mf::LogDebug(__FILE__)<< "Interface: " << i << "     ";
////
////		std::string FEType = theFEInterfacesManager_->theFEInterfaces_[i]->getFEType();
////
////		if (FEType == "OtsUDPHardware"){
////			mf::LogDebug(__FILE__)<< "Type: " << FEType << "     ";
////			((FEZEDRyanInterface *)(theFEInterfacesManager_->theFEInterfaces_[i]))->interfaceWrite(0x000000064, writeValue);
////			((FEZEDRyanInterface *)(theFEInterfacesManager_->theFEInterfaces_[i]))->interfaceRead(0x000000064, readValue);
////		}
////		else
////			mf::LogDebug(__FILE__)<< "FE type not recognized" << "     ";
////
////		mf::LogDebug(__FILE__)<< "Name: " << theFEInterfacesManager_->theFEInterfaces_[i]->getFEName() << "     ";
////
////		mf::LogDebug(__FILE__) <<"\tReading message:-";
////
////		printf("0x");
////		for(uint32_t i=0; i<readValue.size(); i++)
////			printf("%2.2X",(unsigned char)readValue[i]);
////			//mf::LogDebug(__FILE__) << std::hex << (int16_t)readValue[i] << "-";
////
////		mf::LogDebug(__FILE__) << std::dec << "     ";
////		mywriteval = 0;
////		memcpy(&mywriteval,&readValue[2],4);
////
////		mf::LogDebug(__FILE__) <<"\tReading value:-";
////		printf("0x%16.16lX",mywriteval);
////		mf::LogDebug(__FILE__) <<  "     ";
////	}
//}
//
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
}

void MacroMakerSupervisor::getFElist(HttpXmlDocument& xmldoc)
{
	mf::LogDebug(__FILE__) << "Getting FE list!!!!!!!!!" << "     ";

	SOAPParameters parameters; //params for xoap to send
	parameters.addParameter("Request", "GetInterfaces");

	SOAPParameters retParameters;  //params for xoap to recv
	retParameters.addParameter("FEList");

	SupervisorDescriptors::const_iterator it;
	it = theSupervisorsConfiguration_.getFEDescriptors().begin();

	mf::LogDebug(__FILE__)<< "PixelFESupervisor instance size " <<
			theSupervisorsConfiguration_.getFEDescriptors().size() << "     ";

	//loop through each front end, and send xoap request for front end list
	for (; it != theSupervisorsConfiguration_.getFEDescriptors().end(); it++)
	{
		mf::LogDebug(__FILE__)<< "PixelFESupervisor instance " << it->first <<
				"...and..." << it->second << "     ";
		mf::LogDebug(__FILE__)<< "Look! Here's a FE! @@@" << "     ";

		xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
				it->second,
				"MacroMakerSupervisorRequest",
				parameters);

		receive(retMsg, retParameters);
		std::string retMsgFEList = retParameters.getValue("FEList");

		mf::LogDebug(__FILE__)<< "FE List received : " <<
				retMsgFEList << "     ";

		std::istringstream f(retMsgFEList);
		std::string oneInterface;
		while (std::getline(f, oneInterface)){
			std::stringstream buffer;
			buffer << oneInterface.substr(0,oneInterface.rfind(":")+1)
				   << std::to_string(it->first) << oneInterface.substr(oneInterface.rfind(":"),oneInterface.length())<< "     ";
			interfaceList.push_back(buffer.str());
		    xmldoc.addTextElementToData("FE", buffer.str());
		}
	}
	return;
}

void MacroMakerSupervisor::writeData(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi)
{
	mf::LogDebug(__FILE__) << "¡¡¡¡¡¡MacroMaker wants to write data!!!!!!!!!" << "     ";
	std::string Address = CgiDataUtilities::getData(cgi, "Address");
	std::string Data = CgiDataUtilities::getData(cgi, "Data");
	std::string interfaceIndexArray = CgiDataUtilities::getData(cgi, "interfaceIndex");
	std::string supervisorIndexArray = CgiDataUtilities::getData(cgi, "supervisorIndex");

	SOAPParameters parameters; //params for xoap to send
		parameters.addParameter("Request", "UniversalWrite");
		parameters.addParameter("Address",Address);
		parameters.addParameter("Data",Data);

		mf::LogDebug(__FILE__) << "Address: " << Address << " Data: " << Data << "     ";

	mf::LogDebug(__FILE__) <<"Here comes the array from multiselect box for WRITE, behold: "
			<< supervisorIndexArray << interfaceIndexArray <<"     ";

	SupervisorDescriptors FESupervisors = theSupervisorsConfiguration_.getFEDescriptors();

	////////////////////////////////Store cgi arrays into vectors/////////////////////////////
	std::vector<int> interfaceIndices;
	std::istringstream f(interfaceIndexArray);
	std::string s;
	while (getline(f, s, ',')) interfaceIndices.push_back(std::stoi(s));
	std::vector<int> supervisorIndices;
	std::istringstream g(supervisorIndexArray);
	std::string t;
	while (getline(g, t, ',')) supervisorIndices.push_back(std::stoi(t));


    for(unsigned int i=0; i < supervisorIndices.size(); i++)
    {
    	unsigned int FEIndex = supervisorIndices[i];
    	unsigned int interfaceIndex = interfaceIndices[i];

    	parameters.addParameter("InterfaceIndex",interfaceIndex);

	    mf::LogDebug(__FILE__) <<"The index of the supervisor instance is: " << FEIndex << "     ";
	    mf::LogDebug(__FILE__) <<"...and the index of the interface is: " << interfaceIndex << "     ";

	    SupervisorDescriptors::iterator it = FESupervisors.find(FEIndex);
        if (it == FESupervisors.end())
	    {
			mf::LogDebug(__FILE__) << "ERROR!? FE Index doesn't exist" << "     ";
			return;
	    }

	    xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
				it->second,
				"MacroMakerSupervisorRequest",
				parameters);
	    receive(retMsg);
    }

	//	xoap::MessageReARTDAQFEence retMsg = SOAPMessenger::sendWithSOAPReply(theSupervisorsConfiguration_.getFEDescriptors().begin()->second,
	//			"MacroMakerSupervisorRequest",parameters);
	//			//Selected FE Descriptor,
	//			//Request Test Name,
	//			//Parameters
	//	

	//
	//

	//	std::string addressFormat = CgiDataUtilities::getData(cgi, "addressFormat");
	//	int addressFormatIndex = std::stoi(addressFormat);
	//
	//	mf::LogDebug(__FILE__) << "Raw address from server: " << Address << "     ";
	//	mf::LogDebug(__FILE__) << "Raw data from server: " << Data << "     ";
	//	mf::LogDebug(__FILE__) << "Format: " << addressFormatIndex << "     ";
	//
	//	std::uint64_t addr;
	//	if (addressFormatIndex == 1)
	//	  {
	//		//converting Address from std::string to uint64_t IN HEX
	//		std::stringstream ss;
	//		ss.str(Address);
	//		ss >> std::hex >> addr;
	//		mf::LogDebug(__FILE__) << "I am in if" << "     ";
	//
	//	  }
	//	else
	//	{
	//		//std::stringstream stream(Address);
	//	  //  stream >> std::hex >> addr;
	//
	//		//converting Address to uint64_t IN DEC
	//		std::string s(Address);
	//		std::stringstream strm(s);
	//		strm >> std::hex >> addr;
	//		mf::LogDebug(__FILE__) << "I am in else" << "     ";
	//	}
	//
	//	mf::LogDebug(__FILE__) << "Address sending to ZEDRyan: " << addr << "     ";
	//
	//	//Converting Data to uint64_t
	//	    std::stringstream stream(Data);
	//	    std::uint64_t mywriteval;
	//	    stream >> std::hex >> mywriteval;
	//
	//	std::string writeValue(8,0);
	//	memcpy(&writeValue[0],&mywriteval,8);
	//	for(unsigned int i=0;i<theFEInterfacesManager_->theFEInterfaces_.size();++i)
	//		((FEZEDRyanInterface *)(theFEInterfacesManager_->theFEInterfaces_[i]))->interfaceWrite(addr, writeValue);
	//
	//	mf::LogDebug(__FILE__) <<"\tValue written by user:-";
	//	printf("0x%16.16lX",mywriteval);
	//	
}

void MacroMakerSupervisor::readData(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi)
{
	mf::LogDebug(__FILE__) << "@@@@@@@ MacroMaker wants to read data @@@@@@@@" << "     ";
	std::string Address = CgiDataUtilities::getData(cgi, "Address");
	std::string interfaceIndexArray = CgiDataUtilities::getData(cgi, "interfaceIndex");
	std::string supervisorIndexArray = CgiDataUtilities::getData(cgi, "supervisorIndex");

	SOAPParameters parameters; //params for xoap to send
	parameters.addParameter("Request", "UniversalRead");
	parameters.addParameter("Address",Address);

	SOAPParameters retParameters;
    retParameters.addParameter("dataResult");
	mf::LogDebug(__FILE__) <<"Here comes the array from multiselect box for READ, behold: "
			<< supervisorIndexArray << "," << interfaceIndexArray << "     ";

	SupervisorDescriptors FESupervisors = theSupervisorsConfiguration_.getFEDescriptors();


	unsigned int FEIndex = stoi(supervisorIndexArray);
	unsigned int interfaceIndex = stoi(interfaceIndexArray);


	parameters.addParameter("InterfaceIndex",interfaceIndex);

	mf::LogDebug(__FILE__) <<"The index of the supervisor instance is: " << FEIndex << "     ";
	mf::LogDebug(__FILE__) <<"...and the index of the interface is: " << interfaceIndex << "     ";

	SupervisorDescriptors::iterator it = FESupervisors.find(FEIndex);
	if (it == FESupervisors.end())
	{
		mf::LogDebug(__FILE__) << "ERROR!? FE Index doesn't exist" << "     ";
		return;
	}

	xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
			it->second,
			"MacroMakerSupervisorRequest",
			parameters);

	receive(retMsg,retParameters);
	std::string dataReadReturnMsg = retParameters.getValue("dataResult");
	//Why is the thing above not working??
	//std::string dataReadReturnMsg = "123456";
	mf::LogDebug(__FILE__) << "Data reading result received: " << dataReadReturnMsg << "     ";
	xmldoc.addTextElementToData("readData",dataReadReturnMsg);


////	std::uint64_t addr;
////    if (addressFormatIndex == 1)
////      {
////		//converting Address from std::string to uint64_t IN HEX
////		std::stringstream ss;
////		ss.str(Address);
////		ss >> std::hex >> addr;
////      }
////    else
////    {
////    	//converting Address to uint64_t IN DEC
////		std::string s(Address);
////		std::stringstream strm(s);
////		strm >> std::hex >> addr;
////    }
////
////	mf::LogDebug(__FILE__) << "Address sending to ZEDRyan: " << addr << "     ";
////
////	std::string readValue = "";
////	for(unsigned int i=0;i<theFEInterfacesManager_->theFEInterfaces_.size();++i)
////	    ((FEZEDRyanInterface *)(theFEInterfacesManager_->theFEInterfaces_[i]))->interfaceRead(addr, readValue);
////
////	mf::LogDebug(__FILE__) <<"\tReading message:-";
////
////			printf("0x");
////			for(uint32_t i=0; i<readValue.size(); i++)
////				printf("%2.2X",(unsigned char)readValue[i]);
////
////			mf::LogDebug(__FILE__) << std::dec << "     ";
////	std::uint64_t myreadval = 0;
////	memcpy(&myreadval,&readValue[2],4);
////
////
////	char toJS[100];
////
////	sprintf(toJS,"0x%16.16lX",myreadval);
////	xmldoc.addTextElementToData("readData",toJS);
////
////	mf::LogDebug(__FILE__) <<"\tReading value from readData:-";
////	printf(toJS,"0x%16.16lX",myreadval);
////	mf::LogDebug(__FILE__) <<  "     ";
//
}
void MacroMakerSupervisor::createMacro(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi)
{
	mf::LogDebug(__FILE__) << "¡¡¡¡¡¡MacroMaker wants to create a macro!!!!!!!!!" << "     ";
	std::string Name = CgiDataUtilities::getData(cgi, "Name");
	std::string Sequence = CgiDataUtilities::getData(cgi, "Sequence");
	std::string Time = CgiDataUtilities::getData(cgi, "Time");
	std::string Notes = CgiDataUtilities::getData(cgi, "Notes");

	mf::LogDebug(__FILE__) << MACROS_DB_PATH << "     ";

	std::string fileName = Name + ".dat";
	std::string fullPath = (std::string)MACROS_DB_PATH + fileName;
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
		mf::LogDebug(__FILE__) << "Unable to open file" << "     ";
}

void MacroMakerSupervisor::loadMacros(HttpXmlDocument& xmldoc)
{
	DIR *dir;
	struct dirent *ent;
	std::string returnStr = "";
	if ((dir = opendir (MACROS_DB_PATH)) != NULL)
	{
	  /* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL)
		{
		/* File name validation check */
			if ((unsigned)strlen(ent->d_name) > 4)
			{
				std::string line;
				std::ifstream read (((MACROS_DB_PATH + (std::string)ent->d_name)).c_str());//reading a file
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
					  mf::LogDebug(__FILE__) << "Unable to open file" << "     ";
			}
		}
		mf::LogDebug(__FILE__) << "New macro created: " << returnStr.substr(0, returnStr.size()-1) << "     ";
		std::string returnMacroStr = returnStr.substr(0, returnStr.size()-1);
		closedir (dir);
		xmldoc.addTextElementToData("returnMacroStr",returnMacroStr);
	}
	else mf::LogDebug(__FILE__) << "Looping through MacroData folder failed! Wrong directory" << "     ";
}
