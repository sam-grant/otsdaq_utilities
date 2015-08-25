#include "otsdaq-utilities/Visualization/VisualDataManager.h"

#include <iostream>
#include <sstream>
#include <cassert>

using namespace ots;


//========================================================================================================================
VisualDataManager::VisualDataManager(ConfigurationManager* configurationManager) :
        cProcessName_           ("VisualSupervisor"),
        theConfigurationManager_(configurationManager),
        theDataListener_        (0),
        theLiveDQMHistos_       (0),
        theFileDQMHistos_       ("FileDQMHistos")
{}

//========================================================================================================================
VisualDataManager::~VisualDataManager(void)
{}

//========================================================================================================================
void VisualDataManager::configure()
{
    DataManager::resetAllProcesses(); //Deletes all pointers created and given to the DataManager!

    //FIXME according to the configuration I will instantiate all the producers consumers I need
    //FIXME the name should be a property of the supervisors
    theDataListener_  = new DataListener (cProcessName_+"DataListener","192.168.133.1", 50002);
    theLiveDQMHistos_ = new DQMHistos    (cProcessName_+"LiveDQMHistos");


    DataManager::configureProcess<std::string>(cProcessName_, theDataListener_);
    DataManager::addConsumer                  (cProcessName_, theLiveDQMHistos_);

}

//========================================================================================================================
void VisualDataManager::halt(void)
{
    stop();
    DataManager::resetProcess(cProcessName_); //Deletes all pointers created and given to the DataManager!
    theLiveDQMHistos_ = 0;
}

//========================================================================================================================
void VisualDataManager::start(std::string runNumber)
{
    //FIXME according to the configuration I will give the right filename prefix
    std::stringstream fileName;
    fileName <<  "Run" << runNumber << "_Histos.root";
    std::cout << __PRETTY_FUNCTION__ << "Starting visualizing run " << runNumber << std::endl;
    //FIXME Maybe this has to go
    if(theLiveDQMHistos_ != 0)
        theLiveDQMHistos_->book(fileName.str(), theConfigurationManager_);

    DataManager::startProcess(cProcessName_);
}

//========================================================================================================================
void VisualDataManager::stop()
{
    DataManager::stopProcess(cProcessName_);
    if(theLiveDQMHistos_ != 0)
        theLiveDQMHistos_->save();
}

//========================================================================================================================
void VisualDataManager::load(std::string fileName, std::string type)
{
    if(type == "Histograms")
        theFileDQMHistos_.load(fileName);
    else if(type == "Monicelli")
        theMonicelliEventAnalyzer_.load(fileName);
    else if(type == "Geometry")
        theMonicelliGeometryConverter_.loadGeometry(fileName);

}

//========================================================================================================================
DQMHistos* VisualDataManager::getLiveDQMHistos(void)
{
    return theLiveDQMHistos_;
}

//========================================================================================================================
DQMHistos& VisualDataManager::getFileDQMHistos(void)
{
    return theFileDQMHistos_;
}

//========================================================================================================================
const Visual3DEvents& VisualDataManager::getVisual3DEvents(void)
{
    return theMonicelliEventAnalyzer_.getEvents();
}

//========================================================================================================================
const Visual3DGeometry& VisualDataManager::getVisual3DGeometry(void)
{
    return theMonicelliGeometryConverter_.getGeometry();
}
