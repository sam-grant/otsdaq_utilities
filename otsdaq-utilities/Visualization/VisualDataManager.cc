#include "otsdaq-utilities/Visualization/VisualDataManager.h"
#include "otsdaq-core/DataManager/DQMHistosConsumerBase.h"
#include "otsdaq-core/DataManager/DataManager.h"
#include "otsdaq-core/DataManager/DataProcessor.h"
#include "otsdaq-core/ConfigurationInterface/ConfigurationManager.h"

#include <iostream>
#include <sstream>
#include <cassert>

using namespace ots;


//========================================================================================================================
VisualDataManager::VisualDataManager(const ConfigurationTree& theXDAQContextConfigTree, const std::string& supervisorConfigurationPath)
: DataManager               (theXDAQContextConfigTree, supervisorConfigurationPath)
, theLiveDQMHistos_         (0)
//, theFileDQMHistos_       (supervisorType, supervisorInstance, "VisualBuffer", "FileDQMHistos")
//, theFileDQMHistos_       (supervisorType, supervisorInstance, "VisualBuffer", "FileDQMHistos",0)
//, theFileDQMHistos_       ()
{}

//========================================================================================================================
VisualDataManager::~VisualDataManager(void)
{}

//========================================================================================================================
void VisualDataManager::configure(void)
{
	DataManager::configure();
}

//========================================================================================================================
void VisualDataManager::halt(void)
{
	theLiveDQMHistos_ = 0;
	DataManager::halt();
}

//========================================================================================================================
void VisualDataManager::pause(void)
{
	__COUT__ << "Pausing..." << std::endl;
	DataManager::pause();
}

//========================================================================================================================
void VisualDataManager::resume(void)
{
	DataManager::resume();
}

//========================================================================================================================
void VisualDataManager::start(std::string runNumber)
{
	DataManager::start(runNumber);
	for(const auto& buffer: theXDAQContextConfigTree_.getNode(theConfigurationPath_+"/LinkToDataManagerConfiguration").getChildren())
	{
		__COUT__ << "Data Buffer Name: "<< buffer.first << std::endl;
		if(buffer.second.getNode(ViewColumnInfo::COL_NAME_STATUS).getValue<bool>())
		{
			std::vector<std::string> producers;
			std::vector<std::string> consumers;
			auto bufferConfigurationMap = buffer.second.getNode("LinkToDataBufferConfiguration").getChildren();
			for(const auto& bufferConfiguration: bufferConfigurationMap)
			{
				__COUT__ << "Processor id: " << bufferConfiguration.first << std::endl;
				if(bufferConfiguration.second.getNode(ViewColumnInfo::COL_NAME_STATUS).getValue<bool>()
						&& (bufferConfiguration.second.getNode("ProcessorType").getValue<std::string>() == "Consumer")
				)
				{
						for(const auto& itConsumer: buffers_[buffer.first].consumers_)
						{
							std::cout << __PRETTY_FUNCTION__ << "CONSUMER PROCESSOR: " << itConsumer->getProcessorID() << std::endl;
							if(itConsumer->getProcessorID() == bufferConfiguration.second.getNode("ProcessorUID").getValue<std::string>())
							{
								std::cout << __PRETTY_FUNCTION__ << "CONSUMER: " << itConsumer->getProcessorID() << std::endl;
								theLiveDQMHistos_ = static_cast<DQMHistosConsumerBase*>(itConsumer.get());
							}
						}
					}
			}
		}
	}

}

//========================================================================================================================
void VisualDataManager::stop(void)
{
	theLiveDQMHistos_ = 0;
	DataManager::stop();
}

//========================================================================================================================
void VisualDataManager::load(std::string fileName, std::string type)
{
	if(type == "Histograms")
		theFileDQMHistos_.load(fileName);
//	else if(type == "Monicelli")
//		theMonicelliEventAnalyzer_.load(fileName);
//	else if(type == "Geometry")
//		theMonicelliGeometryConverter_.loadGeometry(fileName);

}

//========================================================================================================================
DQMHistosBase* VisualDataManager::getLiveDQMHistos(void)
{
	return theLiveDQMHistos_;
}

//========================================================================================================================
DQMHistosBase& VisualDataManager::getFileDQMHistos(void)
{
	return theFileDQMHistos_;
}

////========================================================================================================================
//const Visual3DEvents& VisualDataManager::getVisual3DEvents(void)
//{
//	return theMonicelliEventAnalyzer_.getEvents();
//}
//
////========================================================================================================================
//const Visual3DGeometry& VisualDataManager::getVisual3DGeometry(void)
//{
//	return theMonicelliGeometryConverter_.getGeometry();
//}
