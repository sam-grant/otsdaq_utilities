#include "otsdaq-utilities/MacroMaker/MacroMakerSupervisor.h"

#include "otsdaq-core/Macros/OTSMacros.h"
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"
#include "otsdaq-core/SOAPUtilities/SOAPUtilities.h"
#include "otsdaq-core/SOAPUtilities/SOAPParameters.h"
#include "otsdaq-core/ConfigurationDataFormats/ConfigurationKey.h"
#include "otsdaq-core/ConfigurationInterface/ConfigurationManager.h"



#include <xdaq/NamespaceURI.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h> //for DIR
#include <sys/stat.h> //for mkdir

using namespace ots;


XDAQ_INSTANTIATOR_IMPL(MacroMakerSupervisor)

//========================================================================================================================
MacroMakerSupervisor::MacroMakerSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception):
xdaq::Application(s   ),
SOAPMessenger  (this),
theRemoteWebUsers_(this)
{

	xgi::bind (this, &MacroMakerSupervisor::Default,                	"Default" );
	xgi::bind (this, &MacroMakerSupervisor::MacroMakerRequest,          "MacroMakerRequest" );

	init();
	SupervisorDescriptors::const_iterator it;
	it = theSupervisorsConfiguration_.getFEDescriptors().begin();
	std::cout << __COUT_HDR__<< "PixelFESupervisor instance size " <<
			theSupervisorsConfiguration_.getFEDescriptors().size() << std::endl;
	for (; it != theSupervisorsConfiguration_.getFEDescriptors().end(); it++)
	{
		std::cout << __COUT_HDR__<< "PixelFESupervisor instance " << it->first <<
				"...and..." << it->second << std::endl;
		std::cout << __COUT_HDR__<< "Look! Here's a FE! @@@" << std::endl;
	}


//	//getARTDAQFEDescriptors
//	for (const auto& it: theSupervisorsConfiguration_.getFEDescriptors())
//	{
//		std::cout << __COUT_HDR__<< "PixelFESupervisor instance " << it.first << std::endl;
//		std::cout << __COUT_HDR__<< "Look! Here's a FE! @@@" << std::endl;
//	}
//	for (const auto& it: theSupervisorsConfiguration_.getFEDataManagerDescriptors())
//	{
//		std::cout << __COUT_HDR__<< "PixelFEDataManagerSupervisor instance " << it.first << std::endl;
//		std::cout << __COUT_HDR__<< "Look! Here's a FE! @@@" << std::endl;
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
	std::cout << __COUT_HDR__<< "#######################################" << std::endl;
	std::cout << __COUT_HDR__<< "#######################################" << std::endl;

	std::cout << __COUT_HDR__<< "Running in MacroMaker Supervisor" << std::endl;

	std::cout << __COUT_HDR__<< "#######################################" << std::endl;
	std::cout << __COUT_HDR__<< "#######################################" << std::endl;
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
	std::cout << __COUT_HDR__ << std::endl;

	cgicc::Cgicc cgi(in);
	std::string Command = CgiDataUtilities::getData(cgi, "RequestType");
	std::cout << __COUT_HDR__ << "Command: " << Command << std::endl;

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
		std::cout << __COUT_HDR__ << "Invalid Cookie Code" << std::endl;
		return;
	}
	//**** end LOGIN GATEWAY CODE ***//

	std::cout << __COUT_HDR__ << std::endl;


	HttpXmlDocument xmldoc(cookieCode);
	handleRequest(Command,xmldoc,cgi);
	//return xml doc holding server response
	xmldoc.outputXmlDocument((std::ostringstream*) out, false);
}

