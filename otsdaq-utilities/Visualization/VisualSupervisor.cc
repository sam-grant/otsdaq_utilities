#include "otsdaq-utilities/Visualization/VisualSupervisor.h"
//#include "otsdaq-core/RootUtilities/DQMHistos.h"
#include "otsdaq-core/DataManager/DataManagerSingleton.h"
#include "otsdaq-core/Macros/BinaryStringMacros.h"

// ROOT documentation
// http://root.cern.ch/root/html/index.html

#include <TBuffer.h>
#include <TBufferJSON.h>
#include <TCanvas.h>
#include <TClass.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TIterator.h>
#include <TKey.h>
#include <TProfile.h>
#include <TROOT.h>
#include <TRegexp.h>
#include <TString.h>
#include "TBufferFile.h"
#include "TObject.h"

#include <dirent.h>   /*DIR and dirent*/
#include <sys/stat.h> /*mkdir*/

#include <xdaq/NamespaceURI.h>

#include <iostream>

#define ROOT_BROWSER_PATH __ENV__("ROOT_BROWSER_PATH")
#define ROOT_DISPLAY_CONFIG_PATH __ENV__("ROOT_DISPLAY_CONFIG_PATH")

#define LIVEDQM_DIR std::string("LIVE_DQM")
#define PRE_MADE_ROOT_CFG_DIR std::string("Pre-made Views")

#define PRE_MADE_ROOT_CFG_FILE_EXT std::string(".rcfg")

#define PREFERENCES_PATH std::string(__ENV__("SERVICE_DATA_PATH")) + "/VisualizerData/"
#define PREFERENCES_FILE_EXT ".pref"

#define ROOT_VIEWER_PERMISSIONS_THRESHOLD 100

using namespace ots;

#undef __MF_SUBJECT__
#define __MF_SUBJECT__ "Visualizer"

XDAQ_INSTANTIATOR_IMPL(VisualSupervisor)

//========================================================================================================================
VisualSupervisor::VisualSupervisor(xdaq::ApplicationStub* stub)
    : CoreSupervisorBase(stub), theDataManager_(0), loadedRunNumber_(-1)
{
	__SUP_COUT__ << "Constructor." << __E__;
	INIT_MF("VisualSupervisor");

	theDataManager_ = DataManagerSingleton::getInstance<VisualDataManager>(
	    theConfigurationManager_->getNode(
	        theConfigurationManager_->__GET_CONFIG__(XDAQContextTable)->getTableName()),
			CorePropertySupervisorBase::getSupervisorConfigurationPath(),
			CorePropertySupervisorBase::getSupervisorUID());

	CoreSupervisorBase::theStateMachineImplementation_.push_back(theDataManager_);

	__SUP_COUT__ << "Done instantiating Visual data manager." << __E__;

	// xgi::bind(this, &VisualSupervisor::safariDefaultPage, "safari" );

	// make preferences directory in case they don't exist
	mkdir(((std::string)PREFERENCES_PATH).c_str(), 0755);

	__SUP_COUT__ << "Constructed." << __E__;
} //end constructor

//========================================================================================================================
VisualSupervisor::~VisualSupervisor(void)
{
	__SUP_COUT__ << "Destructor." << __E__;
	destroy();
	__SUP_COUT__ << "Destructed." << __E__;
}  // end destructor()

//========================================================================================================================
void VisualSupervisor::destroy(void)
{
	__SUP_COUT__ << "Destroying..." << __E__;

	// called by destructor
	// delete theConfigurationManager_;

	DataManagerSingleton::deleteInstance(
	    CorePropertySupervisorBase::getSupervisorUID());
	theStateMachineImplementation_.pop_back();
} //end destroy()
//
////========================================================================================================================
// void VisualSupervisor::defaultPage(xgi::Input * in, xgi::Output * out )
//{
//	//__SUP_COUT__ << this->getApplicationContext()->getURL() << __E__;
//
//	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame
// src='/WebPath/html/Visualization.html?urn=" <<
//			this->getApplicationDescriptor()->getLocalId() <<"'></frameset></html>";
//
//}
//
////========================================================================================================================
// void VisualSupervisor::safariDefaultPage(xgi::Input * in, xgi::Output * out )
//{
//	*out << "<!DOCTYPE HTML><html lang='en'><iframe
// style='width:100%;height:100%;position:absolute;left:0;top:0;border:0;padding:0;margin:0;'
// src='/WebPath/html/Visualization.html?urn=" <<
//			this->getApplicationDescriptor()->getLocalId() <<"'></iframe></html>";
//}

