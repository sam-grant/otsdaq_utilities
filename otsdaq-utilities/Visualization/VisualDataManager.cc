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
VisualDataManager::VisualDataManager(ConfigurationManager* configurationManager, std::string supervisorContextUID, std::string supervisorApplicationUID)
: DataManager             (configurationManager, supervisorContextUID, supervisorApplicationUID)
, theConfigurationManager_(configurationManager)
, theLiveDQMHistos_       (0)
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
	const DataManagerConfiguration* dataManagerConfiguration = theConfigurationManager_->__GET_CONFIG__(DataManagerConfiguration);
	const DataBufferConfiguration*  dataBufferConfiguration  = theConfigurationManager_->__GET_CONFIG__(DataBufferConfiguration);

	__MOUT__ << "NEED TO FIX ALL THIS ROUTINE USING THE TREE!!!!!!" << std::endl;
	__MOUT__ << "NEED TO FIX ALL THIS ROUTINE USING THE TREE!!!!!!" << std::endl;
	__MOUT__ << "NEED TO FIX ALL THIS ROUTINE USING THE TREE!!!!!!" << std::endl;
	__MOUT__ << "NEED TO FIX ALL THIS ROUTINE USING THE TREE!!!!!!" << std::endl;
	__MOUT__ << "NEED TO FIX ALL THIS ROUTINE USING THE TREE!!!!!!" << std::endl;
	__MOUT__ << "NEED TO FIX ALL THIS ROUTINE USING THE TREE!!!!!!" << std::endl;
	__MOUT__ << "NEED TO FIX ALL THIS ROUTINE USING THE TREE!!!!!!" << std::endl;
	__MOUT__ << "NEED TO FIX ALL THIS ROUTINE USING THE TREE!!!!!!" << std::endl;
	__MOUT__ << "NEED TO FIX ALL THIS ROUTINE USING THE TREE!!!!!!" << std::endl;
/*
	std::vector<std::string> bufferList = dataManagerConfiguration->getListOfDataBuffers(supervisorType_,supervisorInstance_);
	for(const auto& itBuffers: bufferList)
	{
		std::vector<std::string> consumerList = dataBufferConfiguration->getConsumerIDList(itBuffers);
		for(const auto& it: consumerList)
		{
			std::cout << __PRETTY_FUNCTION__ << "CONSUMER: " << it << std::endl;
			if(dataBufferConfiguration->getConsumerClass(itBuffers,it) == "OTDQMHistosConsumer")
			{
				std::cout << __PRETTY_FUNCTION__ << "FOUND DQM: " << it << std::endl;
				for(const auto& itConsumer: buffers_[itBuffers].consumers_)
				{
					std::cout << __PRETTY_FUNCTION__ << "CONSUMER PROCESSOR: " << itConsumer->getProcessorID() << std::endl;
					if(itConsumer->getProcessorID() == it)
					{
						std::cout << __PRETTY_FUNCTION__ << "CONSUMER: " << it << std::endl;
						theLiveDQMHistos_ = static_cast<DQMHistosConsumerBase*>(itConsumer.get());
						//theLiveDQMHistos_->setConfigurationManager(theConfigurationManager_);
					}
				}
			}
		}
	}
*/
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