//
//
//#include "otsdaq-core/Supervisor/MacroMaker.h"
//#include "otsdaq-core/Macros/OTSMacros.h"
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
//////	std::cout << __COUT_HDR__ << "\n\nGetting Supervisor Status\n\n" << std::endl;
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
//////		std::cout << __COUT_HDR__<< "PixelFESupervisor instance " << it->first << " is in FSM state " << state << std::endl;
//////		//it->write(1,0);
//////		std::cout << __COUT_HDR__<< "Look! Here's a FE! @@@" << std::endl;
//////
//////	}
//////	it = superConfiguration_->getARTDAQFEDescriptors().begin();
//////	for (; it != superConfiguration_->getARTDAQFEDescriptors().end();
//////			it++) {
//////		std::string state = "";
//////		//		send(it->second,"StateMachineStateRequest");
//////		superInfo_->getARTDAQFESupervisorInfo(it->first).setStatus(
//////				state);
//////		std::cout << __COUT_HDR__<< "PixelARTDAQFESupervisor instance " << it->first << " is in FSM state " << state << std::endl;
//////		std::cout << __COUT_HDR__<< "Look! Here's a ARTDAQFE! @@@" << std::endl;
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
////		std::cout << __COUT_HDR__<< "Interface: " << i << std::endl;
////
////		std::string FEType = theFEInterfacesManager_->theFEInterfaces_[i]->getFEType();
////
////		if (FEType == "OtsUDPHardware"){
////			std::cout << __COUT_HDR__<< "Type: " << FEType << std::endl;
////			((FEZEDRyanInterface *)(theFEInterfacesManager_->theFEInterfaces_[i]))->interfaceWrite(0x000000064, writeValue);
////			((FEZEDRyanInterface *)(theFEInterfacesManager_->theFEInterfaces_[i]))->interfaceRead(0x000000064, readValue);
////		}
////		else
////			std::cout << __COUT_HDR__<< "FE type not recognized" << std::endl;
////
////		std::cout << __COUT_HDR__<< "Name: " << theFEInterfacesManager_->theFEInterfaces_[i]->getFEName() << std::endl;
////
////		std::cout << __COUT_HDR__ <<"\tReading message:-";
////
////		printf("0x");
////		for(uint32_t i=0; i<readValue.size(); i++)
////			printf("%2.2X",(unsigned char)readValue[i]);
////			//std::cout << std::hex << (int16_t)readValue[i] << "-";
////
////		std::cout << std::dec << std::endl;
////		mywriteval = 0;
////		memcpy(&mywriteval,&readValue[2],4);
////
////		std::cout << __COUT_HDR__ <<"\tReading value:-";
////		printf("0x%16.16lX",mywriteval);
////		std::cout <<  std::endl;
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
}

