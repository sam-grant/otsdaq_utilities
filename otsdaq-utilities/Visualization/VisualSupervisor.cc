#include "otsdaq-utilities/Visualization/VisualSupervisor.h"
#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/Macros/CoutHeaderMacros.h"
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"
#include "otsdaq-core/SOAPUtilities/SOAPUtilities.h"
#include "otsdaq-core/SOAPUtilities/SOAPParameters.h"
#include "otsdaq-core/SOAPUtilities/SOAPCommand.h"
//#include "otsdaq-core/DataManager/NetworkDataManager.h"
#include "otsdaq-core/ConfigurationInterface/ConfigurationManager.h"
#include "otsdaq-core/ConfigurationPluginDataFormats/XDAQContextConfiguration.h"
//#include "otsdaq-core/RootUtilities/DQMHistos.h"
#include "otsdaq-core/ConfigurationDataFormats/ConfigurationGroupKey.h"
#include "otsdaq-core/DataManager/DataManagerSingleton.h"


//ROOT documentation
//http://root.cern.ch/root/html/index.html

#include "TBufferFile.h"
#include "TObject.h"
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>
#include <TCanvas.h>
#include <TBuffer.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TROOT.h>
#include <TKey.h>
#include <TRegexp.h>
#include <TIterator.h>
#include <TString.h>
#include <TClass.h>
#include <TBufferJSON.h>


#include <dirent.h> /*DIR and dirent*/
#include <sys/stat.h> /*mkdir*/

#include <xdaq/NamespaceURI.h>

#include <iostream>


#define ROOT_BROWSER_PATH			getenv("ROOT_BROWSER_PATH")
#define ROOT_DISPLAY_CONFIG_PATH	getenv("ROOT_DISPLAY_CONFIG_PATH")

#define LIVEDQM_DIR 				std::string("LIVE_DQM")
#define PRE_MADE_ROOT_CFG_DIR 		std::string("Pre-made Views")

#define PRE_MADE_ROOT_CFG_FILE_EXT	std::string(".rcfg")

#define PREFERENCES_PATH 			std::string(getenv("SERVICE_DATA_PATH")) + "/VisualizerData/"
#define PREFERENCES_FILE_EXT		".pref"

#define ROOT_VIEWER_PERMISSIONS_THRESHOLD 	100

using namespace ots;

#undef 	__MF_SUBJECT__
#define __MF_SUBJECT__ "Visualizer"



XDAQ_INSTANTIATOR_IMPL(VisualSupervisor)

//========================================================================================================================
VisualSupervisor::VisualSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception)
: xdaq::Application           (s)
, SOAPMessenger               (this)
, RunControlStateMachine      ("VisualSupervisor")
, theConfigurationManager_    (new ConfigurationManager)//(Singleton<ConfigurationManager>::getInstance()) //I always load the full config but if I want to load a partial configuration (new ConfigurationManager)
, supervisorContextUID_    (theConfigurationManager_->__GET_CONFIG__(XDAQContextConfiguration)->getContextUID(getApplicationContext()->getContextDescriptor()->getURL()))
, supervisorApplicationUID_(theConfigurationManager_->__GET_CONFIG__(XDAQContextConfiguration)->getApplicationUID
		(
				getApplicationContext()->getContextDescriptor()->getURL(),
				getApplicationDescriptor()->getLocalId()
		))
