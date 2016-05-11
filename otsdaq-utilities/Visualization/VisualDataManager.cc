#include "otsdaq-utilities/Visualization/VisualDataManager.h"
#include "otsdaq-core/DataManager/DataManager.h"
#include "otsdaq-core/DataManager/DataProcessor.h"
#include "otsdaq-core/ConfigurationPluginDataFormats/DataManagerConfiguration.h"
#include "otsdaq-core/ConfigurationInterface/ConfigurationManager.h"

#include <iostream>
#include <sstream>
#include <cassert>

using namespace ots;


//========================================================================================================================
VisualDataManager::VisualDataManager(std::string supervisorType, unsigned int supervisorInstance, ConfigurationManager* configurationManager)
: DataManager             (supervisorType, supervisorInstance, configurationManager)
, theConfigurationManager_(configurationManager)
, theLiveDQMHistos_       (0)
, theFileDQMHistos_       (supervisorType, supervisorInstance, "VisualBuffer", "FileDQMHistos")
{}

//========================================================================================================================
VisualDataManager::~VisualDataManager(void)
{}

//========================================================================================================================
void VisualDataManager::halt(void)
{
    stop();
//    DataManager::resetAllBuffers(); //Deletes all pointers created and given to the DataManager!
    DataManager::eraseAllBuffers(); //Deletes all pointers created and given to the DataManager!
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

    DataManager::startAllBuffers(runNumber);
}

//========================================================================================================================
void VisualDataManager::stop()
{
    DataManager::stopAllBuffers();
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