//========================================================================================================================
// setSupervisorPropertyDefaults
//		override to set defaults for supervisor property values (before user settings
// override)
void VisualSupervisor::setSupervisorPropertyDefaults()
{
	CorePropertySupervisorBase::setSupervisorProperty(
	    CorePropertySupervisorBase::SUPERVISOR_PROPERTIES.AllowNoLoginRequestTypes,
	    "setUserPreferences |  getUserPreferences | getDirectoryContents | getRoot | "
	    "getEvents");

	CorePropertySupervisorBase::setSupervisorProperty(
	    CorePropertySupervisorBase::SUPERVISOR_PROPERTIES.UserPermissionsThreshold,
	    "*=1 | rootAdminControls=100");
}

//========================================================================================================================
// forceSupervisorPropertyValues
//		override to force supervisor property values (and ignore user settings)
void VisualSupervisor::forceSupervisorPropertyValues()
{
	CorePropertySupervisorBase::setSupervisorProperty(
	    CorePropertySupervisorBase::SUPERVISOR_PROPERTIES.AutomatedRequestTypes,
	    "getRoot | getEvents");
	CorePropertySupervisorBase::setSupervisorProperty(
	    CorePropertySupervisorBase::SUPERVISOR_PROPERTIES.NoXmlWhiteSpaceRequestTypes,
	    "getRoot | getEvents");  // json data in ROOTJS library expects no funny
	                             // characters
	                             //	CorePropertySupervisorBase::setSupervisorProperty(CorePropertySupervisorBase::SUPERVISOR_PROPERTIES.NeedUsernameRequestTypes,
	                             //			"setUserPreferences | getUserPreferences");
}