, supervisorConfigurationPath_  ("/" + supervisorContextUID_ + "/LinkToApplicationConfiguration/" + supervisorApplicationUID_ + "/LinkToSupervisorConfiguration")
, theRemoteWebUsers_          (this)
, theDataManager_             (0)
, loadedRunNumber_	          (-1)
{
	INIT_MF("VisualSupervisor");
	__MOUT__ << __PRETTY_FUNCTION__ << std::endl;
	__MOUT__ << __PRETTY_FUNCTION__ << std::endl;
	__MOUT__ << __PRETTY_FUNCTION__ << std::endl;
	__MOUT__ << __PRETTY_FUNCTION__ << std::endl;
	__MOUT__ << __PRETTY_FUNCTION__ << std::endl;
	__MOUT__ << __PRETTY_FUNCTION__ << std::endl;
	__MOUT__ << __PRETTY_FUNCTION__ << std::endl;
	__MOUT__ << __PRETTY_FUNCTION__ << std::endl;
	__MOUT__ << __PRETTY_FUNCTION__ << std::endl;
	__MOUT__ << __PRETTY_FUNCTION__ << std::endl;
	theDataManager_ = DataManagerSingleton::getInstance<VisualDataManager>
	(
			theConfigurationManager_->getNode(theConfigurationManager_->__GET_CONFIG__(XDAQContextConfiguration)->getConfigurationName()),
			supervisorConfigurationPath_,
			supervisorApplicationUID_
	);


	__MOUT__ << __PRETTY_FUNCTION__ << "done data manager" << std::endl;
	xgi::bind(this, &VisualSupervisor::Default, "Default" );
	xgi::bind(this, &VisualSupervisor::request, "request");


	xgi::bind(this, &VisualSupervisor::safari, "safari" );


	//make preferences directory in case they don't exist
	mkdir(((std::string)PREFERENCES_PATH).c_str(), 0755);

	init();
}

//========================================================================================================================
void VisualSupervisor::binaryBufferToHexString(char *buff, unsigned int len,  std::string& dest)
{
	dest = "";
	dest.reserve(len*2);
	char hexstr[3];

	for(unsigned int i=0;i<len;++i)
	{
		sprintf(hexstr,"%02X",(unsigned char)buff[i]);
		dest += hexstr;
	}

}

//========================================================================================================================
VisualSupervisor::~VisualSupervisor(void)
{
	destroy();
}
//========================================================================================================================
void VisualSupervisor::init(void)
{
	//called by constructor
	supervisorDescriptorInfo_.init(getApplicationContext());
}

//========================================================================================================================
void VisualSupervisor::destroy(void)
{
	//called by destructor
	delete theConfigurationManager_;
}

