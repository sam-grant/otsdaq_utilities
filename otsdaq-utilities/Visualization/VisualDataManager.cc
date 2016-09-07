#include "otsdaq-utilities/Visualization/VisualDataManager.h"
#include "otsdaq-core/DataManager/DataManager.h"
#include "otsdaq-core/DataManager/DataProcessor.h"
#include "otsdaq-core/DataProcessorPlugins/DQMHistosConsumer.h"
#include "otsdaq-core/ConfigurationPluginDataFormats/DataManagerConfiguration.h"
#include "otsdaq-core/ConfigurationPluginDataFormats/DataBufferConfiguration.h"
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
void VisualDataManager::configure(void)
{
	theLiveDQMHistos_ = 0;

	DataManager::configure();
	const DataManagerConfiguration*              dataManagerConfiguration           = theConfigurationManager_->__GET_CONFIG__(DataManagerConfiguration);
	const DataBufferConfiguration*               dataBufferConfiguration            = theConfigurationManager_->__GET_CONFIG__(DataBufferConfiguration);

	std::vector<std::string> bufferList = dataManagerConfiguration->getListOfDataBuffers(supervisorType_,supervisorInstance_);
	for(const auto& itBuffers: bufferList)
	{
		std::vector<std::string> consumerList = dataBufferConfiguration->getConsumerIDList(itBuffers);
		for(const auto& it: consumerList)
		{
			//std::cout << __PRETTY_FUNCTION__ << "CONSUMER: " << it << std::endl;
			if(dataBufferConfiguration->getConsumerClass(itBuffers,it) == "DQMHistosConsumer")
			{
				//std::cout << __PRETTY_FUNCTION__ << "FOUND DQM: " << it << std::endl;
				for(const auto& itConsumer: buffers_[itBuffers].consumers_)
				{
					//std::cout << __PRETTY_FUNCTION__ << "CONSUMER PROCESSOR: " << itConsumer->getProcessorID() << std::endl;
					if(itConsumer->getProcessorID() == it)
					{
						//std::cout << __PRETTY_FUNCTION__ << "CONSUMER: " << it << std::endl;
						theLiveDQMHistos_ = static_cast<DQMHistosConsumer*>(itConsumer.get());
						theLiveDQMHistos_->setConfigurationManager(theConfigurationManager_);
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
