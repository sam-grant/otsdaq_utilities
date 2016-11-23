#include "otsdaq-utilities/Visualization/VisualDataManager.h"
#include "otsdaq-core/DataManager/DQMHistosConsumerBase.h"
#include "otsdaq-core/DataManager/DataManager.h"
#include "otsdaq-core/DataManager/DataProcessor.h"
#include "otsdaq-core/ConfigurationPluginDataFormats/DataManagerConfiguration.h"
#include "otsdaq-core/ConfigurationPluginDataFormats/DataBufferConfiguration.h"
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
	theLiveDQMHistos_ = 0;

	DataManager::configure();

	//FIXME SUPER FIXME THIS WORKS ONLY WITH THE OTDQM CONSUMER!!!!!!!
	__MOUT__ << "SUPER FIXME THIS WORKS ONLY WITH THE OTDQM CONSUMER!!!!!!!" << std::endl;
	__MOUT__ << "SUPER FIXME THIS WORKS ONLY WITH THE OTDQM CONSUMER!!!!!!!" << std::endl;
	__MOUT__ << "SUPER FIXME THIS WORKS ONLY WITH THE OTDQM CONSUMER!!!!!!!" << std::endl;
	__MOUT__ << "SUPER FIXME THIS WORKS ONLY WITH THE OTDQM CONSUMER!!!!!!!" << std::endl;
	for(const auto& buffer: theXDAQContextConfigTree_.getNode(theConfigurationPath_+"/LinkToDataManagerConfiguration").getChildren())
	{
		__MOUT__ << "Data Buffer Name: "<< buffer.first << std::endl;
		if(buffer.second.getNode("DataBufferStatus").getValue<bool>())
		{
			std::vector<std::string> producers;
			std::vector<std::string> consumers;
			auto bufferConfigurationMap = buffer.second.getNode("LinkToDataBufferConfiguration").getChildren();
			for(const auto& bufferConfiguration: bufferConfigurationMap)
			{
				__MOUT__ << "Processor id: " << bufferConfiguration.first << std::endl;
				if(bufferConfiguration.second.getNode("ProcessorStatus").getValue<bool>()
						&& (bufferConfiguration.second.getNode("ProcessorType").getValue<std::string>() == "Consumer")
						//&& (bufferConfiguration.second.getNode("ProcessorPluginName").getValue<std::string>() == "OTDQMHistosConsumer")
				)
				{
						__MOUT__ << "FOUND DQM: OTDQMHistosConsumer!" << std::endl;
						for(const auto& itConsumer: buffers_[buffer.first].consumers_)
						{
							std::cout << __PRETTY_FUNCTION__ << "CONSUMER PROCESSOR: " << itConsumer->getProcessorID() << std::endl;
							if(itConsumer->getProcessorID() == bufferConfiguration.second.getNode("ProcessorUID").getValue<std::string>())
							{
								std::cout << __PRETTY_FUNCTION__ << "CONSUMER: " << itConsumer->getProcessorID() << std::endl;
								theLiveDQMHistos_ = static_cast<DQMHistosConsumerBase*>(itConsumer.get());
								//theLiveDQMHistos_->setConfigurationManager(theConfigurationManager_);
							}
						}
					}
			}
		}
	}
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
DQMHistosBase* VisualDataManager::getLiveDQMHistos(void)
{
	return theLiveDQMHistos_;
}

//========================================================================================================================
DQMHistosBase& VisualDataManager::getFileDQMHistos(void)
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
