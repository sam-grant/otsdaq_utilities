#include "otsdaq-utilities/Visualization/VisualDataManager.h"
#include "otsdaq-core/DataManager/DataManager.h"
#include "otsdaq-core/DataManager/DataStreamer.h"
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
, theDataListener_        (0)
, theLiveDQMHistos_       (0)
, theFileDQMHistos_       (supervisorType, supervisorInstance, 0, "FileDQMHistos")
{}

//========================================================================================================================
VisualDataManager::~VisualDataManager(void)
{}

//========================================================================================================================
void VisualDataManager::configure()
{
    DataManager::resetAllProcesses(); //Deletes all pointers created and given to the DataManager!

	const DataManagerConfiguration* dataManagerConfiguration = theConfigurationManager_->getConfiguration<DataManagerConfiguration>();
	auto processesList = dataManagerConfiguration->getListOfProcesses(supervisorType_, supervisorInstance_);
	for(const auto& itProcessID: processesList)
	{
		DataProcessor* aDataProcessor = nullptr;
		std::cout << __PRETTY_FUNCTION__ << "Process Name: " << itProcessID << std::endl;
		auto producersList = dataManagerConfiguration->getProducersList(supervisorType_, supervisorInstance_, itProcessID);
		for(const auto& itProducerName: producersList)
		{
			//std::cout << __PRETTY_FUNCTION__ << "Producer Name: " << *itProducers << std::endl;
			if(dataManagerConfiguration->getProducerStatus(supervisorType_, supervisorInstance_, itProcessID, itProducerName))
			{
				if(dataManagerConfiguration->getProducerType(supervisorType_, supervisorInstance_, itProcessID, itProducerName) == "DataListener")
				{
					//std::cout << __PRETTY_FUNCTION__ << "Producer Name NEW: " << *itProducers << std::endl;
					//                        DataManager::configureProcess<std::string>(cProcessName_+processName.str()
					//                        		, new DataListener (cProcessName_+processName.str()+"DataListener",
					//                                        dataManagerConfiguration->getProducerParameter(itProcessID,*itProducers,"IP"),
					//                                        strtoul(dataManagerConfiguration->getProducerParameter(itProcessID,*itProducers,"Port").c_str(),NULL,10)));

					aDataProcessor = new DataListener(
							supervisorType_,
							supervisorInstance_,
							itProcessID,
							itProducerName,
							strtoul(dataManagerConfiguration->getProducerParameter(supervisorType_, supervisorInstance_, itProcessID, itProducerName,"BufferSize").c_str(),NULL,10),
							dataManagerConfiguration->getProducerParameter(supervisorType_, supervisorInstance_, itProcessID, itProducerName, "IP"),
							strtoul(dataManagerConfiguration->getProducerParameter(supervisorType_, supervisorInstance_, itProcessID, itProducerName,"Port").c_str(),NULL,10));

				}
				aDataProcessor->registerToProcess();
			}
		}
		auto consumersList = dataManagerConfiguration->getConsumersList(supervisorType_, supervisorInstance_, itProcessID);
		for(const auto& itConsumerName: consumersList)
		{
			std::cout << __PRETTY_FUNCTION__ << "Consumer Name: " << itConsumerName << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "Consumer Type: " << dataManagerConfiguration->getConsumerType(supervisorType_, supervisorInstance_, itProcessID, itConsumerName) << std::endl;
			std::cout << __PRETTY_FUNCTION__ << "Consumer Type: " << dataManagerConfiguration->getConsumerStatus(supervisorType_, supervisorInstance_, itProcessID, itConsumerName) << std::endl;
			if(dataManagerConfiguration->getConsumerStatus(supervisorType_, supervisorInstance_, itProcessID, itConsumerName))
			{
				std::cout << __PRETTY_FUNCTION__ << "I SHOULD CREATE DQM!!!Consumer Name: " << dataManagerConfiguration->getConsumerType(supervisorType_, supervisorInstance_, itProcessID, itConsumerName) << std::endl;
				std::cout << __PRETTY_FUNCTION__ << "I SHOULD CREATE DQM!!!Consumer Name: " << dataManagerConfiguration->getConsumerType(supervisorType_, supervisorInstance_, itProcessID, itConsumerName) << std::endl;
				if(dataManagerConfiguration->getConsumerType(supervisorType_, supervisorInstance_, itProcessID, itConsumerName) == "DQMHistos")
				{
					std::cout << __PRETTY_FUNCTION__ << "CREATING DQM!!!Consumer Name: " << itConsumerName << std::endl;
					std::cout << __PRETTY_FUNCTION__ << "CREATING DQM!!!Consumer Name: " << itConsumerName << std::endl;
					std::cout << __PRETTY_FUNCTION__ << "CREATING DQM!!!Consumer Name: " << itConsumerName << std::endl;
					std::cout << __PRETTY_FUNCTION__ << "CREATING DQM!!!Consumer Name: " << itConsumerName << std::endl;
					std::cout << __PRETTY_FUNCTION__ << "CREATING DQM!!!Consumer Name: " << itConsumerName << std::endl;
					std::cout << __PRETTY_FUNCTION__ << "CREATING DQM!!!Consumer Name: " << itConsumerName << std::endl;
				    aDataProcessor = new DQMHistosConsumer    (supervisorType_, supervisorInstance_, itProcessID, itConsumerName);
					//theEventBuilderMap_   [itProcessID] = new AssociativeMemoryEventBuilder(cProcessName_+processName.str()+"EventBuilder");
					//DataManager::addConsumer     (cProcessName_+processName.str(), theEventBuilderMap_[itProcessID]);

				}
				aDataProcessor->registerToProcess();
			}
		}
	}
//    //FIXME according to the configuration I will instantiate all the producers consumers I need
//    //FIXME the name should be a property of the supervisors
//	const DataManagerConfiguration* dataManagerConfiguration = theConfigurationManager_->getConfiguration<DataManagerConfiguration>();
//	theDataListener_ = new DataListener(
//			supervisorType_,
//			supervisorInstance_,
//			0,
//			"DataListener",
//			dataManagerConfiguration->getProducerParameter(supervisorType_, supervisorInstance_, itProcessID, itProducerName, "IP"),
//			strtoul(dataManagerConfiguration->getProducerParameter(supervisorType_, supervisorInstance_, itProcessID, itProducerName,"Port").c_str(),NULL,10));
//    theDataListener_  = new DataListener (supervisorType_, supervisorInstance_, 0, "192.168.133.1", 50002);
//    theLiveDQMHistos_ = new DQMHistos    (supervisorType_, supervisorInstance_, 0, "LiveDQMHistos");
//
//
//    DataManager::configureProcess<std::string>(cProcessName_, theDataListener_);
//    DataManager::addConsumer                  (cProcessName_, theLiveDQMHistos_);

}

//========================================================================================================================
void VisualDataManager::halt(void)
{
    stop();
    DataManager::resetAllProcesses(); //Deletes all pointers created and given to the DataManager!
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

    DataManager::startAllProcesses(runNumber);
}

//========================================================================================================================
void VisualDataManager::stop()
{
    DataManager::stopAllProcesses();
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
