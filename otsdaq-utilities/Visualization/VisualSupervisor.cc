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
#include "otsdaq-core/RootUtilities/DQMHistos.h"
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

#define PREFERENCES_PATH 			std::string(getenv("SERVICE_DATA")) + "/VisualizerData/"
#define PREFERENCES_FILE_EXT		".pref"

#define ROOT_VIEWER_PERMISSIONS_THRESHOLD 	100

using namespace ots;

#undef 	__MF_SUBJECT__
#define __MF_SUBJECT__ "Visualizer"



XDAQ_INSTANTIATOR_IMPL(VisualSupervisor)

//========================================================================================================================
VisualSupervisor::VisualSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception)
: xdaq::Application        (s)
, SOAPMessenger            (this)
, RunControlStateMachine   ("VisualSupervisor")
, supervisorType_          ("Visual")
, supervisorInstance_      (this->getApplicationDescriptor()->getInstance())
, theRemoteWebUsers_       (this)
, theConfigurationManager_ (new ConfigurationManager)//(Singleton<ConfigurationManager>::getInstance()) //I always load the full config but if I want to load a partial configuration (new ConfigurationManager)
, theDataManager_          (0)
, loadedRunNumber_	       (-1)
{
	INIT_MF("VisualSupervisor");
<<<<<<< Updated upstream
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
=======
	std::cout << __COUT_HDR_FL__ << __PRETTY_FUNCTION__ << std::endl;
	std::cout << __COUT_HDR_FL__ << __PRETTY_FUNCTION__ << std::endl;
	std::cout << __COUT_HDR_FL__ << __PRETTY_FUNCTION__ << std::endl;
	std::cout << __COUT_HDR_FL__ << __PRETTY_FUNCTION__ << std::endl;
	std::cout << __COUT_HDR_FL__ << __PRETTY_FUNCTION__ << std::endl;
	std::cout << __COUT_HDR_FL__ << __PRETTY_FUNCTION__ << std::endl;
	std::cout << __COUT_HDR_FL__ << __PRETTY_FUNCTION__ << std::endl;
	std::cout << __COUT_HDR_FL__ << __PRETTY_FUNCTION__ << std::endl;
	std::cout << __COUT_HDR_FL__ << __PRETTY_FUNCTION__ << std::endl;
	std::cout << __COUT_HDR_FL__ << __PRETTY_FUNCTION__ << std::endl;
>>>>>>> Stashed changes
	theDataManager_ = DataManagerSingleton::getInstance<VisualDataManager>(
			supervisorType_,
			supervisorInstance_,
			theConfigurationManager_);


<<<<<<< Updated upstream
	__MOUT__ << __PRETTY_FUNCTION__ << "done data manager" << std::endl;
	xgi::bind(this, &VisualSupervisor::Default, "Default" );
	xgi::bind(this, &VisualSupervisor::request, "request");

=======
	std::cout << __COUT_HDR_FL__ << __PRETTY_FUNCTION__ << "done data manager" << std::endl;
	xgi::bind(this, &VisualSupervisor::Default, "Default" );
	xgi::bind(this, &VisualSupervisor::request, "request");
>>>>>>> Stashed changes

	xgi::bind(this, &VisualSupervisor::safari, "safari" );

<<<<<<< Updated upstream

	//make preferences directory in case they don't exist
	mkdir(((std::string)PREFERENCES_PATH).c_str(), 0755);
=======
	xgi::bind(this, &VisualSupervisor::safari, "safari" );

	init();
>>>>>>> Stashed changes

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
	theSupervisorsConfiguration_.init(getApplicationContext());
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
			getenv("VISUAL_SUPERVISOR_ID") <<"'></frameset></html>";

}