//========================================================================================================================
void VisualSupervisor::request(const std::string&               requestType,
                               cgicc::Cgicc&                    cgiIn,
                               HttpXmlDocument&                 xmlOut,
                               const WebUsers::RequestUserInfo& userInfo)
{
	// Commands
	//	getRawData
	//	getGeometry
	//	getEvents
	//	getRoot
	//	getDirectoryContents
	//	setUserPreferences
	//	getUserPreference

	//    if (requestType == "getHisto" && theDataManager_->getLiveDQMHistos() != 0)
	//    {
	//       // TH1I* histo =
	//       (TH1I*)theDataManager_->getLiveDQMHistos()->get("Planes/Plane_0_Occupancy");
	//
	//        //        theDataManager_->load("Run185_Histo.root","Histograms");
	//        //TH1F*     histo1d  = theDataManager_->getFileDQMHistos().getHisto1D();
	//        //TCanvas*  canvas   = theDataManager_->getFileDQMHistos().getCanvas ();
	//        //TH2F*     histo2d  = theDataManager_->getFileDQMHistos().getHisto2D();
	//        //TProfile* profile  = theDataManager_->getFileDQMHistos().getProfile();
	//
	//    }
	if(requestType == "getRawData")
	{
		__SUP_COUT__ << __E__;
		try
		{
			// TODO -- add timestamp, so we know if data is new
			__SUP_COUT__ << "Getting Raw data and converting to binary string" << __E__;
			xmlOut.addBinaryStringToData("rawData", theDataManager_->getRawData());
			__SUP_COUT__ << __E__;
		}
		catch(std::exception const& e)
		{
			__SUP_COUT__
			    << "ERROR! Exception while getting raw data. Incoming exception data..."
			    << __E__;
			__SUP_COUT__ << e.what() << __E__;
			__SUP_COUT__ << "End Exception Data" << __E__;
		}
		catch(...)
		{
			__SUP_COUT__ << "ERROR! Something went wrong trying to get raw data."
			             << __E__;
			__SUP_COUT_INFO__ << "ERROR! Something went wrong trying to get raw data."
			                  << __E__;
		}
	}
	else if(requestType == "setUserPreferences" &&
	        userInfo.username_ != "" /*from allow no user*/)
	{
		__SUP_COUT__ << "userInfo.username_: " << userInfo.username_ << __E__;
		std::string fullPath =
		    (std::string)PREFERENCES_PATH + userInfo.username_ + PREFERENCES_FILE_EXT;
		__SUP_COUT__ << "fullPath: " << fullPath << __E__;

		std::string radioSelect = CgiDataUtilities::getData(cgiIn, "radioSelect");
		std::string autoRefresh = CgiDataUtilities::getData(cgiIn, "autoRefresh");
		std::string autoHide    = CgiDataUtilities::getData(cgiIn, "autoHide");
		std::string hardRefresh = CgiDataUtilities::getData(cgiIn, "hardRefresh");
		std::string autoRefreshPeriod =
		    CgiDataUtilities::getData(cgiIn, "autoRefreshPeriod");

		__SUP_COUT__ << "radioSelect: " << radioSelect << __E__;
		__SUP_COUT__ << "autoRefresh: " << autoRefresh << __E__;
		__SUP_COUT__ << "autoHide: " << autoHide << __E__;
		__SUP_COUT__ << "hardRefresh: " << hardRefresh << __E__;
		__SUP_COUT__ << "autoRefreshPeriod: " << autoRefreshPeriod << __E__;

		// read existing
		FILE* fp = fopen(fullPath.c_str(), "r");
		if(fp)
		{
			char line[100];
			char val[100];

			fgets(line, 100, fp);
			sscanf(line, "%*s %s", val);
			if(radioSelect == "")
				radioSelect = val;

			fgets(line, 100, fp);
			sscanf(line, "%*s %s", val);
			if(autoRefresh == "")
				autoRefresh = val;

			fgets(line, 100, fp);
			sscanf(line, "%*s %s", val);
			if(autoHide == "")
				autoHide = val;

			fgets(line, 100, fp);
			sscanf(line, "%*s %s", val);
			if(hardRefresh == "")
				hardRefresh = val;

			fgets(line, 100, fp);
			sscanf(line, "%*s %s", val);
			if(autoRefreshPeriod == "")
				autoRefreshPeriod = val;

			fclose(fp);
		}

		// write new
		fp = fopen(fullPath.c_str(), "w");
		if(fp)
		{
			fprintf(fp, "radioSelect %s\n", radioSelect.c_str());
			fprintf(fp, "autoRefresh %s\n", autoRefresh.c_str());
			fprintf(fp, "autoHide %s\n", autoHide.c_str());
			fprintf(fp, "hardRefresh %s\n", hardRefresh.c_str());
			fprintf(fp, "autoRefreshPeriod %s\n", autoRefreshPeriod.c_str());
			fclose(fp);
		}
		else
			__SUP_COUT_ERR__ << "Failure writing preferences to file: " << fullPath
			                 << __E__;
	}
	else if(requestType == "getUserPreferences")
	{
		__SUP_COUT__ << "userInfo.username_: " << userInfo.username_ << __E__;
		std::string fullPath =
		    (std::string)PREFERENCES_PATH + userInfo.username_ + PREFERENCES_FILE_EXT;
		__SUP_COUT__ << "fullPath: " << fullPath << __E__;

		FILE* fp = fopen(fullPath.c_str(), "r");
		if(fp)
		{
			char line[100];
			// char val[100];
			int val;

			fgets(line, 100, fp);
			sscanf(line, "%*s %d", &val);
			if(val < 0 || val > 3)
				val = 0;  // FIXME.. value can get corrupt...
			xmlOut.addTextElementToData("radioSelect", std::to_string(val));
			fgets(line, 100, fp);
			sscanf(line, "%*s %d", &val);
			xmlOut.addTextElementToData("autoRefresh", std::to_string(val));
			fgets(line, 100, fp);
			sscanf(line, "%*s %d", &val);
			xmlOut.addTextElementToData("autoHide", std::to_string(val));
			fgets(line, 100, fp);
			sscanf(line, "%*s %d", &val);
			xmlOut.addTextElementToData("hardRefresh", std::to_string(val));
			fgets(line, 100, fp);
			sscanf(line, "%*s %d", &val);
			xmlOut.addTextElementToData("autoRefreshPeriod", std::to_string(val));
			fclose(fp);
		}
		else
		{
			// else assume user has no preferences yet
			xmlOut.addTextElementToData("radioSelect", "");
			xmlOut.addTextElementToData("autoRefresh", "");
			xmlOut.addTextElementToData("autoHide", "");
			xmlOut.addTextElementToData("hardRefresh", "");
			xmlOut.addTextElementToData("autoRefreshPeriod", "");
		}
	}
	else if(requestType == "getDirectoryContents")
	{
		// return directory structure for requested path, types are "dir" and "file"

		std::string rootpath = std::string(ROOT_BROWSER_PATH) + "/";
		std::string path     = CgiDataUtilities::postData(cgiIn, "Path");
		__SUP_COUT__ << path << __E__;

		// return 1 if user has access to admin controls, else 0
		char permStr[10];
		sprintf(permStr,
		        "%d",
		        userInfo.permissionLevel_ >=
		            CoreSupervisorBase::getSupervisorPropertyUserPermissionsThreshold(
		                "rootAdminControls"));
		xmlOut.addTextElementToData("permissions", permStr);  // add permissions

		std::string dirpath = rootpath + path;
		if(path == "/" + PRE_MADE_ROOT_CFG_DIR + "/")
			dirpath = ROOT_DISPLAY_CONFIG_PATH;

		if(path.find("/" + PRE_MADE_ROOT_CFG_DIR + "/") ==
		   0)  // ROOT config path must start the path
			dirpath = std::string(ROOT_DISPLAY_CONFIG_PATH) + "/" +
			          path.substr(PRE_MADE_ROOT_CFG_DIR.length() + 2);

		__SUP_COUT__ << "full path: " << dirpath << __E__;

		DIR*           pDIR;
		struct dirent* entry;
		bool           isNotRtCfg;
		bool           isDir;
		if((pDIR = opendir(dirpath.c_str())))
		{
			xmlOut.addTextElementToData("path", path);

			// add LIVE if path is / and DQM is active
			// add Pre-made Views if path is / and ROOT_DISPLAY_CONFIG_PATH isnt already
			// there
			if(path == "/")
			{
				if(theDataManager_->getLiveDQMHistos() != 0)
					xmlOut.addTextElementToData("dir",
					                            LIVEDQM_DIR + ".root");  // add to xml

				// check for ROOT_DISPLAY_CONFIG_PATH
				DIR* pRtDIR  = opendir(ROOT_DISPLAY_CONFIG_PATH);
				bool recheck = false;
				if(!pRtDIR)  // if doesn't exist, make it
				{
					recheck = true;
					if(mkdir(ROOT_DISPLAY_CONFIG_PATH,
					         S_IRWXU | (S_IRGRP | S_IXGRP) |
					             (S_IROTH | S_IXOTH)))  // mode = drwx r-x r-x
						__SUP_COUT__ << "Failed to make directory for pre made views: "
						             << ROOT_DISPLAY_CONFIG_PATH << __E__;
				}
				else
					closedir(pRtDIR);  // else close and display

				if(!recheck || (pRtDIR = opendir(ROOT_DISPLAY_CONFIG_PATH)))
				{
					xmlOut.addTextElementToData("dir",
					                            PRE_MADE_ROOT_CFG_DIR);  // add to xml
					if(recheck)
						closedir(pRtDIR);
				}
			}

			while((entry = readdir(pDIR)))
			{
				//__SUP_COUT__ << int(entry->d_type) << " " << entry->d_name << "\n" <<
				// __E__;
				if(entry->d_name[0] != '.' &&
				   (entry->d_type ==
				        0 ||  // 0 == UNKNOWN (which can happen - seen in SL7 VM)
				    entry->d_type == 4 ||
				    entry->d_type == 8))
				{
					//__SUP_COUT__ << int(entry->d_type) << " " << entry->d_name << "\n"
					//<< __E__;
					isNotRtCfg =
					    std::string(entry->d_name).find(".rcfg") == std::string::npos;
					isDir = false;

					if(entry->d_type == 0)
					{
						// unknown type .. determine if directory
						DIR* pTmpDIR = opendir((dirpath + entry->d_name).c_str());
						if(pTmpDIR)
						{
							isDir = true;
							closedir(pTmpDIR);
						}
						// else //assume file
					}

					if((entry->d_type == 8 ||
					    (!isDir && entry->d_type == 0))  // file type
					   && std::string(entry->d_name).find(".root") == std::string::npos &&
					   isNotRtCfg)
						continue;  // skip if not a root file or a config file
					else if(entry->d_type == 4)
						isDir = true;  // flag directory types

					xmlOut.addTextElementToData(
					    isDir ? "dir" : (isNotRtCfg ? "dir" : "file"), entry->d_name);
				}
			}
			closedir(pDIR);
		}
		else
			__SUP_COUT__ << "Failed to access directory contents!" << __E__;
	}
	else if(requestType == "getRoot")
	{
		// return directory structure for requested ROOT path, types are "dir" and "file"

		std::string path     = CgiDataUtilities::postData(cgiIn, "RootPath");
		std::string fullPath = std::string(__ENV__("ROOT_BROWSER_PATH")) + path;

		//__SUP_COUT__ << "Full path:-" << fullPath << "-" << __E__;

		std::string rootFileName = fullPath.substr(0, fullPath.find(".root") + 5);
		std::string rootDirectoryName =
		    rootFileName + ":" +
		    fullPath.substr(fullPath.find(".root") + 5,
		                    fullPath.size() - fullPath.find(".root") + 5 + 1);

		std::string::size_type LDQM_pos = path.find("/" + LIVEDQM_DIR + ".root/");
		TFile*                 rootFile;

		if(theDataManager_->getLiveDQMHistos() != nullptr && LDQM_pos == 0)
		{
			//__SUP_COUT__ << "Attempting to get LIVE file." << __E__;
			rootFile = theDataManager_->getLiveDQMHistos()->getFile();
			if(!rootFile)
				__SUP_COUT__ << "File was closed." << __E__;
			else
			{
				//__SUP_COUT__ << "LIVE file name: " << rootFile->GetName() << __E__;
				rootDirectoryName = path.substr(("/" + LIVEDQM_DIR + ".root").length());
			}
		}
		else
			rootFile = TFile::Open(rootFileName.c_str());

		//__SUP_COUT__ << "FileName : " << rootFileName << " Object: " <<
		// rootDirectoryName << __E__;

		if(!rootFile || !rootFile->IsOpen())
		{
			__SUP_COUT__ << "Failed to access root file: " << rootFileName << __E__;
		}
		else
		{
			xmlOut.addTextElementToData("path", path);

			TDirectory* directory;
			if((directory = rootFile->GetDirectory(rootDirectoryName.c_str())) == 0)
			{
				//__SUP_COUT__ << "This is not a directory!" << __E__;
				directory = rootFile;

				// failed directory so assume it's file
				//__SUP_COUT__ << "Getting object name: " << rootDirectoryName <<
				// __E__;
				TObject* histoClone = nullptr;
				TObject* histo      = (TObject*)rootFile->Get(rootDirectoryName.c_str());

				if(histo != nullptr)  // turns out was a root object path
				{
					// Clone histo to avoid conflict when it is filled by other threads
					histoClone       = histo->Clone();
					TString     json = TBufferJSON::ConvertToJSON(histoClone);
					TBufferFile tBuffer(TBuffer::kWrite);
					histoClone->Streamer(tBuffer);

					//__SUP_COUT__ << "histo length " << tbuff.Length() << __E__;

					std::string destination = BinaryStringMacros::binaryToHexString(
					    tBuffer.Buffer(), tBuffer.Length());

					xmlOut.addTextElementToData("rootType", histoClone->ClassName());
					xmlOut.addTextElementToData("rootData", destination);
					xmlOut.addTextElementToData("rootJSON", json.Data());
					delete histoClone;
				}
				else
					__SUP_COUT_ERR__ << "Failed to access:-" << rootDirectoryName << "-"
					                 << __E__;
			}
			else
			{
				__SUP_COUT__ << "directory found getting the content!" << __E__;
				TRegexp re("*", kTRUE);
				if(LDQM_pos == 0)
				{
					TObject* obj;
					TIter    nextobj(directory->GetList());
					while((obj = (TObject*)nextobj()))
					{
						TString s = obj->GetName();
						if(s.Index(re) == kNPOS)
							continue;
						__SUP_COUT__ << "Class Name: " << obj->IsA()->GetName()
						             << __E__;
						xmlOut.addTextElementToData(
						    (std::string(obj->IsA()->GetName()).find("Directory") !=
						     std::string::npos)
						        ? "dir"
						        : "file",
						    obj->GetName());
					}
				}
				else
				{
					TKey* key;
					TIter next(directory->GetListOfKeys());
					while((key = (TKey*)next()))
					{
						TString s = key->GetName();
						if(s.Index(re) == kNPOS)
							continue;
						__SUP_COUT__ << "Class Name: " << key->GetClassName()
						             << __E__;
						xmlOut.addTextElementToData(
						    (std::string(key->GetClassName()).find("Directory") !=
						     std::string::npos)
						        ? "dir"
						        : "file",
						    key->GetName());
					}
				}
			}
			if(LDQM_pos == std::string::npos)
				rootFile->Close();
		}
	}
	else if(requestType == "getEvents")
	{
		int Run = atoi(cgiIn("run").c_str());

		__SUP_COUT__ << "getEvents for run " << Run << __E__;

		if(Run != (int)loadedRunNumber_ || loadedRunNumber_ == (unsigned int)-1)
		{
			theDataManager_->load("Run1684.root", "Monicelli");
			loadedRunNumber_ = Run;
		}

		DOMElement* eventsParent = xmlOut.addTextElementToData("events", "");
		DOMElement* eventParent;
		char        str[40];

		//		const Visual3DEvents& events = theDataManager_->getVisual3DEvents();
		//		__SUP_COUT__ << "Preparing hits xml" << __E__;
		//		int numberOfEvents = 0;
		//		for(Visual3DEvents::const_iterator it=events.begin(); it!=events.end() &&
		// numberOfEvents < 10000; it++, numberOfEvents++)
		//		{
		//			//__SUP_COUT__ << "Event: " << numberOfEvents << __E__;
		//			eventParent = xmlOut.addTextElementToParent("event", str,
		// eventsParent); 			const VisualHits& hits = it->getHits();
		//			for(VisualHits::const_iterator itHits=hits.begin();
		// itHits!=hits.end();  itHits++)
		//			{
		//				sprintf(str,"%f",itHits->x);
		//				xmlOut.addTextElementToParent("xyz_point", str, eventParent);
		//				sprintf(str,"%f",itHits->y);
		//				xmlOut.addTextElementToParent("xyz_point", str, eventParent);
		//				sprintf(str,"%f",itHits->z);
		//				xmlOut.addTextElementToParent("xyz_point", str, eventParent);
		//				//__SUP_COUT__ << "X: " << itHits->x << " Y: " << itHits->y << "
		// Z:
		//"
		//<<  itHits->z << __E__;
		//			}
		//			const VisualTracks& tracks = it->getTracks();
		//			for(VisualTracks::const_iterator itTrks=tracks.begin();
		// itTrks!=tracks.end(); itTrks++)
		//			{
		//				sprintf(str,"%f",itTrks->slopeX);
		//				xmlOut.addTextElementToParent("slope", str, eventParent);
		//				sprintf(str,"%f",itTrks->slopeY);
		//				xmlOut.addTextElementToParent("slope", str, eventParent);
		//				sprintf(str,"%f",itTrks->interceptX);
		//				xmlOut.addTextElementToParent("intcpt", str, eventParent);
		//				sprintf(str,"%f",itTrks->interceptY);
		//				xmlOut.addTextElementToParent("intcpt", str, eventParent);
		//
		//			}
		//
		//		}
		__SUP_COUT__ << "Done hits xml" << __E__;
	}
	else if(requestType == "getGeometry")
	{
		__SUP_COUT__ << "getGeometry" << __E__;

		// FIXME -- this crashes when the file doesn't exist!
		theDataManager_->load("Run1684.geo", "Geometry");

		__SUP_COUT__ << "getGeometry" << __E__;

		DOMElement* geometryParent = xmlOut.addTextElementToData("geometry", "");
		//		const Visual3DShapes& shapes =
		// theDataManager_->getVisual3DGeometry().getShapes();
		//
		//		__SUP_COUT__ << "getGeometry" << __E__;
		//
		//
		//		DOMElement* objectParent;
		//		char str[40];
		//		for(Visual3DShapes::const_iterator itShapes=shapes.begin();
		// itShapes!=shapes.end(); itShapes++)
		//		{
		//			objectParent = xmlOut.addTextElementToParent("object", str,
		// geometryParent);
		//			xmlOut.addTextElementToParent("object_type", "LINE_LOOP",
		// objectParent); 			sprintf(str,"%d",itShapes->numberOfRows);
		//			xmlOut.addTextElementToParent("object_rows", str, objectParent);
		//			sprintf(str,"%d",itShapes->numberOfColumns);
		//			xmlOut.addTextElementToParent("object_columns", str, objectParent);
		//			for(Points::const_iterator itCorners=itShapes->corners.begin();
		// itCorners!=itShapes->corners.end(); itCorners++)
		//			{
		//				sprintf(str,"%f",itCorners->x);
		//				xmlOut.addTextElementToParent("xyz_point", str, objectParent);
		//				sprintf(str,"%f",itCorners->y);
		//				xmlOut.addTextElementToParent("xyz_point", str, objectParent);
		//				sprintf(str,"%f",itCorners->z);
		//				xmlOut.addTextElementToParent("xyz_point", str, objectParent);
		//			}
		//		}
	}
	else if(requestType == "getRootConfig")
	{
		std::string path = CgiDataUtilities::postData(cgiIn, "RootConfigPath");
		__SUP_COUT__ << "path " << path << __E__;

		if(path.find("/" + PRE_MADE_ROOT_CFG_DIR + "/") ==
		   0)  // ROOT config path must start the path
		{
			path = std::string(ROOT_DISPLAY_CONFIG_PATH) + "/" +
			       path.substr(PRE_MADE_ROOT_CFG_DIR.length() + 2);
			__SUP_COUT__ << "mod path " << path << __E__;
		}

		HttpXmlDocument cfgXml;
		if(cfgXml.loadXmlDocument(path))
		{
			xmlOut.addTextElementToData("status", "1");
			xmlOut.copyDataChildren(cfgXml);  // copy file to output xml
			cfgXml.saveXmlDocument(path);
		}
		else
			xmlOut.addTextElementToData("status",
			                            "Failed. File to properly load config file.");
	}
	else if(requestType == "rootAdminControls")
	{
		//		if(userPermissions < ROOT_VIEWER_PERMISSIONS_THRESHOLD)
		//		{
		//			__SUP_COUT__ << "Insufficient permissions for Root Viewer Admin
		// Controls: " << userPermissions << " < " << ROOT_VIEWER_PERMISSIONS_THRESHOLD <<
		// __E__;
		//			xmlOut.addTextElementToData("status", "Failed. Insufficient user
		// permissions.");
		//		}
		//		else
		//		{
		std::string cmd = cgiIn("cmd");  // possible commands are
		//	mkdir
		//	save
		// 	delete

		std::string path = CgiDataUtilities::postData(cgiIn, "path");
		std::string name = CgiDataUtilities::postData(cgiIn, "name");
		__SUP_COUT__ << "cmd " << cmd << __E__;
		__SUP_COUT__ << "path " << path << __E__;
		__SUP_COUT__ << "name " << name << __E__;

		if(path.find("/" + PRE_MADE_ROOT_CFG_DIR + "/") ==
		   0)  // ROOT config path must start the path
		{
			path = std::string(ROOT_DISPLAY_CONFIG_PATH) + "/" +
			       path.substr(PRE_MADE_ROOT_CFG_DIR.length() + 2) + name;
			__SUP_COUT__ << "mod path " << path << __E__;

			if(cmd == "mkdir")
			{
				if(mkdir(path.c_str(),
				         S_IRWXU | (S_IRGRP | S_IXGRP) |
				             (S_IROTH | S_IXOTH)))  // mode = drwx r-x r-x
					xmlOut.addTextElementToData("status",
					                            "Failed. Directory create rejected.");
				else
					xmlOut.addTextElementToData("status", "1");  // success
			}
			else if(cmd == "save")
			{
				path += PRE_MADE_ROOT_CFG_FILE_EXT;  // add file extension

				bool useRunWildCard =
				    atoi(CgiDataUtilities::postData(cgiIn, "useRunWildCard")
				             .c_str());  // 0 or 1
				std::string config = CgiDataUtilities::postData(cgiIn, "config");
				__SUP_COUT__ << "config " << config << __E__;
				__SUP_COUT__ << "useRunWildCard " << useRunWildCard << __E__;

				// check if file already exists
				FILE* fp = fopen(path.c_str(), "r");
				if(fp)
				{
					fclose(fp);
					xmlOut.addTextElementToData("status", "Failed. File already exists.");
					__SUP_COUT__ << " Failed. File already exists." << __E__;
				}
				else
				{
					// dump to file
					// verify proper format through read back
					// if successfully loaded, re-save for formatting

					// dump to file
					fp = fopen(path.c_str(), "w");
					fputs(config.c_str(), fp);
					fclose(fp);

					// verify proper format through read back
					HttpXmlDocument cfgXml;
					if(cfgXml.loadXmlDocument(path))
					{
						// successfully loaded, re-save for formatting
						cfgXml.saveXmlDocument(path);
						xmlOut.addTextElementToData("status", "1");  // success
					}
					else  // failed to load properly
					{
						xmlOut.addTextElementToData(
						    "status", "Failed. Fatal. Improper file format.");
						if(remove(path.c_str()) != 0)
							__SUP_COUT__ << "Failed. Could not remove poorly formed Root "
							                "config file!"
							             << __E__;
					}
				}
			}
			else if(cmd == "delete")
			{
				// guess first directory and then file
				if(rmdir(path.c_str()) == 0 ||
				   remove((path + PRE_MADE_ROOT_CFG_FILE_EXT).c_str()) == 0)
					xmlOut.addTextElementToData("status", "1");  // success
				else
					xmlOut.addTextElementToData("status",
					                            "Failed. Target could not be deleted.");
			}
			else
				xmlOut.addTextElementToData("status", "Failed. Unrecognized command.");
		}
		else
			xmlOut.addTextElementToData("status", "Failed. Invalid path.");

		//}
	}
	else
		__SUP_COUT__ << "requestType request, " << requestType << ", not recognized."
		             << __E__;
	// return xml doc holding server response
	// xmlOut.outputXmlDocument((std::ostringstream*) out, false);
}
