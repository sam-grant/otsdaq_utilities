#include "otsdaq-utilities/Logbook/LogbookSupervisor.h"
#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/Macros/CoutHeaderMacros.h"
#include "otsdaq-core/CgiDataUtilities/CgiDataUtilities.h"
#include "otsdaq-core/XmlUtilities/HttpXmlDocument.h"
#include "otsdaq-core/SOAPUtilities/SOAPUtilities.h"
#include "otsdaq-core/SOAPUtilities/SOAPParameters.h"

#include <xdaq/NamespaceURI.h>

#include <iostream>
#include <fstream>
#include <string>
#include <dirent.h> 	//for DIR
#include <sys/stat.h> 	//for mkdir
#include <thread>       // std::thread

using namespace ots;


const std::string LOGBOOK_PATH = getenv("LOGBOOK_DATA_PATH") + std::string("/");
#define LOGBOOK_EXPERIMENT_LIST_PATH 	LOGBOOK_PATH + "experiment_list.xml"
#define LOGBOOK_EXPERIMENT_DIR_PREFACE	"log_"
#define LOGBOOK_UPLOADS_PATH 			"uploads/" 	//within experiment directory
#define LOGBOOK_LOGBOOKS_PATH 			"logbooks/"
#define LOGBOOK_PREVIEWS_PATH 			"previews/"
#define LOGBOOK_FILE_PREFACE			"entries_"
#define LOGBOOK_FILE_EXTENSION			".xml"

#define ACTIVE_EXPERIMENT_PATH 			LOGBOOK_PATH + "active_experiment.txt"
#define REMOVE_EXPERIMENT_LOG_PATH 		LOGBOOK_PATH + "removed_experiments.log"

#define XML_ADMIN_STATUS				"logbook_admin_status"
#define XML_STATUS						"logbook_status"
#define XML_MOST_RECENT_DAY				"most_recent_day"
#define XML_EXPERIMENTS_ROOT 			"experiments"
#define XML_EXPERIMENT		 			"experiment"
#define XML_ACTIVE_EXPERIMENT		 	"active_experiment"
#define XML_EXPERIMENT_CREATE			"create_time"
#define XML_EXPERIMENT_CREATOR 			"creator"

#define XML_LOGBOOK_ENTRY               "logbook_entry"
#define XML_LOGBOOK_ENTRY_SUBJECT       "logbook_entry_subject"
#define XML_LOGBOOK_ENTRY_TEXT          "logbook_entry_text"
#define XML_LOGBOOK_ENTRY_FILE          "logbook_entry_file"
#define XML_LOGBOOK_ENTRY_TIME          "logbook_entry_time"
#define XML_LOGBOOK_ENTRY_CREATOR       "logbook_entry_creator"
#define XML_LOGBOOK_ENTRY_HIDDEN 	    "logbook_entry_hidden"
#define XML_LOGBOOK_ENTRY_HIDER			"logbook_entry_hider"
#define XML_LOGBOOK_ENTRY_HIDDEN_TIME   "logbook_entry_hidden_time"

#define XML_PREVIEW_INDEX				"preview_index"
#define LOGBOOK_PREVIEW_FILE        	"preview.xml"
#define LOGBOOK_PREVIEW_UPLOAD_PREFACE  "upload_"

XDAQ_INSTANTIATOR_IMPL(LogbookSupervisor)

#undef 	__MF_SUBJECT__
#define __MF_SUBJECT__ "Logbook"

//========================================================================================================================
//sendmail ~~
//	Helper function to send emails to the subscriber list of the active experiment
int sendmail(const char *to, const char *from, const char *subject, const char *message)
{
    int retval = -1;
    FILE *mailpipe = popen("/usr/lib/sendmail -t", "w");
    if (mailpipe != NULL) {
        fprintf(mailpipe, "To: %s\n", to);
        fprintf(mailpipe, "From: %s\n", from);
        fprintf(mailpipe, "Subject: %s\n\n", subject);
        fwrite(message, 1, strlen(message), mailpipe);
        fwrite(".\n", 1, 2, mailpipe);
        pclose(mailpipe);
        retval = 0;
     }
     else {
         perror("Failed to invoke sendmail");
     }
     return retval;
}



//========================================================================================================================
LogbookSupervisor::LogbookSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception):
xdaq::Application(s   ),
SOAPMessenger  (this),
theRemoteWebUsers_(this)
{
	INIT_MF("LogbookSupervisor");
	xgi::bind (this, &LogbookSupervisor::Default,                	"Default" );
	xgi::bind (this, &LogbookSupervisor::Log,                		"Log" );
	xgi::bind (this, &LogbookSupervisor::LogImage,               	"LogImage" );
	xgi::bind (this, &LogbookSupervisor::LogReport,             	"LogReport" );

	xoap::bind(this, &LogbookSupervisor::MakeSystemLogbookEntry,   	"MakeSystemLogbookEntry"      	, XDAQ_NS_URI);

	//init allowed file upload types
	allowedFileUploadTypes_.push_back("image/png"); matchingFileUploadTypes_.push_back("png");
	allowedFileUploadTypes_.push_back("image/jpeg"); matchingFileUploadTypes_.push_back("jpeg");
	allowedFileUploadTypes_.push_back("image/gif"); matchingFileUploadTypes_.push_back("gif");
	allowedFileUploadTypes_.push_back("image/bmp"); matchingFileUploadTypes_.push_back("bmp");
	allowedFileUploadTypes_.push_back("application/pdf"); matchingFileUploadTypes_.push_back("pdf");
	allowedFileUploadTypes_.push_back("application/zip"); matchingFileUploadTypes_.push_back("zip");
	allowedFileUploadTypes_.push_back("text/plain"); matchingFileUploadTypes_.push_back("txt");

	init();

	//TODO allow admins to subscribe email addresses to the active experiment
	//sendmail("rrivera@fnal.gov","ots-instance@otsdaq.fnal.gov","My Subject","Message\nHello!\n\nwhats up.");
}