//========================================================================================================================
void VisualSupervisor::safari(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{

	*out << "<!DOCTYPE HTML><html lang='en'><iframe style='width:100%;height:100%;position:absolute;left:0;top:0;border:0;padding:0;margin:0;' src='/WebPath/html/Visualization.html?urn=" <<
			getenv("VISUAL_SUPERVISOR_ID") <<"'></iframe></html>";

}


//========================================================================================================================
void VisualSupervisor::request(xgi::Input * in, xgi::Output * out)
throw (xgi::exception::Exception)
{
<<<<<<< Updated upstream


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

=======


	cgicc::Cgicc cgi(in);
	std::string Command;
	if((Command = CgiDataUtilities::postData(cgi,"RequestType")) == "")
		Command = cgi("RequestType"); //from GET or POST

	//std::cout << __COUT_HDR_FL__ << "Command " << Command << " files: " << cgi.getFiles().size() << std::endl;

	//Commands
	//getGeometry
	//getEvents
	//getRoot
	//getDirectoryContents

	HttpXmlDocument xmldoc;
	uint8_t userPermissions;

	//**** start LOGIN GATEWAY CODE ***//
	//check cookieCode, sequence, userWithLock, and permissions access all in one shot!
	{
		bool automaticCommand = Command == "getRoot" || Command == "getEvents"; //automatic commands should not refresh cookie code.. only user initiated commands should!
		bool checkLock = true;

>>>>>>> Stashed changes
		if(!theRemoteWebUsers_.xmlLoginGateway(
				cgi,out,&xmldoc,theSupervisorsConfiguration_,
				&userPermissions,  			//acquire user's access level (optionally null pointer)
				!automaticCommand,			//true/false refresh cookie code
				1, 							//set access level requirement to pass gateway
				checkLock,					//true/false enable check that system is unlocked or this user has the lock
<<<<<<< Updated upstream
				requireLock,				//true/false requires this user has the lock to proceed
				0,//&userWithLock,			//acquire username with lock (optionally null pointer)
				(needUserName?&userName:0)	//acquire username of this user (optionally null pointer)
				,0//&displayName			//acquire user's Display Name
				,0//&activeSessionIndex		//acquire user's session index associated with the cookieCode
=======
				0,//&userWithLock,				//acquire username with lock (optionally null pointer)
				0,//&userName					//acquire username of this user (optionally null pointer)
				0,//&displayName			//acquire user's Display Name
				0//&activeSessionIndex		//acquire user's session index associated with the cookieCode
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
	if (Command == "setUserPreferences")
	{
		__MOUT__ << "userName: " << userName << std::endl;
		std::string fullPath = (std::string)PREFERENCES_PATH + userName + PREFERENCES_FILE_EXT;
		__MOUT__ << "fullPath: " << fullPath << std::endl;

		std::string radioSelect = CgiDataUtilities::getData(cgi,"radioSelect");
		std::string autoRefresh = CgiDataUtilities::getData(cgi,"autoRefresh");
		std::string autoHide = CgiDataUtilities::getData(cgi,"autoHide");
		__MOUT__ << "radioSelect: " << radioSelect 	<< std::endl;
		__MOUT__ << "autoRefresh: " << autoRefresh 	<< std::endl;
		__MOUT__ << "autoHide: " 	<< autoHide 	<< std::endl;

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

			fclose(fp);
		}

		//write new
		fp = fopen(fullPath.c_str(),"w");
		if(fp)
		{
			fprintf(fp,"radioSelect %s\n",radioSelect.c_str());
			fprintf(fp,"autoRefresh %s\n",autoRefresh.c_str());
			fprintf(fp,"autoHide %s\n",autoHide.c_str());
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
			char val[100];

			fgets(line,100,fp);
			sscanf(line,"%*s %s",val);
			xmldoc.addTextElementToData("radioSelect", val);
			fgets(line,100,fp);
			sscanf(line,"%*s %s",val);
			xmldoc.addTextElementToData("autoRefresh", val);
			fgets(line,100,fp);
			sscanf(line,"%*s %s",val);
			xmldoc.addTextElementToData("autoHide", val);
			fclose(fp);
		}
		else
		{
			//else assume user has no preferences yet
			xmldoc.addTextElementToData("radioSelect", "");
			xmldoc.addTextElementToData("autoRefresh", "");
			xmldoc.addTextElementToData("autoHide", "");
		}
	}
	else if (Command == "getDirectoryContents")
=======
	if (Command == "getDirectoryContents")
>>>>>>> Stashed changes
	{
		//return directory structure for requested path, types are "dir" and "file"

		std::string rootpath = std::string(ROOT_BROWSER_PATH) + "/";
		std::string path  = CgiDataUtilities::postData(cgi,"Path");
<<<<<<< Updated upstream
		__MOUT__ << path << std::endl;
=======
		std::cout << __COUT_HDR_FL__ << path << std::endl;
>>>>>>> Stashed changes

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
<<<<<<< Updated upstream
						__MOUT__ << "Failed to make directory for pre made views: " << ROOT_DISPLAY_CONFIG_PATH << std::endl;
=======
						std::cout << __COUT_HDR_FL__ << "Failed to make directory for pre made views: " << ROOT_DISPLAY_CONFIG_PATH << std::endl;
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
					//__MOUT__ << int(entry->d_type) << " " << entry->d_name << "\n" << std::endl;
=======
					//std::cout << __COUT_HDR_FL__ << int(entry->d_type) << " " << entry->d_name << "\n" << std::endl;
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
			__MOUT__ << "Failed to access directory contents!" << std::endl;
=======
			std::cout << __COUT_HDR_FL__ << "Failed to access directory contents!" << std::endl;
>>>>>>> Stashed changes
	}
	else if (Command == "getRoot")
	{
		//return directory structure for requested ROOT path, types are "dir" and "file"

		std::string path = CgiDataUtilities::postData(cgi,"RootPath");
		std::string fullPath  = std::string(getenv("ROOT_BROWSER_PATH")) + path;

<<<<<<< Updated upstream
		__MOUT__ << "Full path:-" << fullPath << "-" << std::endl;
=======
		std::cout << __COUT_HDR_FL__ << "Full path:-" << fullPath << "-" << std::endl;
>>>>>>> Stashed changes

		std::string rootFileName      = fullPath.substr(0,fullPath.find(".root")+5);
		std::string rootDirectoryName = rootFileName + ":" + fullPath.substr(fullPath.find(".root")+5,fullPath.size()-fullPath.find(".root")+5+1);

		std::string::size_type LDQM_pos = path.find("/" + LIVEDQM_DIR + ".root/");
		TFile* rootFile;

<<<<<<< Updated upstream
		if(theDataManager_->getLiveDQMHistos() != 0 && LDQM_pos == 0)
		{
			__MOUT__ << "Attempting to get LIVE file." << std::endl;
			rootFile = theDataManager_->getLiveDQMHistos()->getFile();
			__MOUT__ << "LIVE file name: " << rootFile->GetName() << std::endl;
=======
		std::cout << __COUT_HDR_FL__ << "getting Live DQM Histos $$$$$$$$$$$$$$$$$$$$$$$$$$" << theDataManager_->getLiveDQMHistos() << std::endl;

		if(theDataManager_->getLiveDQMHistos() != 0 && LDQM_pos == 0)
		{
			std::cout << __COUT_HDR_FL__ << "Attempting to get LIVE file." << std::endl;
			rootFile = theDataManager_->getLiveDQMHistos()->getFile();

			if(!rootFile)
			{
				std::cout << __COUT_HDR_FL__ << "Invalid LIVE rootFile: " << rootFile << std::endl; //0, if stop before pause refresh
				goto CLEANUP;
			}
			std::cout << __COUT_HDR_FL__ << "rootFile: " << rootFile << std::endl; //0, if stop before pause refresh

			std::cout << __COUT_HDR_FL__ << "LIVE file name: " << rootFile->GetName() << std::endl;
>>>>>>> Stashed changes
			rootDirectoryName = path.substr(("/" + LIVEDQM_DIR + ".root").length());
		}
		else
			rootFile = TFile::Open(rootFileName.c_str());

<<<<<<< Updated upstream
		__MOUT__ << "FileName : " << rootFileName << " Object: " << rootDirectoryName << std::endl;

		if(!rootFile || !rootFile->IsOpen())
		{
			__MOUT__ << "Failed to access root file: " << rootFileName << std::endl;
=======

		std::cout << __COUT_HDR_FL__ << "FileName : " << rootFileName << " Object: " << rootDirectoryName << std::endl;

		if(!rootFile || !rootFile->IsOpen())
		{
			std::cout << __COUT_HDR_FL__ << "Failed to access root file: " << rootFileName << std::endl;
>>>>>>> Stashed changes
		}
		else
		{
			xmldoc.addTextElementToData("path", path);

			TDirectory* directory;
			if((directory = rootFile->GetDirectory(rootDirectoryName.c_str())) == 0)
			{
<<<<<<< Updated upstream
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
=======
				std::cout << __COUT_HDR_FL__ << "This is not a directory!" << std::endl;
				directory = rootFile;

				//failed directory so assume it's file
				std::cout << __COUT_HDR_FL__ << "Getting object name: " << rootDirectoryName << std::endl;
				TObject* histo = (TObject*)rootFile->Get(rootDirectoryName.c_str());

				if(!histo)
					std::cout << __COUT_HDR_FL__ << "Failed to access:-" << rootDirectoryName << "-" << std::endl;
				else //turns out was a root object path
				{
					std::cout << __COUT_HDR_FL__ << "Converting histo to json: " << histo->GetName() << std::endl;
					TString json = TBufferJSON::ConvertToJSON(histo);
					//std::cout << __COUT_HDR_FL__ << "json " << json << std::endl;
>>>>>>> Stashed changes

					TBufferFile tbuff(TBuffer::kWrite);

					std::string rootType = histo->ClassName();
<<<<<<< Updated upstream
					__MOUT__ << "rootType " << rootType << std::endl;

					histo->Streamer(tbuff);

					//__MOUT__ << "histo length " << tbuff.Length() << std::endl;
=======
					std::cout << __COUT_HDR_FL__ << "rootType " << rootType << std::endl;

					histo->Streamer(tbuff);

					//std::cout << __COUT_HDR_FL__ << "histo length " << tbuff.Length() << std::endl;
>>>>>>> Stashed changes

					std::string dest;
					binaryBufferToHexString(tbuff.Buffer(), tbuff.Length(), dest);

					//*out << json.Data() << std::endl;
					//return;
					xmldoc.addTextElementToData("rootType", rootType);
					xmldoc.addTextElementToData("rootData", dest);
					xmldoc.addTextElementToData("rootJSON", json.Data());
<<<<<<< Updated upstream
					__MOUT__ << "Done" << std::endl;
=======
					std::cout << __COUT_HDR_FL__ << "Done" << std::endl;
>>>>>>> Stashed changes

					//std::cout << "\n\n" << json.Data() << "\n\n";
				}
			}
			else
			{
<<<<<<< Updated upstream
				__MOUT__ << "directory found getting the content!" << std::endl;
=======
				std::cout << __COUT_HDR_FL__ << "directory found getting the content!" << std::endl;
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
						__MOUT__ << __PRETTY_FUNCTION__ << "Class Name: " << obj->IsA()->GetName() << std::endl;
=======
						std::cout << __COUT_HDR_FL__ << __PRETTY_FUNCTION__ << "Class Name: " << obj->IsA()->GetName() << std::endl;
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
						__MOUT__ << __PRETTY_FUNCTION__ << "Class Name: " << key->GetClassName() << std::endl;
=======
						std::cout << __COUT_HDR_FL__ << __PRETTY_FUNCTION__ << "Class Name: " << key->GetClassName() << std::endl;
>>>>>>> Stashed changes
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

<<<<<<< Updated upstream
		__MOUT__ << "getEvents for run " << Run << std::endl;
=======
		std::cout << __COUT_HDR_FL__ << "getEvents for run " << Run << std::endl;
>>>>>>> Stashed changes

		if(Run != (int)loadedRunNumber_ || loadedRunNumber_ == (unsigned int)-1)
		{
			theDataManager_->load("Run1684.root","Monicelli");
			loadedRunNumber_ = Run;
		}

		DOMElement* eventsParent = xmldoc.addTextElementToData("events", "");
		DOMElement* eventParent;
		char str[40];

		const Visual3DEvents& events = theDataManager_->getVisual3DEvents();
<<<<<<< Updated upstream
		__MOUT__ << "Preparing hits xml" << std::endl;
		int numberOfEvents = 0;
		for(Visual3DEvents::const_iterator it=events.begin(); it!=events.end() && numberOfEvents < 10000; it++, numberOfEvents++)
		{
			//__MOUT__ << "Event: " << numberOfEvents << std::endl;
=======
		std::cout << __COUT_HDR_FL__ << "Preparing hits xml" << std::endl;
		int numberOfEvents = 0;
		for(Visual3DEvents::const_iterator it=events.begin(); it!=events.end() && numberOfEvents < 10000; it++, numberOfEvents++)
		{
			//std::cout << __COUT_HDR_FL__ << "Event: " << numberOfEvents << std::endl;
>>>>>>> Stashed changes
			eventParent = xmldoc.addTextElementToParent("event", str, eventsParent);
			const VisualHits& hits = it->getHits();
			for(VisualHits::const_iterator itHits=hits.begin(); itHits!=hits.end(); itHits++)
			{
				sprintf(str,"%f",itHits->x);
				xmldoc.addTextElementToParent("xyz_point", str, eventParent);
				sprintf(str,"%f",itHits->y);
				xmldoc.addTextElementToParent("xyz_point", str, eventParent);
				sprintf(str,"%f",itHits->z);
				xmldoc.addTextElementToParent("xyz_point", str, eventParent);
<<<<<<< Updated upstream
				//__MOUT__ << "X: " << itHits->x << " Y: " << itHits->y << " Z: " << itHits->z << std::endl;
=======
				//std::cout << __COUT_HDR_FL__ << "X: " << itHits->x << " Y: " << itHits->y << " Z: " << itHits->z << std::endl;
>>>>>>> Stashed changes
			}
			const VisualTracks& tracks = it->getTracks();
			for(VisualTracks::const_iterator itTrks=tracks.begin(); itTrks!=tracks.end(); itTrks++)
			{
				sprintf(str,"%f",itTrks->slopeX);
				xmldoc.addTextElementToParent("slope", str, eventParent);
				sprintf(str,"%f",itTrks->slopeY);
				xmldoc.addTextElementToParent("slope", str, eventParent);
				sprintf(str,"%f",itTrks->interceptX);
				xmldoc.addTextElementToParent("intcpt", str, eventParent);
				sprintf(str,"%f",itTrks->interceptY);
				xmldoc.addTextElementToParent("intcpt", str, eventParent);

			}

		}
<<<<<<< Updated upstream
		__MOUT__ << "Done hits xml" << std::endl;
	}
	else if (Command == "getGeometry")
	{
		__MOUT__ << "getGeometry" << std::endl;
=======
		std::cout << __COUT_HDR_FL__ << "Done hits xml" << std::endl;
	}
	else if (Command == "getGeometry")
	{
		std::cout << __COUT_HDR_FL__ << "getGeometry" << std::endl;
>>>>>>> Stashed changes

		//FIXME -- this crashes when the file doesn't exist!
		theDataManager_->load("Run1684.geo","Geometry");

<<<<<<< Updated upstream
		__MOUT__ << "getGeometry" << std::endl;
=======
		std::cout << __COUT_HDR_FL__ << "getGeometry" << std::endl;
>>>>>>> Stashed changes

		DOMElement* geometryParent = xmldoc.addTextElementToData("geometry", "");
		const Visual3DShapes& shapes = theDataManager_->getVisual3DGeometry().getShapes();

<<<<<<< Updated upstream
		__MOUT__ << "getGeometry" << std::endl;
=======
		std::cout << __COUT_HDR_FL__ << "getGeometry" << std::endl;
>>>>>>> Stashed changes


		DOMElement* objectParent;
		char str[40];
		for(Visual3DShapes::const_iterator itShapes=shapes.begin(); itShapes!=shapes.end(); itShapes++)
		{
			objectParent = xmldoc.addTextElementToParent("object", str, geometryParent);
			xmldoc.addTextElementToParent("object_type", "LINE_LOOP", objectParent);
			sprintf(str,"%d",itShapes->numberOfRows);
			xmldoc.addTextElementToParent("object_rows", str, objectParent);
			sprintf(str,"%d",itShapes->numberOfColumns);
			xmldoc.addTextElementToParent("object_columns", str, objectParent);
			for(Points::const_iterator itCorners=itShapes->corners.begin(); itCorners!=itShapes->corners.end(); itCorners++)
			{
				sprintf(str,"%f",itCorners->x);
				xmldoc.addTextElementToParent("xyz_point", str, objectParent);
				sprintf(str,"%f",itCorners->y);
				xmldoc.addTextElementToParent("xyz_point", str, objectParent);
				sprintf(str,"%f",itCorners->z);
				xmldoc.addTextElementToParent("xyz_point", str, objectParent);
			}
		}
	}
	else if (Command == "getRootConfig")
	{
		std::string path =  CgiDataUtilities::postData(cgi,"RootConfigPath");
<<<<<<< Updated upstream
		__MOUT__ << "path " << path << std::endl;
=======
		std::cout << __COUT_HDR_FL__ << "path " << path << std::endl;
>>>>>>> Stashed changes

		if(path.find("/" + PRE_MADE_ROOT_CFG_DIR + "/") == 0) //ROOT config path must start the path
		{
			path = std::string(ROOT_DISPLAY_CONFIG_PATH) + "/" + path.substr(PRE_MADE_ROOT_CFG_DIR.length()+2);
<<<<<<< Updated upstream
			__MOUT__ << "mod path " << path << std::endl;
=======
			std::cout << __COUT_HDR_FL__ << "mod path " << path << std::endl;
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
			__MOUT__ << "Insufficient permissions for Root Viewer Admin Controls: " << userPermissions << " < " << ROOT_VIEWER_PERMISSIONS_THRESHOLD << std::endl;
=======
			std::cout << __COUT_HDR_FL__ << "Insufficient permissions for Root Viewer Admin Controls: " << userPermissions << " < " << ROOT_VIEWER_PERMISSIONS_THRESHOLD << std::endl;
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
			__MOUT__ << "cmd " << cmd << std::endl;
			__MOUT__ << "path " << path << std::endl;
			__MOUT__ << "name " << name << std::endl;
=======
			std::cout << __COUT_HDR_FL__ << "cmd " << cmd << std::endl;
			std::cout << __COUT_HDR_FL__ << "path " << path << std::endl;
			std::cout << __COUT_HDR_FL__ << "name " << name << std::endl;
>>>>>>> Stashed changes

			if(path.find("/" + PRE_MADE_ROOT_CFG_DIR + "/") == 0) //ROOT config path must start the path
			{
				path = std::string(ROOT_DISPLAY_CONFIG_PATH) + "/" + path.substr(PRE_MADE_ROOT_CFG_DIR.length()+2) + name;
<<<<<<< Updated upstream
				__MOUT__ << "mod path " << path << std::endl;
=======
				std::cout << __COUT_HDR_FL__ << "mod path " << path << std::endl;
>>>>>>> Stashed changes


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
<<<<<<< Updated upstream
					__MOUT__ << "config " << config << std::endl;
					__MOUT__ << "useRunWildCard " << useRunWildCard << std::endl;
=======
					std::cout << __COUT_HDR_FL__ << "config " << config << std::endl;
					std::cout << __COUT_HDR_FL__ << "useRunWildCard " << useRunWildCard << std::endl;
>>>>>>> Stashed changes

					//check if file already exists
					FILE *fp = fopen(path.c_str(),"r");
					if(fp)
					{
						fclose(fp);
						xmldoc.addTextElementToData("status", "Failed. File already exists.");
<<<<<<< Updated upstream
						__MOUT__ << " Failed. File already exists." << std::endl;
=======
						std::cout << __COUT_HDR_FL__ << " Failed. File already exists." << std::endl;
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
								__MOUT__ << "Failed. Could not remove poorly formed Root config file!" << std::endl;
=======
								std::cout << __COUT_HDR_FL__ << "Failed. Could not remove poorly formed Root config file!" << std::endl;
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
		__MOUT__ << "Command request, " << Command << ", not recognized." << std::endl;
=======
		std::cout << __COUT_HDR_FL__ << "Command request, " << Command << ", not recognized." << std::endl;


	CLEANUP:

>>>>>>> Stashed changes
	//return xml doc holding server response
	xmldoc.outputXmlDocument((std::ostringstream*) out, false);
}


//========================================================================================================================
<<<<<<< Updated upstream
void VisualSupervisor::stateRunning(toolbox::fsm::FiniteStateMachine& fsm)
throw (toolbox::fsm::exception::Exception)
{

=======
void VisualSupervisor::stateRunning(toolbox::fsm::FiniteStateMachine& fsm) throw (toolbox::fsm::exception::Exception)
				{
>>>>>>> Stashed changes


				}

//========================================================================================================================
<<<<<<< Updated upstream
void VisualSupervisor::transitionConfiguring(toolbox::Event::Reference e)
throw (toolbox::fsm::exception::Exception)
{
=======
void VisualSupervisor::transitionConfiguring(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception)
				{
>>>>>>> Stashed changes

	theConfigurationGroupKey_ = theConfigurationManager_->makeTheConfigurationGroupKey(atoi(SOAPUtilities::translate(theStateMachine_.getCurrentMessage()).getParameters().getValue("ConfigurationGroupKey").c_str()));
	theConfigurationManager_->setupAllSupervisorConfigurations(theConfigurationGroupKey_,0);
	theDataManager_->configure();
<<<<<<< Updated upstream
}

//========================================================================================================================
void VisualSupervisor::transitionHalting(toolbox::Event::Reference e)
throw (toolbox::fsm::exception::Exception)
{

	theDataManager_->halt();
}

//========================================================================================================================
void VisualSupervisor::transitionStarting(toolbox::Event::Reference e)
throw (toolbox::fsm::exception::Exception)
{

	theDataManager_->start(SOAPUtilities::translate(theStateMachine_.getCurrentMessage()).getParameters().getValue("RunNumber"));
}

//========================================================================================================================
void VisualSupervisor::transitionStopping(toolbox::Event::Reference e)
throw (toolbox::fsm::exception::Exception)
{

	theDataManager_->stop();
}
=======
				}

//========================================================================================================================
void VisualSupervisor::transitionHalting(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception)
				{

	theDataManager_->halt();
				}

//========================================================================================================================
void VisualSupervisor::transitionStarting(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception)
				{

	theDataManager_->start(SOAPUtilities::translate(theStateMachine_.getCurrentMessage()).getParameters().getValue("RunNumber"));
				}

//========================================================================================================================
void VisualSupervisor::transitionStopping(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception)
				{

	theDataManager_->stop();
				}
>>>>>>> Stashed changes