void MacroMakerSupervisor::getFElist(HttpXmlDocument& xmldoc)
{
	std::cout << __COUT_HDR__ << "Getting FE list!!!!!!!!!" << std::endl;

	SOAPParameters parameters; //params for xoap to send
	parameters.addParameter("Request", "GetInterfaces");

	SOAPParameters retParameters;  //params for xoap to recv
	retParameters.addParameter("FEList");

	SupervisorDescriptors::const_iterator it;
	it = theSupervisorsConfiguration_.getFEDescriptors().begin();

	std::cout << __COUT_HDR__<< "PixelFESupervisor instance size " <<
			theSupervisorsConfiguration_.getFEDescriptors().size() << std::endl;

	//loop through each front end, and send xoap request for front end list
	for (; it != theSupervisorsConfiguration_.getFEDescriptors().end(); it++)
	{
		std::cout << __COUT_HDR__<< "PixelFESupervisor instance " << it->first <<
				"...and..." << it->second << std::endl;
		std::cout << __COUT_HDR__<< "Look! Here's a FE! @@@" << std::endl;

		xoap::MessageReference retMsg = SOAPMessenger::sendWithSOAPReply(
				it->second,
				"MacroMakerSupervisorRequest",
				parameters);

		receive(retMsg, retParameters);
		std::string retMsgFEList = retParameters.getValue("FEList");

		std::cout << __COUT_HDR__<< "FE List received : " <<
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
	std::cout << __COUT_HDR__ << "¡¡¡¡¡¡MacroMaker wants to write data!!!!!!!!!" << std::endl;
	std::string Address = CgiDataUtilities::getData(cgi, "Address");
	std::string Data = CgiDataUtilities::getData(cgi, "Data");
	std::string interfaceIndexArray = CgiDataUtilities::getData(cgi, "interfaceIndex");
	std::string supervisorIndexArray = CgiDataUtilities::getData(cgi, "supervisorIndex");

	SOAPParameters parameters; //params for xoap to send
		parameters.addParameter("Request", "UniversalWrite");
		parameters.addParameter("Address",Address);
		parameters.addParameter("Data",Data);

		std::cout << __COUT_HDR__ << "Address: " << Address << " Data: " << Data << std::endl;

	std::cout << __COUT_HDR__ <<"Here comes the array from multiselect box for WRITE, behold: "
			<< supervisorIndexArray << interfaceIndexArray <<std::endl;

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

	    std::cout << __COUT_HDR__ <<"The index of the supervisor instance is: " << FEIndex << std::endl;
	    std::cout << __COUT_HDR__ <<"...and the index of the interface is: " << interfaceIndex << std::endl;

	    SupervisorDescriptors::iterator it = FESupervisors.find(FEIndex);
        if (it == FESupervisors.end())
	    {
			std::cout << __COUT_HDR__ << "ERROR!? FE Index doesn't exist" << std::endl;
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
	//	std::cout << __COUT_HDR__ << std::endl;

	//
	//

	//	std::string addressFormat = CgiDataUtilities::getData(cgi, "addressFormat");
	//	int addressFormatIndex = std::stoi(addressFormat);
	//
	//	std::cout << __COUT_HDR__ << "Raw address from server: " << Address << std::endl;
	//	std::cout << __COUT_HDR__ << "Raw data from server: " << Data << std::endl;
	//	std::cout << __COUT_HDR__ << "Format: " << addressFormatIndex << std::endl;
	//
	//	std::uint64_t addr;
	//	if (addressFormatIndex == 1)
	//	  {
	//		//converting Address from std::string to uint64_t IN HEX
	//		std::stringstream ss;
	//		ss.str(Address);
	//		ss >> std::hex >> addr;
	//		std::cout << __COUT_HDR__ << "I am in if" << std::endl;
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
	//		std::cout << __COUT_HDR__ << "I am in else" << std::endl;
	//	}
	//
	//	std::cout << __COUT_HDR__ << "Address sending to ZEDRyan: " << addr << std::endl;
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
	//	std::cout << __COUT_HDR__ <<"\tValue written by user:-";
	//	printf("0x%16.16lX",mywriteval);
	//	std::cout << std::endl;
}

void MacroMakerSupervisor::readData(HttpXmlDocument& xmldoc, cgicc::Cgicc& cgi)
{
	std::cout << __COUT_HDR__ << "@@@@@@@ MacroMaker wants to read data @@@@@@@@" << std::endl;
	std::string Address = CgiDataUtilities::getData(cgi, "Address");
	std::string interfaceIndexArray = CgiDataUtilities::getData(cgi, "interfaceIndex");
	std::string supervisorIndexArray = CgiDataUtilities::getData(cgi, "supervisorIndex");

	SOAPParameters parameters; //params for xoap to send
	parameters.addParameter("Request", "UniversalRead");
	parameters.addParameter("Address",Address);

	SOAPParameters retParameters;
    retParameters.addParameter("dataResult");
	std::cout << __COUT_HDR__ <<"Here comes the array from multiselect box for READ, behold: "
			<< supervisorIndexArray << "," << interfaceIndexArray << std::endl;

	SupervisorDescriptors FESupervisors = theSupervisorsConfiguration_.getFEDescriptors();


	unsigned int FEIndex = stoi(supervisorIndexArray);
	unsigned int interfaceIndex = stoi(interfaceIndexArray);


	parameters.addParameter("InterfaceIndex",interfaceIndex);

	std::cout << __COUT_HDR__ <<"The index of the supervisor instance is: " << FEIndex << std::endl;
	std::cout << __COUT_HDR__ <<"...and the index of the interface is: " << interfaceIndex << std::endl;

	SupervisorDescriptors::iterator it = FESupervisors.find(FEIndex);
	if (it == FESupervisors.end())
	{
		std::cout << __COUT_HDR__ << "ERROR!? FE Index doesn't exist" << std::endl;
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
	std::cout << __COUT_HDR__ << "Data reading result received: " << dataReadReturnMsg << std::endl;
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
////	std::cout << __COUT_HDR__ << "Address sending to ZEDRyan: " << addr << std::endl;
////
////	std::string readValue = "";
////	for(unsigned int i=0;i<theFEInterfacesManager_->theFEInterfaces_.size();++i)
////	    ((FEZEDRyanInterface *)(theFEInterfacesManager_->theFEInterfaces_[i]))->interfaceRead(addr, readValue);
////
////	std::cout << __COUT_HDR__ <<"\tReading message:-";
////
////			printf("0x");
////			for(uint32_t i=0; i<readValue.size(); i++)
////				printf("%2.2X",(unsigned char)readValue[i]);
////
////			std::cout << std::dec << std::endl;
////	std::uint64_t myreadval = 0;
////	memcpy(&myreadval,&readValue[2],4);
////
////
////	char toJS[100];
////
////	sprintf(toJS,"0x%16.16lX",myreadval);
////	xmldoc.addTextElementToData("readData",toJS);
////
////	std::cout << __COUT_HDR__ <<"\tReading value from readData:-";
////	printf(toJS,"0x%16.16lX",myreadval);
////	std::cout <<  std::endl;
//
}
//
//
//