//========================================================================================================================
LogbookSupervisor::~LogbookSupervisor(void)
{
	destroy();
}
//========================================================================================================================
void LogbookSupervisor::init(void)
{
	//called by constructor
	theSupervisorsConfiguration_.init(getApplicationContext());


	if(1) //check if LOGBOOK_PATH and subpaths event exist?! (if not, attempt to create)
	{
		std::string path = LOGBOOK_PATH;
		DIR *dir = opendir(path.c_str());
		if(dir)
			closedir(dir);
		else if(-1 == mkdir(path.c_str(),0755))
		{
			//lets create the service folder (for first time)
			std::stringstream ss;
			ss << __COUT_HDR_FL__ << "Service directory creation failed: " <<
					path << std::endl;
			std::cout << ss;
			throw std::runtime_error(ss.str());
		}

		path = LOGBOOK_PATH + LOGBOOK_UPLOADS_PATH;
		dir = opendir(path.c_str());
		if(dir)
			closedir(dir);
		else if(-1 == mkdir((path).c_str(),0755))
		{
			//lets create the service folder (for first time)
			std::stringstream ss;
			ss << __COUT_HDR_FL__ << "Service directory creation failed: " <<
					path <<  std::endl;
			std::cout << ss;
			throw std::runtime_error(ss.str());
		}

		path = LOGBOOK_PATH + LOGBOOK_LOGBOOKS_PATH;
		dir = opendir(path.c_str());
		if(dir)
			closedir(dir);
		else if(-1 == mkdir(path.c_str(),0755))
		{
			//lets create the service folder (for first time)
			std::stringstream ss;
			ss << __COUT_HDR_FL__ << "Service directory creation failed: " <<
					path << std::endl;
			std::cout << ss;
			throw std::runtime_error(ss.str());
		}
	}

	getActiveExperiment();	//init active experiment
	__MOUT__ << "Active Experiment is " << activeExperiment_ << std::endl;
	mostRecentDayIndex_ = 0;


	//start mf msg listener
	//std::thread([](){ LogbookSupervisor::MFReceiverWorkLoop(); }).detach();
}

//========================================================================================================================
void LogbookSupervisor::destroy(void)
{
	//called by destructor

}
//
//#include "otsdaq-core/NetworkUtilities/ReceiverSocket.h"
//#define MF_POS_OF_MSG	11
////========================================================================================================================
//void LogbookSupervisor::MFReceiverWorkLoop()
//{
//	__MOUT__ << std::endl;
//	ReceiverSocket rsock("127.0.0.1",30001);
//	rsock.initialize();
//
//	//	int receive(std::string& buffer, unsigned int timeoutSeconds=1, unsigned int timeoutUSeconds=0);
//	std::string buffer;
//	int i,p;
//	while(1)
//	{
//		//if receive succeeds display message
//		if(rsock.receive(buffer,1,0,false) != -1) //set to rcv quiet mode
//		{
//			//find position of message and save to p
//				//by jumping to the correct '|' marker
//			for(p=0,i=0;i<MF_POS_OF_MSG;++i)
//				p = buffer.find('|',p)+1;
//			std::cout << &buffer[p] << std::endl;
//		}
//	}
//
//}