//========================================================================================================================
void VisualSupervisor::Default(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{

	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/Visualization.html?urn=" <<
			this->getApplicationDescriptor()->getLocalId() <<"'></frameset></html>";

}

//========================================================================================================================
void VisualSupervisor::safari(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{

	*out << "<!DOCTYPE HTML><html lang='en'><iframe style='width:100%;height:100%;position:absolute;left:0;top:0;border:0;padding:0;margin:0;' src='/WebPath/html/Visualization.html?urn=" <<
			this->getApplicationDescriptor()->getLocalId() <<"'></iframe></html>";

}


//========================================================================================================================
void VisualSupervisor::request(xgi::Input * in, xgi::Output * out)
throw (xgi::exception::Exception)
{


	cgicc::Cgicc cgi(in);
	std::string Command;
	if((Command = CgiDataUtilities::postData(cgi,"RequestType")) == "")
		Command = cgi("RequestType"); //from GET or POST

	__MOUT__ << "Command " << Command << " files: " << cgi.getFiles().size() << std::endl;

	//Commands
	//getGeometry
	//getEvents
	//getRoot
	//getDirectoryContents
	//setUserPreferences
	//getUserPreferences

	HttpXmlDocument xmldoc;
	uint8_t userPermissions;
	std::string userName;

	//**** start LOGIN GATEWAY CODE ***//
	//check cookieCode, sequence, userWithLock, and permissions access all in one shot!
	{
		bool automaticCommand = Command == "getRoot" || Command == "getEvents"; //automatic commands should not refresh cookie code.. only user initiated commands should!
		bool checkLock = true;
		bool needUserName = Command == "setUserPreferences" || Command == "getUserPreferences";
		bool requireLock = false;

		if(!theRemoteWebUsers_.xmlLoginGateway(
				cgi,
				out,
				&xmldoc,
				supervisorDescriptorInfo_,
				&userPermissions,  			//acquire user's access level (optionally null pointer)
				!automaticCommand,			//true/false refresh cookie code
				1, 							//set access level requirement to pass gateway
				checkLock,					//true/false enable check that system is unlocked or this user has the lock
				requireLock,				//true/false requires this user has the lock to proceed
				0,//&userWithLock,			//acquire username with lock (optionally null pointer)
				(needUserName?&userName:0)	//acquire username of this user (optionally null pointer)
				,0//&displayName			//acquire user's Display Name
				,0//&activeSessionIndex		//acquire user's session index associated with the cookieCode
		))
		{	//failure
			//std::cout << out->str() << std::endl; //could print out return string on failure
			return;
		}
	}
	//done checking cookieCode, sequence, userWithLock, and permissions access all in one shot!
	//**** end LOGIN GATEWAY CODE ***//



	//    if (Command == "getHisto" && theDataManager_->getLiveDQMHistos() != 0)
	//    {
	//       // TH1I* histo = (TH1I*)theDataManager_->getLiveDQMHistos()->get("Planes/Plane_0_Occupancy");
	//
	//        //        theDataManager_->load("Run185_Histo.root","Histograms");
	//        //TH1F*     histo1d  = theDataManager_->getFileDQMHistos().getHisto1D();
	//        //TCanvas*  canvas   = theDataManager_->getFileDQMHistos().getCanvas ();
	//        //TH2F*     histo2d  = theDataManager_->getFileDQMHistos().getHisto2D();
	//        //TProfile* profile  = theDataManager_->getFileDQMHistos().getProfile();
	//
	//    }
	if (Command == "setUserPreferences")
	{
		__MOUT__ << "userName: " << userName << std::endl;
		std::string fullPath = (std::string)PREFERENCES_PATH + userName + PREFERENCES_FILE_EXT;
		__MOUT__ << "fullPath: " << fullPath << std::endl;

		std::string radioSelect = CgiDataUtilities::getData(cgi,"radioSelect");
		std::string autoRefresh = CgiDataUtilities::getData(cgi,"autoRefresh");
		std::string autoHide = CgiDataUtilities::getData(cgi,"autoHide");
		std::string hardRefresh = CgiDataUtilities::getData(cgi,"hardRefresh");
		std::string autoRefreshPeriod = CgiDataUtilities::getData(cgi,"autoRefreshPeriod");

		__MOUT__ << "radioSelect: " << radioSelect 	<< std::endl;
		__MOUT__ << "autoRefresh: " << autoRefresh 	<< std::endl;
		__MOUT__ << "autoHide: " 	<< autoHide 	<< std::endl;
		__MOUT__ << "hardRefresh: " << hardRefresh  << std::endl;
		__MOUT__ << "autoRefreshPeriod: " << autoRefreshPeriod  << std::endl;


		//read existing
		FILE *fp = fopen(fullPath.c_str(),"r");
		if(fp)
		{
			char line[100];
			char val[100];

			fgets(line,100,fp);
			sscanf(line,"%*s %s",val);
			if(radioSelect == "")
				radioSelect = val;

			fgets(line,100,fp);
			sscanf(line,"%*s %s",val);
			if(autoRefresh == "")
				autoRefresh = val;

			fgets(line,100,fp);
			sscanf(line,"%*s %s",val);
			if(autoHide == "")
				autoHide = val;

			fgets(line,100,fp);
			sscanf(line,"%*s %s",val);
			if(hardRefresh == "")
				hardRefresh = val;

			fgets(line,100,fp);
			sscanf(line,"%*s %s",val);
			if(autoRefreshPeriod == "")
				autoRefreshPeriod = val;

			fclose(fp);
		}

		//write new
		fp = fopen(fullPath.c_str(),"w");
		if(fp)
		{
			fprintf(fp,"radioSelect %s\n",radioSelect.c_str());
			fprintf(fp,"autoRefresh %s\n",autoRefresh.c_str());
			fprintf(fp,"autoHide %s\n",autoHide.c_str());
			fprintf(fp,"hardRefresh %s\n",hardRefresh.c_str());
			fprintf(fp,"autoRefreshPeriod %s\n",autoRefreshPeriod.c_str());
			fclose(fp);
		}
		else
			__MOUT_ERR__ << "Failure writing preferences to file: " << fullPath << std::endl;
	}
	else if (Command == "getUserPreferences")
	{
		__MOUT__ << "userName: " << userName << std::endl;
		std::string fullPath = (std::string)PREFERENCES_PATH + userName + PREFERENCES_FILE_EXT;
		__MOUT__ << "fullPath: " << fullPath << std::endl;

		FILE *fp = fopen(fullPath.c_str(),"r");
		if(fp)
		{
			char line[100];
			//char val[100];
			int val;

			fgets(line,100,fp);
			sscanf(line,"%*s %d",&val);
			if(val < 0 || val > 3) val = 0; //FIXME.. value can get corrupt...
			xmldoc.addTextElementToData("radioSelect", std::to_string(val));
			fgets(line,100,fp);
			sscanf(line,"%*s %d",&val);
			xmldoc.addTextElementToData("autoRefresh", std::to_string(val));
			fgets(line,100,fp);
			sscanf(line,"%*s %d",&val);
			xmldoc.addTextElementToData("autoHide", std::to_string(val));
			fgets(line,100,fp);
			sscanf(line,"%*s %d",&val);
			xmldoc.addTextElementToData("hardRefresh", std::to_string(val));
			fgets(line,100,fp);
			sscanf(line,"%*s %d",&val);
			xmldoc.addTextElementToData("autoRefreshPeriod", std::to_string(val));
			fclose(fp);
		}
		else
		{
			//else assume user has no preferences yet
			xmldoc.addTextElementToData("radioSelect", "");
			xmldoc.addTextElementToData("autoRefresh", "");
			xmldoc.addTextElementToData("autoHide", "");
			xmldoc.addTextElementToData("hardRefresh", "");
			xmldoc.addTextElementToData("autoRefreshPeriod", "");
		}
	}
	else if (Command == "getDirectoryContents")
	{
		//return directory structure for requested path, types are "dir" and "file"

		std::string rootpath = std::string(ROOT_BROWSER_PATH) + "/";
		std::string path  = CgiDataUtilities::postData(cgi,"Path");
		__MOUT__ << path << std::endl;

		char permStr[10];
		sprintf(permStr,"%d",userPermissions);
		xmldoc.addTextElementToData("permissions", permStr); //add permissions

		std::string dirpath = rootpath + path;
		if(path == "/" + PRE_MADE_ROOT_CFG_DIR + "/")
			dirpath = ROOT_DISPLAY_CONFIG_PATH;

		if(path.find("/" + PRE_MADE_ROOT_CFG_DIR + "/") == 0) //ROOT config path must start the path
			dirpath = std::string(ROOT_DISPLAY_CONFIG_PATH) + "/" + path.substr(PRE_MADE_ROOT_CFG_DIR.length()+2);

		DIR *pDIR;
		struct dirent *entry;
		bool isNotRtCfg;
		if( (pDIR=opendir(dirpath.c_str())) )
		{
			xmldoc.addTextElementToData("path", path);

			//add LIVE if path is / and DQM is active
			//add Pre-made Views if path is / and ROOT_DISPLAY_CONFIG_PATH isnt already there
			if(path == "/")
			{

				if(theDataManager_->getLiveDQMHistos() != 0)
					xmldoc.addTextElementToData("dir", LIVEDQM_DIR + ".root"); //add to xml

				//check for ROOT_DISPLAY_CONFIG_PATH
				DIR *pRtDIR = opendir(ROOT_DISPLAY_CONFIG_PATH);
				bool recheck = false;
				if(!pRtDIR) //if doesn't exist, make it
				{
					recheck = true;
					if(mkdir(ROOT_DISPLAY_CONFIG_PATH, S_IRWXU | (S_IRGRP | S_IXGRP) | (S_IROTH | S_IXOTH))) //mode = drwx r-x r-x
						__MOUT__ << "Failed to make directory for pre made views: " << ROOT_DISPLAY_CONFIG_PATH << std::endl;
				}
				else
					closedir(pRtDIR); //else close and display

				if(!recheck || (pRtDIR = opendir(ROOT_DISPLAY_CONFIG_PATH)))
				{
					xmldoc.addTextElementToData("dir", PRE_MADE_ROOT_CFG_DIR); //add to xml
					if(recheck)
						closedir(pRtDIR);
				}
			}

			while((entry = readdir(pDIR)))
			{
				if( entry->d_name[0] != '.' && (entry->d_type == 4 || entry->d_type == 8))
				{
					//__MOUT__ << int(entry->d_type) << " " << entry->d_name << "\n" << std::endl;
					isNotRtCfg = std::string(entry->d_name).find(".rcfg") == std::string::npos;
					if(entry->d_type == 8 && std::string(entry->d_name).find(".root") == std::string::npos
							&& isNotRtCfg)
						continue;
					xmldoc.addTextElementToData(entry->d_type==4?"dir":(isNotRtCfg?"dir":"file"), entry->d_name);
				}
			}
			closedir(pDIR);
		}
		else
			__MOUT__ << "Failed to access directory contents!" << std::endl;
	}
	else if (Command == "getRoot")
	{
		//return directory structure for requested ROOT path, types are "dir" and "file"

		std::string path = CgiDataUtilities::postData(cgi,"RootPath");
		std::string fullPath  = std::string(getenv("ROOT_BROWSER_PATH")) + path;

		__MOUT__ << "Full path:-" << fullPath << "-" << std::endl;

		std::string rootFileName      = fullPath.substr(0,fullPath.find(".root")+5);
		std::string rootDirectoryName = rootFileName + ":" + fullPath.substr(fullPath.find(".root")+5,fullPath.size()-fullPath.find(".root")+5+1);

		std::string::size_type LDQM_pos = path.find("/" + LIVEDQM_DIR + ".root/");
		TFile* rootFile;

		if(theDataManager_->getLiveDQMHistos() != 0 && LDQM_pos == 0)
		{
			__MOUT__ << "Attempting to get LIVE file." << std::endl;
			rootFile = theDataManager_->getLiveDQMHistos()->getFile();
			if(!rootFile)
				__MOUT__ << "File was closed." << std::endl;
			else
			{
				__MOUT__ << "LIVE file name: " << rootFile->GetName() << std::endl;
				rootDirectoryName = path.substr(("/" + LIVEDQM_DIR + ".root").length());
			}
		}
		else
			rootFile = TFile::Open(rootFileName.c_str());

		__MOUT__ << "FileName : " << rootFileName << " Object: " << rootDirectoryName << std::endl;

		if(!rootFile || !rootFile->IsOpen())
		{
			__MOUT__ << "Failed to access root file: " << rootFileName << std::endl;
		}
		else
		{
			xmldoc.addTextElementToData("path", path);

			TDirectory* directory;
			if((directory = rootFile->GetDirectory(rootDirectoryName.c_str())) == 0)
			{
				__MOUT__ << "This is not a directory!" << std::endl;
				directory = rootFile;

				//failed directory so assume it's file
				__MOUT__ << "Getting object name: " << rootDirectoryName << std::endl;
				TObject* histo = (TObject*)rootFile->Get(rootDirectoryName.c_str());

				if(!histo)
					__MOUT__ << "Failed to access:-" << rootDirectoryName << "-" << std::endl;
				else //turns out was a root object path
				{
					__MOUT__ << "Converting histo to json: " << histo->GetName() << std::endl;
					TString json = TBufferJSON::ConvertToJSON(histo);
					//__MOUT__ << "json " << json << std::endl;

					TBufferFile tbuff(TBuffer::kWrite);

					std::string rootType = histo->ClassName();
					__MOUT__ << "rootType " << rootType << std::endl;

					histo->Streamer(tbuff);

					//__MOUT__ << "histo length " << tbuff.Length() << std::endl;

					std::string dest;
					binaryBufferToHexString(tbuff.Buffer(), tbuff.Length(), dest);

					//*out << json.Data() << std::endl;
					//return;
					xmldoc.addTextElementToData("rootType", rootType);
					xmldoc.addTextElementToData("rootData", dest);
					xmldoc.addTextElementToData("rootJSON", json.Data());
					__MOUT__ << "Done" << std::endl;

					//std::cout << "\n\n" << json.Data() << "\n\n";
				}
			}
			else
			{
				__MOUT__ << "directory found getting the content!" << std::endl;
				TRegexp re("*", kTRUE);
				if (LDQM_pos == 0)
				{
					TObject *obj;
					TIter nextobj(directory->GetList());
					while ((obj = (TObject *) nextobj()))
					{
						TString s = obj->GetName();
						if (s.Index(re) == kNPOS)
							continue;
						__MOUT__ << __PRETTY_FUNCTION__ << "Class Name: " << obj->IsA()->GetName() << std::endl;
						xmldoc.addTextElementToData((std::string(obj->IsA()->GetName()).find("Directory") != std::string::npos)?"dir":"file", obj->GetName());
					}
				}
				else
				{

					TKey *key;
					TIter next(directory->GetListOfKeys());
					while ((key = (TKey *) next()))
					{
						TString s = key->GetName();
						if (s.Index(re) == kNPOS)
							continue;
						__MOUT__ << __PRETTY_FUNCTION__ << "Class Name: " << key->GetClassName() << std::endl;
						xmldoc.addTextElementToData((std::string(key->GetClassName()).find("Directory") != std::string::npos)?"dir":"file", key->GetName());
					}
				}
			}
			if(LDQM_pos == std::string::npos)
				rootFile->Close();
		}
	}
	else if (Command == "getEvents")
	{
		int Run = atoi(cgi("run").c_str());

		__MOUT__ << "getEvents for run " << Run << std::endl;

		if(Run != (int)loadedRunNumber_ || loadedRunNumber_ == (unsigned int)-1)
		{
			theDataManager_->load("Run1684.root","Monicelli");
			loadedRunNumber_ = Run;
		}

		DOMElement* eventsParent = xmldoc.addTextElementToData("events", "");
		DOMElement* eventParent;
		char str[40];

		//		const Visual3DEvents& events = theDataManager_->getVisual3DEvents();
		//		__MOUT__ << "Preparing hits xml" << std::endl;
		//		int numberOfEvents = 0;
		//		for(Visual3DEvents::const_iterator it=events.begin(); it!=events.end() && numberOfEvents < 10000; it++, numberOfEvents++)
		//		{
		//			//__MOUT__ << "Event: " << numberOfEvents << std::endl;
		//			eventParent = xmldoc.addTextElementToParent("event", str, eventsParent);
		//			const VisualHits& hits = it->getHits();
		//			for(VisualHits::const_iterator itHits=hits.begin(); itHits!=hits.end(); itHits++)
		//			{
		//				sprintf(str,"%f",itHits->x);
		//				xmldoc.addTextElementToParent("xyz_point", str, eventParent);
		//				sprintf(str,"%f",itHits->y);
		//				xmldoc.addTextElementToParent("xyz_point", str, eventParent);
		//				sprintf(str,"%f",itHits->z);
		//				xmldoc.addTextElementToParent("xyz_point", str, eventParent);
		//				//__MOUT__ << "X: " << itHits->x << " Y: " << itHits->y << " Z: " << itHits->z << std::endl;
		//			}
		//			const VisualTracks& tracks = it->getTracks();
		//			for(VisualTracks::const_iterator itTrks=tracks.begin(); itTrks!=tracks.end(); itTrks++)
		//			{
		//				sprintf(str,"%f",itTrks->slopeX);
		//				xmldoc.addTextElementToParent("slope", str, eventParent);
		//				sprintf(str,"%f",itTrks->slopeY);
		//				xmldoc.addTextElementToParent("slope", str, eventParent);
		//				sprintf(str,"%f",itTrks->interceptX);
		//				xmldoc.addTextElementToParent("intcpt", str, eventParent);
		//				sprintf(str,"%f",itTrks->interceptY);
		//				xmldoc.addTextElementToParent("intcpt", str, eventParent);
		//
		//			}
		//
		//		}
		__MOUT__ << "Done hits xml" << std::endl;
	}
	else if (Command == "getGeometry")
	{
		__MOUT__ << "getGeometry" << std::endl;

		//FIXME -- this crashes when the file doesn't exist!
		theDataManager_->load("Run1684.geo","Geometry");

		__MOUT__ << "getGeometry" << std::endl;

		DOMElement* geometryParent = xmldoc.addTextElementToData("geometry", "");
		//		const Visual3DShapes& shapes = theDataManager_->getVisual3DGeometry().getShapes();
		//
		//		__MOUT__ << "getGeometry" << std::endl;
		//
		//
		//		DOMElement* objectParent;
		//		char str[40];
		//		for(Visual3DShapes::const_iterator itShapes=shapes.begin(); itShapes!=shapes.end(); itShapes++)
		//		{
		//			objectParent = xmldoc.addTextElementToParent("object", str, geometryParent);
		//			xmldoc.addTextElementToParent("object_type", "LINE_LOOP", objectParent);
		//			sprintf(str,"%d",itShapes->numberOfRows);
		//			xmldoc.addTextElementToParent("object_rows", str, objectParent);
		//			sprintf(str,"%d",itShapes->numberOfColumns);
		//			xmldoc.addTextElementToParent("object_columns", str, objectParent);
		//			for(Points::const_iterator itCorners=itShapes->corners.begin(); itCorners!=itShapes->corners.end(); itCorners++)
		//			{
		//				sprintf(str,"%f",itCorners->x);
		//				xmldoc.addTextElementToParent("xyz_point", str, objectParent);
		//				sprintf(str,"%f",itCorners->y);
		//				xmldoc.addTextElementToParent("xyz_point", str, objectParent);
		//				sprintf(str,"%f",itCorners->z);
		//				xmldoc.addTextElementToParent("xyz_point", str, objectParent);
		//			}
		//		}
	}
	else if (Command == "getRootConfig")
	{
		std::string path =  CgiDataUtilities::postData(cgi,"RootConfigPath");
		__MOUT__ << "path " << path << std::endl;

		if(path.find("/" + PRE_MADE_ROOT_CFG_DIR + "/") == 0) //ROOT config path must start the path
		{
			path = std::string(ROOT_DISPLAY_CONFIG_PATH) + "/" + path.substr(PRE_MADE_ROOT_CFG_DIR.length()+2);
			__MOUT__ << "mod path " << path << std::endl;
		}

		HttpXmlDocument cfgXml;
		if(cfgXml.loadXmlDocument(path))
		{
			xmldoc.addTextElementToData("status", "1");
			xmldoc.copyDataChildren(cfgXml); //copy file to output xml
			cfgXml.saveXmlDocument(path);
		}
		else
			xmldoc.addTextElementToData("status", "Failed. File to properly load config file.");
	}
	else if (Command == "rootAdminControls")
	{
		if(userPermissions < ROOT_VIEWER_PERMISSIONS_THRESHOLD)
		{
			__MOUT__ << "Insufficient permissions for Root Viewer Admin Controls: " << userPermissions << " < " << ROOT_VIEWER_PERMISSIONS_THRESHOLD << std::endl;
			xmldoc.addTextElementToData("status", "Failed. Insufficient user permissions.");
		}
		else
		{
			std::string cmd = cgi("cmd"); //possible commands are
			//	mkdir
			//	save
			// 	delete

			std::string path =  CgiDataUtilities::postData(cgi,"path");
			std::string name =  CgiDataUtilities::postData(cgi,"name");
			__MOUT__ << "cmd " << cmd << std::endl;
			__MOUT__ << "path " << path << std::endl;
			__MOUT__ << "name " << name << std::endl;

			if(path.find("/" + PRE_MADE_ROOT_CFG_DIR + "/") == 0) //ROOT config path must start the path
			{
				path = std::string(ROOT_DISPLAY_CONFIG_PATH) + "/" + path.substr(PRE_MADE_ROOT_CFG_DIR.length()+2) + name;
				__MOUT__ << "mod path " << path << std::endl;


				if(cmd == "mkdir")
				{
					if(mkdir(path.c_str(), S_IRWXU | (S_IRGRP | S_IXGRP) | (S_IROTH | S_IXOTH))) //mode = drwx r-x r-x
						xmldoc.addTextElementToData("status", "Failed. Directory create rejected.");
					else
						xmldoc.addTextElementToData("status", "1"); //success
				}
				else if(cmd == "save")
				{
					path += PRE_MADE_ROOT_CFG_FILE_EXT; //add file extension

					bool useRunWildCard =  atoi(CgiDataUtilities::postData(cgi,"useRunWildCard").c_str()); //0 or 1
					std::string config =  CgiDataUtilities::postData(cgi,"config");
					__MOUT__ << "config " << config << std::endl;
					__MOUT__ << "useRunWildCard " << useRunWildCard << std::endl;

					//check if file already exists
					FILE *fp = fopen(path.c_str(),"r");
					if(fp)
					{
						fclose(fp);
						xmldoc.addTextElementToData("status", "Failed. File already exists.");
						__MOUT__ << " Failed. File already exists." << std::endl;
					}
					else
					{
						//dump to file
						//verify proper format through read back
						//if successfully loaded, re-save for formatting

						//dump to file
						fp = fopen(path.c_str(),"w");
						fputs(config.c_str(),fp);
						fclose(fp);

						//verify proper format through read back
						HttpXmlDocument cfgXml;
						if(cfgXml.loadXmlDocument(path))
						{
							//successfully loaded, re-save for formatting
							cfgXml.saveXmlDocument(path);
							xmldoc.addTextElementToData("status", "1"); //success
						}
						else //failed to load properly
						{
							xmldoc.addTextElementToData("status", "Failed. Fatal. Improper file format.");
							if(remove(path.c_str()) != 0)
								__MOUT__ << "Failed. Could not remove poorly formed Root config file!" << std::endl;
						}
					}

				}
				else if(cmd == "delete")
				{
					//guess first directory and then file
					if (rmdir(path.c_str()) == 0 || remove((path+PRE_MADE_ROOT_CFG_FILE_EXT).c_str()) == 0 )
						xmldoc.addTextElementToData("status", "1"); //success
					else
						xmldoc.addTextElementToData("status", "Failed. Target could not be deleted.");
				}
				else
					xmldoc.addTextElementToData("status", "Failed. Unrecognized command.");
			}
			else
				xmldoc.addTextElementToData("status", "Failed. Invalid path.");

		}

	}
	else
		__MOUT__ << "Command request, " << Command << ", not recognized." << std::endl;
	//return xml doc holding server response
	xmldoc.outputXmlDocument((std::ostringstream*) out, false);
}


//========================================================================================================================
void VisualSupervisor::stateRunning(toolbox::fsm::FiniteStateMachine& fsm)
throw (toolbox::fsm::exception::Exception)
{


}

//========================================================================================================================
void VisualSupervisor::transitionConfiguring(toolbox::Event::Reference e)
throw (toolbox::fsm::exception::Exception)
{

//	theConfigurationGroupKey_ = theConfigurationManager_->makeTheConfigurationGroupKey(atoi(SOAPUtilities::translate(theStateMachine_.getCurrentMessage()).getParameters().getValue("ConfigurationGroupKey").c_str()));
//	theConfigurationManager_->activateConfigurationGroupKey(theConfigurationGroupKey_,0);
//
//	std::pair<std::string /*group name*/, ConfigurationGroupKey> theGroup(
//			SOAPUtilities::translate(theStateMachine_.getCurrentMessage()).
//									getParameters().getValue("ConfigurationGroupName"),
//						ConfigurationGroupKey(SOAPUtilities::translate(theStateMachine_.getCurrentMessage()).
//						getParameters().getValue("ConfigurationGroupKey")));
//
//	__MOUT__ << "Configuration group name: " << theGroup.first << " key: " <<
//			theGroup.second << std::endl;
//
//	theConfigurationManager_->loadConfigurationGroup(
//			theGroup.first,
//			theGroup.second, true);
//
//	//theDataManager_->configure();
}

//========================================================================================================================
void VisualSupervisor::transitionHalting(toolbox::Event::Reference e)
throw (toolbox::fsm::exception::Exception)
{

	//theDataManager_->halt();
}

//========================================================================================================================
void VisualSupervisor::transitionStarting(toolbox::Event::Reference e)
throw (toolbox::fsm::exception::Exception)
{

	//theDataManager_->start(SOAPUtilities::translate(theStateMachine_.getCurrentMessage()).getParameters().getValue("RunNumber"));
}

//========================================================================================================================
void VisualSupervisor::transitionStopping(toolbox::Event::Reference e)
throw (toolbox::fsm::exception::Exception)
{

	//theDataManager_->stop();
}