//========================================================================================================================
void LogbookSupervisor::Default(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{

	__MOUT__ << " active experiment " << activeExperiment_ << std::endl;
	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/Logbook.html?urn=" <<
			this->getApplicationDescriptor()->getLocalId() << "&active_experiment=" << activeExperiment_ << "'></frameset></html>";
}

//========================================================================================================================
//xoap::MakeSystemLogbookEntry
//	make a system logbook entry into active experiment's logbook from Supervisor only
//	TODO: (how to enforce?)
xoap::MessageReference LogbookSupervisor::MakeSystemLogbookEntry (xoap::MessageReference msg)
throw (xoap::exception::Exception)
{
	SOAPParameters parameters("EntryText");
	//	SOAPParametersV parameters(1);
	//	parameters[0].setName("EntryText");
	receive(msg, parameters);
	std::string EntryText = parameters.getValue("EntryText");

	__MOUT__ << "Received External Supervisor System Entry " << EntryText << std::endl;
	__MOUT__ << "Active Experiment is  " << activeExperiment_ << std::endl;

	std::string retStr = "Success";


	std::string logPath, logDirPath = (std::string)LOGBOOK_PATH + (std::string)LOGBOOK_LOGBOOKS_PATH +
			(std::string)LOGBOOK_EXPERIMENT_DIR_PREFACE + activeExperiment_;


	char dayIndexStr[20];
	HttpXmlDocument logXml;
	char fileIndex[40];
	xercesc::DOMElement* entryEl;
	DIR *dir;

	if(activeExperiment_ == "")
	{
		retStr = "Warning - Currently, no Active Experiment.";
		__MOUT__ << retStr << std::endl;
		goto XOAP_CLEANUP;
	}

	//check that directory exists
	dir = opendir(logDirPath.c_str());
	if(!dir)
	{
		retStr = "Error - Active Experiment directory missing.";
		__MOUT__ << retStr << std::endl;
		goto XOAP_CLEANUP;
	}
	closedir(dir);

	sprintf(dayIndexStr,"%6.6lu",time(0)/(60*60*24)); //get today's index

	logPath = logDirPath + "/" + LOGBOOK_FILE_PREFACE + activeExperiment_ + "_" + (std::string)dayIndexStr + LOGBOOK_FILE_EXTENSION;
	__MOUT__ << "logPath " << logPath << std::endl;

	logXml.loadXmlDocument(logPath);    //NOTE: on failure, no need to do anything
	//because empty XML file is valid structure
	//entry structure:
	//  <XML_LOGBOOK_ENTRY>
	//		<XML_LOGBOOK_ENTRY_TIME>
	//		<XML_LOGBOOK_ENTRY_CREATOR>
	//      <XML_LOGBOOK_ENTRY_TEXT>
	//      <XML_LOGBOOK_ENTRY_FILE value=fileType0>
	//      <XML_LOGBOOK_ENTRY_FILE value=fileType1> ...
	//  </XML_LOGBOOK_ENTRY>

	entryEl = logXml.addTextElementToData(XML_LOGBOOK_ENTRY);

	sprintf(fileIndex,"%lu_%lu",time(0),clock()); //create unique time label for entry time(0)_clock()
	logXml.addTextElementToParent(XML_LOGBOOK_ENTRY_TIME, fileIndex, entryEl);
	logXml.addTextElementToParent(XML_LOGBOOK_ENTRY_CREATOR, "SYSTEM LOG", entryEl);
	logXml.addTextElementToParent(XML_LOGBOOK_ENTRY_TEXT, EntryText, entryEl);
	logXml.addTextElementToParent(XML_LOGBOOK_ENTRY_SUBJECT, "System Log", entryEl);

	logXml.saveXmlDocument(logPath);

	XOAP_CLEANUP:

	//fill return parameters
	SOAPParameters retParameters("Status",retStr);
	//	SOAPParametersV retParameters(1);
	//	retParameters[0].setName("Status");
	//	retParameters[0].setValue(retStr);

	return SOAPUtilities::makeSOAPMessageReference("LogbookEntryStatusResponse",retParameters);
}

//========================================================================================================================
//LogImage
//	Since xdaq's headers are wrong for images, browsers get confused if not wrapped in an html page.
//	This function wraps an uploaded logbook entry image at src for display to user.
void LogbookSupervisor::LogImage(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	cgicc::Cgicc cgi(in);
	std::string src = CgiDataUtilities::getData(cgi,"src");
	__MOUT__ << " Get Log Image " << src << std::endl;
	*out << "<!DOCTYPE HTML><html lang='en'><frameset col='100%' row='100%'><frame src='/WebPath/html/LogbookImage.html?urn=" <<
			this->getApplicationDescriptor()->getLocalId() << "&src=" << src << "'></frameset></html>";
}

//========================================================================================================================
//LogReport
//	Gives controls for generating a logbook report
//	NOTE: to create pdf with command line:
//			paps LogbookData/experiment_list.xml > test.ps
//			ps2pdfwr test.ps test.pdf
void LogbookSupervisor::LogReport(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	cgicc::Cgicc cgi(in);
	std::string activeExperiment = CgiDataUtilities::getData(cgi,"activeExperiment");
	__MOUT__ << " Start Log Report for " << activeExperiment << std::endl;
	*out << "<!DOCTYPE HTML><html lang='en'><header><title>ots Logbook Reports</title></header><frameset col='100%' row='100%'><frame src='/WebPath/html/LogbookReport.html?urn=" <<
			this->getApplicationDescriptor()->getLocalId() << "&activeExperiment=" << activeExperiment << "'></frameset></html>";
}

//========================================================================================================================
// getActiveExperiment
// 		load active experiment from txt file, must be first line in file
std::string LogbookSupervisor::getActiveExperiment()
{
	FILE *fp = fopen(std::string((std::string)ACTIVE_EXPERIMENT_PATH).c_str(),"r");
	if(!fp)	activeExperiment_ = "";
	else
	{
		char line[100];
		if(!fgets(line,100,fp)) line[0] = '\0'; //if null returned, file is empty and line is untouched, so touch.
		fclose(fp);

		//remove \n \r
		if(line[strlen(line)-2] == '\r')
			line[strlen(line)-2] = '\0';
		else if(line[strlen(line)-1] == '\n')
			line[strlen(line)-1] = '\0';

		activeExperiment_ = line;
	}

	return activeExperiment_;
}

//========================================================================================================================
// setActiveExperiment
//		"" means no experiment is active
void LogbookSupervisor::setActiveExperiment(std::string experiment)
{
	FILE *fp = fopen(std::string((std::string)ACTIVE_EXPERIMENT_PATH).c_str(),"w");
	if(!fp)
	{
		__MOUT__ << "FATAL ERROR!!! - file write" << std::endl;
		return;
	}

	fprintf(fp,"%s",experiment.c_str());
	fclose(fp);

	if(activeExperiment_ != "" && activeExperiment_ != experiment) //old active experiment is on its way out
		theRemoteWebUsers_.makeSystemLogbookEntry(theSupervisorsConfiguration_.getSupervisorDescriptor(),"Experiment was made inactive."); //make system logbook entry

	bool entryNeeded = false;
	if(experiment != "" && activeExperiment_ != experiment) //old active experiment is on its way out
		entryNeeded = true;

	activeExperiment_ = experiment;
	__MOUT__ << "Active Experiment set to " << activeExperiment_ << std::endl;

	if(entryNeeded)
		theRemoteWebUsers_.makeSystemLogbookEntry(theSupervisorsConfiguration_.getSupervisorDescriptor(),"Experiment was made active."); //make system logbook entry

}

//========================================================================================================================
// validateExperimentName
//		remove all chars that are not alphanumeric, dashes, or underscores
bool LogbookSupervisor::validateExperimentName(std::string &exp)
{
	if(exp.length() < EXPERIMENT_NAME_MIN_LENTH || exp.length() > EXPERIMENT_NAME_MAX_LENTH) return false;
	for(int i=0;i<(int)exp.length();++i)
		if(!(
				(exp[i] >= 'a' &&  exp[i] <= 'z') ||
				(exp[i] >= 'A' &&  exp[i] <= 'Z') ||
				(exp[i] >= '0' &&  exp[i] <= '9') ||
				(exp[i] == '-' ||  exp[i] == '_') ) )
		{    exp = exp.substr(0,i) + exp.substr(i+1); --i;	} //remove illegal chars and rewind i

	return true;
}

//========================================================================================================================
// getExperiments
//		if xmldoc, then output experiments to xml
//		if out, then output to stream
void LogbookSupervisor::getExperiments(HttpXmlDocument *xmldoc, std::ostringstream *out)
{
	//check that experiment listing doesn't already exist
	HttpXmlDocument expXml;
	if(!expXml.loadXmlDocument((std::string)LOGBOOK_EXPERIMENT_LIST_PATH))
	{
		__MOUT__ << "Fatal Error - Experiment database." << std::endl;
		__MOUT__ << "Creating empty experiment database." << std::endl;

		expXml.addTextElementToData((std::string)XML_EXPERIMENTS_ROOT);
		expXml.saveXmlDocument((std::string)LOGBOOK_EXPERIMENT_LIST_PATH);
		return;
	}

	std::vector<std::string> exps;
	expXml.getAllMatchingValues(XML_EXPERIMENT,exps);

	if(xmldoc)	xmldoc->addTextElementToData(XML_ACTIVE_EXPERIMENT, activeExperiment_);

	for(unsigned int i=0;i<exps.size();++i) //loop experiments
	{
		if(xmldoc)	xmldoc->addTextElementToData(XML_EXPERIMENT, exps[i]);
		if(out)		*out << exps[i] << std::endl;
	}
}

//========================================================================================================================
// createExperiment
void LogbookSupervisor::createExperiment(std::string experiment, std::string creator, HttpXmlDocument *xmldoc)
{
	if(!validateExperimentName(experiment))
	{
		if(xmldoc) xmldoc->addTextElementToData(XML_ADMIN_STATUS,"Error - Experiment name must be 3-25 characters.");
		return;
	}

	__MOUT__ << "experiment " << experiment << std::endl;

	//check that directory doesn't already exist
	std::string dirPath = (std::string)LOGBOOK_PATH + (std::string)LOGBOOK_LOGBOOKS_PATH +
			(std::string)LOGBOOK_EXPERIMENT_DIR_PREFACE + experiment;

	__MOUT__ << "dirPath " << dirPath << std::endl;

	bool directoryExists = false;
	DIR *dir = opendir(dirPath.c_str());
	if(dir)
	{
		closedir(dir);
		directoryExists = true;
	}

	//check that experiment listing doesn't already exist
	HttpXmlDocument expXml;
	if(!expXml.loadXmlDocument((std::string)LOGBOOK_EXPERIMENT_LIST_PATH))
	{
		if(xmldoc) xmldoc->addTextElementToData(XML_ADMIN_STATUS,"Fatal Error - Experiment database.");
		return;
	}

	std::vector<std::string> exps;
	expXml.getAllMatchingValues(XML_EXPERIMENT,exps);

	for(unsigned int i=0;i<exps.size();++i)
		if(experiment == exps[i])
		{
			if(xmldoc) xmldoc->addTextElementToData(XML_ADMIN_STATUS,"Failed - Experiment, " + experiment + ", already exists.");
			return;
		}
	__MOUT__ << "experiments count: " << exps.size() << std::endl;


	//everything checks out, add experiment!
	//add to experiments xml doc and save
	//	<experiments>
	//		...
	//			<experiment_name = "xx">
	//				<create_time = "##"> <who_created = "aa">
	xercesc::DOMElement* expEl = expXml.addTextElementToParent(XML_EXPERIMENT, experiment, XML_EXPERIMENTS_ROOT);
	char createTime[20];
	sprintf(createTime,"%lu",time(0));
	expXml.addTextElementToParent(XML_EXPERIMENT_CREATE, createTime, expEl);
	expXml.addTextElementToParent(XML_EXPERIMENT_CREATOR, creator, expEl);
	expXml.saveXmlDocument((std::string)LOGBOOK_EXPERIMENT_LIST_PATH);

	//create directory only if doesn't already exist
	if(directoryExists)
	{

		//check uploads folder
		dirPath += "/" + (std::string)LOGBOOK_UPLOADS_PATH;
		__MOUT__ << "Checking uploads directory" << std::endl;

		directoryExists = false;
		dir = opendir(dirPath.c_str());
		if(!dir) //check if uploads directory exists within experiment directory
		{
			__MOUT__ << "Creating uploads directory" << std::endl;
			if(-1 == mkdir(dirPath.c_str(),0755)) //make uploads directory
			{
				if(xmldoc) xmldoc->addTextElementToData(XML_ADMIN_STATUS,"Failed - uploads directory for " + experiment + " was not created.");
				__MOUT__ << "Uploads directory failure." << std::endl;
				return;
			}
		}
		else
			closedir(dir);

		xmldoc->addTextElementToData(XML_ADMIN_STATUS,"Directory already exists for " + experiment +
				", re-added to list of experiments.");
		return;
	}
	__MOUT__ << "Creating experiment and uploads directory at: " <<
			dirPath << std::endl;
	if(-1 == mkdir(dirPath.c_str(),0755) ||
			-1 == mkdir((dirPath + "/" +
					(std::string)LOGBOOK_UPLOADS_PATH).c_str(),0755))
	{
		if(xmldoc) xmldoc->addTextElementToData(XML_ADMIN_STATUS,"Failed - directory, " + experiment + ", could not be created.");
		return;
	}

	if(xmldoc) xmldoc->addTextElementToData(XML_ADMIN_STATUS,"Experiment, " + experiment + ", successfully created.");
}

//========================================================================================================================
// webUserSetActiveExperiment
//		if experiment exists, set as active
//		to clear active experiment set to ""
void LogbookSupervisor::webUserSetActiveExperiment(std::string experiment, HttpXmlDocument *xmldoc)
{
	if(experiment == "") //clear active experiment
	{
		setActiveExperiment(experiment);
		if(xmldoc) xmldoc->addTextElementToData(XML_ADMIN_STATUS,"Active experiment cleared successfully.");
	}

	//check that experiment listing exists
	HttpXmlDocument expXml;
	if(!expXml.loadXmlDocument((std::string)LOGBOOK_EXPERIMENT_LIST_PATH))
	{
		if(xmldoc) xmldoc->addTextElementToData(XML_ADMIN_STATUS,"Fatal Error - Experiment database.");
		return;
	}
	std::vector<std::string> exps;
	expXml.getAllMatchingValues(XML_EXPERIMENT,exps);

	unsigned int i;
	for(i=0;i<exps.size();++i)
		if(experiment == exps[i]) break;

	if(i == exps.size()) //not found
	{
		if(xmldoc) xmldoc->addTextElementToData(XML_ADMIN_STATUS,"Failed - Experiment, " + experiment + ", not found.");
		return;
	}

	//found!
	setActiveExperiment(experiment);
	if(xmldoc) xmldoc->addTextElementToData(XML_ADMIN_STATUS,"Active experiment set to " + experiment + " successfully.");
}

//========================================================================================================================
// removeExperiment
//		remove experiment from listing only (do NOT remove logbook data directory)
//		record remover in log file REMOVE_EXPERIMENT_LOG_PATH
void LogbookSupervisor::removeExperiment(std::string experiment, std::string remover, HttpXmlDocument *xmldoc)
{
	__MOUT__ << "experiment " << experiment << std::endl;

	//check that experiment listing exists
	HttpXmlDocument expXml;
	if(!expXml.loadXmlDocument((std::string)LOGBOOK_EXPERIMENT_LIST_PATH))
	{
		if(xmldoc) xmldoc->addTextElementToData(XML_ADMIN_STATUS,"Fatal Error - Experiment database.");
		return;
	}
	std::vector<std::string> exps;
	expXml.getAllMatchingValues(XML_EXPERIMENT,exps);

	unsigned int i;
	for(i=0;i<exps.size();++i)
		if(experiment == exps[i]) break;

	if(i == exps.size()) //not found
	{
		if(xmldoc) xmldoc->addTextElementToData(XML_ADMIN_STATUS,"Failed - Experiment, " + experiment + ", not found.");
		return;
	}

	//found!

	//remove experiment from xml
	xercesc::DOMElement* parent = expXml.getMatchingElement(XML_EXPERIMENTS_ROOT);
	xercesc::DOMElement* child = expXml.getMatchingElement(XML_EXPERIMENT,i);
	__MOUT__ << "experiments original count: " << expXml.getChildrenCount(parent) << std::endl;
	expXml.recursiveRemoveChild(child, parent);
	__MOUT__ << "experiments new count: " << expXml.getChildrenCount(parent) << std::endl;

	//update removed experiments log
	FILE *fp = fopen(((std::string)REMOVE_EXPERIMENT_LOG_PATH).c_str(),"a");
	if(!fp)
	{
		if(xmldoc) xmldoc->addTextElementToData(XML_ADMIN_STATUS,"Fatal Error - Remove log.");
		return;
	}
	fprintf(fp,"%s -- %s Experiment removed by %s.\n",asctime(localtime(&((time_t const&)(time(0))))),
			experiment.c_str(), remover.c_str());
	fclose(fp);

	expXml.saveXmlDocument((std::string)LOGBOOK_EXPERIMENT_LIST_PATH); //save database

	//unset from activeExperiment_ if is active experiment
	if(activeExperiment_ == experiment)
		setActiveExperiment(); //clear active experiment

	if(xmldoc) xmldoc->addTextElementToData(XML_ADMIN_STATUS,"Experiment, " + experiment + ", successfully removed.");
}

//========================================================================================================================
//	refreshLogbook
//		returns all the logbook data for active experiment from starting date and back in time for
//			duration total number of days.
//		e.g. data = today, and duration = 1 returns logbook for today from active experiment
//		The entries are returns from oldest to newest
void LogbookSupervisor::refreshLogbook(time_t date, unsigned char duration,
		HttpXmlDocument *xmldoc, std::ostringstream *out, std::string experiment)
{
	if(experiment == "") experiment = activeExperiment_; //default to active experiment
	if(xmldoc) xmldoc->addTextElementToData(XML_ACTIVE_EXPERIMENT,experiment); //for success

	//check that directory exists
	std::string dirPath = (std::string)LOGBOOK_PATH + (std::string)LOGBOOK_LOGBOOKS_PATH +
			(std::string)LOGBOOK_EXPERIMENT_DIR_PREFACE + experiment;

	if(out) *out << __COUT_HDR_FL__ << "dirPath " << dirPath << std::endl;

	DIR *dir = opendir(dirPath.c_str());
	if(!dir)
	{
		if(xmldoc) xmldoc->addTextElementToData(XML_STATUS,"Error - Directory for experiment, " + experiment + ", missing.");
		if(out) *out << __COUT_HDR_FL__ << "Error - Directory missing" << std::endl;
		return;
	}

	unsigned int baseDay;

	if(!date) //if date is 0 take most recent day and update it
	{
		struct dirent *drnt;
		unsigned int extractedDay;
		int start, finish; //always find number after last _ and before last .

		mostRecentDayIndex_ = 0;
		while ((drnt=readdir(dir))){
			//if(out) *out << __COUT_HDR_FL__ << "dirContents " << drnt->d_name << std::endl;

			if(strcmp(&(drnt->d_name[strlen(drnt->d_name)-4]),".xml")) continue; //skip non logbook files

			for(finish=strlen(drnt->d_name)-1;finish>0;--finish)
				if(drnt->d_name[finish] == '.') break;
			if(finish == 0)
			{
				if(out) *out << __COUT_HDR_FL__ << "failed to find day index finish " << std::endl;
				return;
			}
			for(start=finish-1;start>0;--start)
				if(drnt->d_name[start-1] == '_') break;
			if(start == 0)
			{
				if(out) *out << __COUT_HDR_FL__ << "failed to find day index start " << std::endl;
				return;
			}
			drnt->d_name[finish] = '\0';
			extractedDay = atoi((char *)(&(drnt->d_name[start])));
			if(out) *out << __COUT_HDR_FL__ << "dirContents " << (char *)(&(drnt->d_name[start])) << " " << extractedDay << std::endl;
			if(!mostRecentDayIndex_ || mostRecentDayIndex_ < extractedDay) mostRecentDayIndex_ = extractedDay;
		}
		if(out) *out << __COUT_HDR_FL__ << "dirContents done, found most recent day: " << mostRecentDayIndex_ << std::endl;

		baseDay = mostRecentDayIndex_;
	}
	else
		baseDay = (date/(60*60*24));
	closedir(dir);

	std::string entryPath;
	char dayIndexStr[20];
	FILE *fp;

	//read all days selected out
	//	entries are in file as oldest at top, newest at bottom
	//	so read oldest files first to have global ordering of old to new
	for(unsigned char i=duration;i!=0;--i)
	{
		sprintf(dayIndexStr,"%6.6u",baseDay-i+1); //get day index, back in time
		entryPath = dirPath + "/" + LOGBOOK_FILE_PREFACE + experiment + "_" + (std::string)dayIndexStr + LOGBOOK_FILE_EXTENSION;

		if(out) *out << __COUT_HDR_FL__ << "Directory Entry " << entryPath << std::endl;

		fp = fopen(entryPath.c_str(),"r");
		if(!fp)
		{
			if(out) *out << __COUT_HDR_FL__ << "File not found" << std::endl;
			continue;
		}
		fclose(fp);

		//file found! read file out

		HttpXmlDocument logXml;
		if(!logXml.loadXmlDocument(entryPath))
		{
			if(xmldoc) xmldoc->addTextElementToData(XML_STATUS,"Critical Failure - log did not load. Notify admins.");
			if(out) *out << __COUT_HDR_FL__ << "Failure - log XML did not load" << std::endl;
			return;
		}

		if(xmldoc) xmldoc->copyDataChildren(logXml); //copy file to output xml
	}

	if(xmldoc) xmldoc->addTextElementToData(XML_STATUS,"1"); //for success
	if(out) *out << __COUT_HDR_FL__ << "Today: " << time(0)/(60*60*24)  << std::endl;

	sprintf(dayIndexStr,"%lu",time(0)/(60*60*24) - mostRecentDayIndex_);
	if(xmldoc) xmldoc->addTextElementToData(XML_MOST_RECENT_DAY,dayIndexStr); //send most recent day index
}

//========================================================================================================================
//	cleanUpPreviews
//      cleanup logbook preview directory
//      all names have time_t creation time + "_" + incremented index
void LogbookSupervisor::cleanUpPreviews()
{
	std::string previewPath = (std::string)LOGBOOK_PATH + (std::string)LOGBOOK_PREVIEWS_PATH;

	DIR *dir = opendir(previewPath.c_str());
	if(!dir)
	{
		__MOUT__ << "Error - Previews directory missing: " << previewPath << std::endl;
		return;
	}

	struct dirent *entry;
	time_t dirCreateTime;
	unsigned int i;

	while((entry = readdir(dir))) //loop through all entries in directory and remove anything expired
	{
		if( strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0
				&& strcmp(entry->d_name, ".svn") != 0 )
		{
			//replace _ with space so sscanf works
			for(i=0;i<strlen(entry->d_name);++i)
				if(entry->d_name[i] == '_')
				{	entry->d_name[i] = ' '; break; 	}
			sscanf(entry->d_name,"%li",&dirCreateTime);

			if((time(0) - dirCreateTime) > LOGBOOK_PREVIEW_EXPIRATION_TIME)
			{
				__MOUT__ << "Expired" << std::endl;

				entry->d_name[i] = '_'; //put _ back

				__MOUT__ << "rm -rf " << previewPath + (std::string)entry->d_name <<  std::endl << std::endl;
				system(((std::string)("rm -rf " + previewPath + (std::string)entry->d_name)).c_str());
			}
		}
	}

	closedir(dir);
}


//========================================================================================================================
//	savePostPreview
//      save post to preview directory named with time and incremented index
void LogbookSupervisor::savePostPreview(std::string &subject, std::string &text, const std::vector<cgicc::FormFile> &files, std::string creator,
		HttpXmlDocument *xmldoc)
{
	if(activeExperiment_ == "") //no active experiment!
	{
		if(xmldoc) xmldoc->addTextElementToData(XML_STATUS,"Failed - no active experiment currently!");
		return;
	}

	char fileIndex[40];
	sprintf(fileIndex,"%lu_%lu",time(0),clock()); //create unique time label for entry time(0)_clock()
	std::string previewPath = (std::string)LOGBOOK_PATH + (std::string)LOGBOOK_PREVIEWS_PATH + (std::string)fileIndex;

	__MOUT__ << "previewPath " << previewPath << std::endl;
	if(-1 == mkdir(previewPath.c_str(),0755))
	{
		if(xmldoc) xmldoc->addTextElementToData(XML_STATUS,"Failed - preview could not be generated.");
		return;
	}

	//new directory created successfully, save text and files
	//entry structure:
	//  <XML_LOGBOOK_ENTRY>
	//		<XML_LOGBOOK_ENTRY_TIME>
	//		<XML_LOGBOOK_ENTRY_CREATOR>
	//      <XML_LOGBOOK_ENTRY_SUBJECT>
	//      <XML_LOGBOOK_ENTRY_TEXT>
	//      <XML_LOGBOOK_ENTRY_FILE value=fileType0>
	//      <XML_LOGBOOK_ENTRY_FILE value=fileType1> ...
	//  </XML_LOGBOOK_ENTRY>

	escapeLogbookEntry(text);
	escapeLogbookEntry(subject);
	__MOUT__ << "~~subject " << subject << std::endl << "~~text " << text << std::endl << std::endl;

	HttpXmlDocument previewXml;

	previewXml.addTextElementToData(XML_LOGBOOK_ENTRY);
	previewXml.addTextElementToParent(XML_LOGBOOK_ENTRY_TIME, fileIndex, XML_LOGBOOK_ENTRY);
	if(xmldoc) xmldoc->addTextElementToData(XML_LOGBOOK_ENTRY_TIME,fileIndex); //return time
	previewXml.addTextElementToParent(XML_LOGBOOK_ENTRY_CREATOR, creator, XML_LOGBOOK_ENTRY);
	if(xmldoc) xmldoc->addTextElementToData(XML_LOGBOOK_ENTRY_CREATOR,creator); //return creator
	previewXml.addTextElementToParent(XML_LOGBOOK_ENTRY_TEXT, text, XML_LOGBOOK_ENTRY);
	if(xmldoc) xmldoc->addTextElementToData(XML_LOGBOOK_ENTRY_TEXT,text); //return text
	previewXml.addTextElementToParent(XML_LOGBOOK_ENTRY_SUBJECT, subject, XML_LOGBOOK_ENTRY);
	if(xmldoc) xmldoc->addTextElementToData(XML_LOGBOOK_ENTRY_SUBJECT,subject); //return subject

	__MOUT__ << "file size " << files.size() << std::endl;

	std::string filename;
	std::ofstream myfile;
	for (unsigned int i=0; i<files.size(); ++i)
	{

		previewXml.addTextElementToParent(XML_LOGBOOK_ENTRY_FILE, files[i].getDataType(), XML_LOGBOOK_ENTRY);
		if(xmldoc) xmldoc->addTextElementToData(XML_LOGBOOK_ENTRY_FILE,files[i].getDataType()); //return file type

		if((filename = validateUploadFileType(files[i].getDataType())) == "") //invalid file type
		{
			if(xmldoc) xmldoc->addTextElementToData(XML_STATUS,"Failed - invalid file type, " +
					files[i].getDataType() + ".");
			return;
		}

		//file validated, so save upload to temp directory
		sprintf(fileIndex,"%d",i);
		filename = previewPath + "/" + (std::string)LOGBOOK_PREVIEW_UPLOAD_PREFACE +
				(std::string)fileIndex + "." + filename;

		__MOUT__ << "file " << i << " - " << filename << std::endl;
		myfile.open(filename.c_str());
		if (myfile.is_open())
		{
			files[i].writeToStream(myfile);
			myfile.close();
		}
	}

	//save xml doc for preview entry
	previewXml.saveXmlDocument(previewPath + "/" + (std::string)LOGBOOK_PREVIEW_FILE);

	if(xmldoc) xmldoc->addTextElementToData(XML_STATUS,"1"); //1 indicates success!
	if(xmldoc) xmldoc->addTextElementToData(XML_PREVIEW_INDEX,"1"); //1 indicates is a preview post
}

//========================================================================================================================
//	movePreviewEntry
//      if approve
//          move entry to current active logbook
//      if not approve
//          delete directory
void LogbookSupervisor::movePreviewEntry(std::string previewNumber, bool approve,
		HttpXmlDocument *xmldoc)
{

	__MOUT__ << "previewNumber " << previewNumber << (approve?" Accepted":" Cancelled") << std::endl;

	std::string sysCmd, previewPath = (std::string)LOGBOOK_PATH + (std::string)LOGBOOK_PREVIEWS_PATH + previewNumber;

	if (approve) {
		//move from preview to logbook

		HttpXmlDocument previewXml;
		previewXml.loadXmlDocument(previewPath + "/" + (std::string)LOGBOOK_PREVIEW_FILE);

		std::string logPath, logDirPath = (std::string)LOGBOOK_PATH + (std::string)LOGBOOK_LOGBOOKS_PATH +
				(std::string)LOGBOOK_EXPERIMENT_DIR_PREFACE + activeExperiment_;

		//check that directory exists
		DIR *dir = opendir(logDirPath.c_str());
		if(!dir)
		{
			__MOUT__ << "Error - Active Experiment directory missing: " << logPath << std::endl;
			return;
		}
		closedir(dir);

		char dayIndexStr[20];
		sprintf(dayIndexStr,"%6.6lu",time(0)/(60*60*24)); //get today's index

		logPath = logDirPath + "/" + LOGBOOK_FILE_PREFACE + activeExperiment_ + "_" + (std::string)dayIndexStr + LOGBOOK_FILE_EXTENSION;
		__MOUT__ << "logPath " << logPath << std::endl;

		HttpXmlDocument logXml;
		logXml.loadXmlDocument(logPath);    //NOTE: on failure, no need to do anything
		//because empty XML file is valid structure
		//entry structure:
		//  <XML_LOGBOOK_ENTRY>
		//		<XML_LOGBOOK_ENTRY_TIME>
		//		<XML_LOGBOOK_ENTRY_CREATOR>
		//      <XML_LOGBOOK_ENTRY_TEXT>
		//      <XML_LOGBOOK_ENTRY_FILE value=fileType0>
		//      <XML_LOGBOOK_ENTRY_FILE value=fileType1> ...
		//  </XML_LOGBOOK_ENTRY>

		logXml.copyDataChildren(previewXml);	//Copy from previewXML to logXML
		logXml.saveXmlDocument(logPath);

		//Move upload files
		std::vector<std::string> fileTypes;
		previewXml.getAllMatchingValues(XML_LOGBOOK_ENTRY_FILE,fileTypes);
		std::string entryTimeLabel = previewXml.getMatchingValue(XML_LOGBOOK_ENTRY_TIME);
		std::string fileExtension, previewFilename, logFilename;
		char fileIndex[10];
		for(unsigned int i=0;i<fileTypes.size();++i)
		{
			if((fileExtension = validateUploadFileType(fileTypes[i])) == "") //invalid file type
			{
				__MOUT__ << "Failed - invalid file type: " << fileTypes[i] << std::endl;
				continue;
			}

			//file validated, so save upload to temp directory
			sprintf(fileIndex,"%d",i);
			previewFilename =  (std::string)LOGBOOK_PREVIEW_UPLOAD_PREFACE + (std::string)fileIndex + "." + fileExtension;
			logFilename =  (std::string)LOGBOOK_PREVIEW_UPLOAD_PREFACE + entryTimeLabel + "_" +
					(std::string)fileIndex + "." + fileExtension;

			sysCmd =  "mv " + (previewPath + "/" + previewFilename) + " " +
					(logDirPath + "/" + (std::string)LOGBOOK_UPLOADS_PATH + logFilename);
			__MOUT__ << sysCmd << std::endl;
			system(sysCmd.c_str());
		}
	}

	//remove preview directory
	sysCmd = "rm -rf " + previewPath;
	__MOUT__ << sysCmd <<  std::endl << std::endl;
	system(sysCmd.c_str());
}

//========================================================================================================================
//	validateUploadFileType
//      returns "" if file type is invalide, else returns file extension to use
std::string LogbookSupervisor::validateUploadFileType(const std::string fileType)
{    
	for (unsigned int i=0; i<allowedFileUploadTypes_.size(); ++i)
		if (allowedFileUploadTypes_[i] == fileType)
			return matchingFileUploadTypes_[i];        //found and done

	return ""; //not valid, return ""
}

//========================================================================================================================
//	escapeLogbookEntry
//      replace html/xhtml reserved characters with equivalent.
//      reserved: ", ', &, <, >, \n, double-space
void LogbookSupervisor::escapeLogbookEntry(std::string &entry)
{
	//NOTE: should already be taken care of by web gui javascript! do we care to check?

}

//========================================================================================================================
//	hideLogbookEntry
//		NOTE: does not actually delete entry, just marks as hidden
//      removes/restores logbook entry. Requires admin priveleges
//		Locates the entry within the active experiment and if hide
//			appends xml fields:
//				XML_LOGBOOK_ENTRY_HIDDEN
//				XML_LOGBOOK_ENTRY_HIDER
//				XML_LOGBOOK_ENTRY_HIDDEN_TIME
void LogbookSupervisor::hideLogbookEntry(const std::string &entryId, bool hide,const std::string &hider)
{
	__MOUT__ << "Hide=" << hide << " for entryid " << entryId << std::endl;

	//get path to entries file for entry at entryId
	char dayIndexStr[20];
	unsigned int i;
	for(i=0;i<entryId.length();++i)
		if(entryId[i] == '_') { dayIndexStr[i] = '\0'; break;}
		else
			dayIndexStr[i]  =  entryId[i];
	time_t days;
	sscanf(dayIndexStr,"%li",&days); //get seconds
	days /= 60*60*24; //get days
	sprintf(dayIndexStr,"%6.6lu",days);

	std::string logDirPath = (std::string)LOGBOOK_PATH + (std::string)LOGBOOK_LOGBOOKS_PATH +
			(std::string)LOGBOOK_EXPERIMENT_DIR_PREFACE + activeExperiment_;
	std::string logPath = logDirPath + "/" + LOGBOOK_FILE_PREFACE + activeExperiment_ + "_" + (std::string)dayIndexStr + LOGBOOK_FILE_EXTENSION;

	__MOUT__ << "logPath=" << logPath << std::endl;

	//locate entry
	HttpXmlDocument logXml;
	if(!logXml.loadXmlDocument(logPath))
	{
		__MOUT__ << "Failure - log XML did not load" << std::endl;
		return;
	}

	std::vector<std::string> allEntryIds;
	logXml.getAllMatchingValues(XML_LOGBOOK_ENTRY_TIME,allEntryIds);
	for(i=0;i<allEntryIds.size();++i)
		if(allEntryIds[i] == entryId) break;
	if(i == allEntryIds.size())
	{
		__MOUT__ << "Failure - entry not found" << std::endl;
		return;
	}

	__MOUT__ << "found " << logXml.getMatchingValue(XML_LOGBOOK_ENTRY_TEXT,i) << std::endl;

	xercesc::DOMElement* hiddenParentEl, *entryParentEl = logXml.getMatchingElement(XML_LOGBOOK_ENTRY,i); //get entry element

	//check if already hidden
	hiddenParentEl = logXml.getMatchingElementInSubtree(entryParentEl,XML_LOGBOOK_ENTRY_HIDDEN);

	if(hide) //remove entry
	{
		if(hiddenParentEl)
		{
			__MOUT__ << "Hidden tag already applied to entry." << std::endl;
			return;
		}
		hiddenParentEl = logXml.addTextElementToParent(XML_LOGBOOK_ENTRY_HIDDEN,"1",entryParentEl); //add hidden parent with value "1"
		logXml.addTextElementToParent(XML_LOGBOOK_ENTRY_HIDER,hider,hiddenParentEl);		//hider
		sprintf(dayIndexStr,"%lu",time(0));
		logXml.addTextElementToParent(XML_LOGBOOK_ENTRY_HIDDEN_TIME,dayIndexStr,hiddenParentEl); //hide time
	}
	else  //restore entry
	{
		if(!hiddenParentEl)
		{
			__MOUT__ << "Entry already was not hidden." << std::endl;
			return;
		}

		logXml.recursiveRemoveChild(hiddenParentEl,entryParentEl); //remove hidden parent
	}
	logXml.saveXmlDocument(logPath);
	__MOUT__ << "Success." << std::endl;
}
//========================================================================================================================
//	Log
//		Handles Web Interface requests to Logbook supervisor.
//		Does not refresh cookie for automatic update checks.
void LogbookSupervisor::Log(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	cgicc::Cgicc cgi(in);
	std::string Command;
	if((Command = CgiDataUtilities::postData(cgi,"RequestType")) == "")
		Command = cgi("RequestType"); //get command from form, if PreviewEntry

	__MOUT__ << "Command " << Command << " files: " << cgi.getFiles().size() << std::endl;

	//Commands
	//CreateExperiment
	//RemoveExperiment
	//GetExperimentList
	//SetActiveExperiment
	//RefreshLogbook
	//PreviewEntry
	//ApproveEntry
	//AdminRemoveRestoreEntry

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
		bool requireLock = false;

		if(!theRemoteWebUsers_.xmlLoginGateway(
				cgi,out,&xmldoc,theSupervisorsConfiguration_,
				&userPermissions,  		//acquire user's access level (optionally null pointer)
				!automaticCommand,			//true/false refresh cookie code
				1, //set access level requirement to pass gateway
				checkLock,					//true/false enable check that system is unlocked or this user has the lock
				requireLock,				//true/false requires this user has the lock to proceed
				0,//&userWithLock,			//acquire username with lock (optionally null pointer)
				(getUser?&user:0)				//acquire username of this user (optionally null pointer)
				,0//,&displayName			//acquire user's Display Name
				,&activeSessionIndex		//acquire user's session index associated with the cookieCode
				))
		{	//failure
			__MOUT__ << "Failed Login Gateway: " <<
					out->str() << std::endl; //print out return string on failure
			return;
		}
	}
	//**** end LOGIN GATEWAY CODE ***//


	//to report to logbook admin status use xmldoc.addTextElementToData(XML_ADMIN_STATUS,tempStr);

	if(Command == "CreateExperiment")
	{
		//check that  user is admin
		//check that experiment directory does not exist, and it is not in xml list
		//create experiment
		//create directory
		//add to experiments list

		if(userPermissions < ADMIN_PERMISSIONS_THRESHOLD)
		{
			xmldoc.addTextElementToData(XML_ADMIN_STATUS,"Error - Insufficient permissions.");
			goto CLEANUP;
		}

		//user is admin

		__MOUT__ << "Admin" << std::endl;

		//get creator name
		std::string creator = user;

		createExperiment(CgiDataUtilities::postData(cgi,"Experiment"), creator, &xmldoc);

		__MOUT__ << "Created" << std::endl;
	}
	else if(Command == "RemoveExperiment")
	{
		//check that  user is admin
		//remove from xml list, but do not remove directory (requires manual delete so mistakes aren't made)

		if(userPermissions < ADMIN_PERMISSIONS_THRESHOLD)
		{
			xmldoc.addTextElementToData(XML_ADMIN_STATUS,"Error - Insufficient permissions.");
			goto CLEANUP;
		}

		//get remover name
		std::string remover = user;
		removeExperiment(CgiDataUtilities::postData(cgi,"Experiment"), remover, &xmldoc);
	}
	else if(Command == "GetExperimentList")
	{
		//check that  user is admin
		//remove from xml list, but do not remove directory (requires manual delete so mistakes aren't made)

		if(userPermissions < ADMIN_PERMISSIONS_THRESHOLD)
		{
			xmldoc.addTextElementToData("is_admin","0"); //indicate not an admin
			goto CLEANUP;
		}
		xmldoc.addTextElementToData("is_admin","1"); //indicate not an admin

		getExperiments(&xmldoc);
	}
	else if(Command == "SetActiveExperiment")
	{
		//check that  user is admin
		//check that experiment exists
		//set active experiment

		if(userPermissions < ADMIN_PERMISSIONS_THRESHOLD)
		{
			xmldoc.addTextElementToData(XML_ADMIN_STATUS,"Error - Insufficient permissions.");
			goto CLEANUP;
		}

		webUserSetActiveExperiment(CgiDataUtilities::postData(cgi,"Experiment"), &xmldoc);
	}
	else if(Command == "RefreshLogbook")
	{
		//returns logbook for currently active experiment based on date and duration parameters

		std::string Date = CgiDataUtilities::postData(cgi,"Date");
		std::string Duration = CgiDataUtilities::postData(cgi,"Duration");

		time_t date;
		unsigned char duration;
		sscanf(Date.c_str(),"%li",&date);		//scan for unsigned long
		sscanf(Duration.c_str(),"%hhu",&duration); 	//scan for unsigned char

		__MOUT__ << "date " << date << " duration " << (int)duration << std::endl;
		std::stringstream str;
		refreshLogbook(date, duration, &xmldoc, (std::ostringstream *)&str);
		__MOUT__ << str.str() << std::endl;
	}
	else if(Command == "PreviewEntry")
	{
		//cleanup temporary folder
		//NOTE: all input parameters for PreviewEntry will be attached to form
		//	so use cgi(xxx) to get values.
		//increment number for each temporary preview, previewPostTempIndex_
		//save entry and uploads to previewPath / previewPostTempIndex_ /.

		cleanUpPreviews();
		std::string EntryText = cgi("EntryText");
		__MOUT__ << "EntryText " << EntryText <<  std::endl << std::endl;
		std::string EntrySubject = cgi("EntrySubject");
		__MOUT__ << "EntrySubject " << EntrySubject <<  std::endl << std::endl;

		//get creator name
		std::string creator = user;

		savePostPreview(EntrySubject,EntryText,cgi.getFiles(),creator,&xmldoc);
		//else xmldoc.addTextElementToData(XML_STATUS,"Failed - could not get username info.");
	}
	else if(Command == "ApproveEntry")
	{
		//If Approve = "1", then previewed Log entry specified by PreviewNumber
		//  is moved to logbook
		//Else the specified Log entry is deleted.
		std::string PreviewNumber = CgiDataUtilities::postData(cgi,"PreviewNumber");
		std::string Approve = CgiDataUtilities::postData(cgi,"Approve");

		movePreviewEntry(PreviewNumber,Approve=="1",&xmldoc);
	}
	else if(Command == "AdminRemoveRestoreEntry")
	{

		if(userPermissions < ADMIN_PERMISSIONS_THRESHOLD)
		{
			xmldoc.addTextElementToData(XML_ADMIN_STATUS,"Error - Insufficient permissions.");
			goto CLEANUP;
		}

		std::string EntryId = CgiDataUtilities::postData(cgi,"EntryId");
		bool Hide = CgiDataUtilities::postData(cgi,"Hide")=="1"?true:false;

		//get creator name
		std::string hider = user;

		hideLogbookEntry(EntryId,Hide,hider);

		xmldoc.addTextElementToData(XML_ADMIN_STATUS,"1"); //success
	}
	else
		__MOUT__ << "Command request not recognized." << std::endl;

	CLEANUP:

	//return xml doc holding server response
	xmldoc.outputXmlDocument((std::ostringstream*)out);
}









