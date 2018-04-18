#include "otsdaq-utilities/Visualization/VisualDataManager.h"
#include "otsdaq-core/DataManager/DQMHistosConsumerBase.h"
#include "otsdaq-core/DataManager/DataManager.h"
#include "otsdaq-core/DataManager/DataProcessor.h"
#include "otsdaq-core/ConfigurationInterface/ConfigurationManager.h"

#include "otsdaq-core/DataProcessorPlugins/RawDataVisualizerConsumer.h"

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
	__COUT__ << "Start!" << __E__;
	
	theLiveDQMHistos_ = NULL;
	theRawDataConsumer_ = NULL;

	DataManager::start(runNumber);
	
	
	auto buffers = theXDAQContextConfigTree_.getNode(theConfigurationPath_+"/LinkToDataManagerConfiguration").getChildren();
	
	__COUT__ << "Buffer count " << buffers.size() << __E__;
	
	
	for(const auto& buffer:buffers)
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
						__COUT__ << "Consumer Plugin Type = " << bufferConfiguration.second.getNode("ProcessorPluginName") << __E__;
						
						for(const auto& itConsumer: buffers_[buffer.first].consumers_)
						{
							__COUT__ << "CONSUMER PROCESSOR: " << itConsumer->getProcessorID() << std::endl;
							if(itConsumer->getProcessorID() == bufferConfiguration.second.getNode("ProcessorUID").getValue<std::string>())
							{
								__COUT__ << "CONSUMER: " << itConsumer->getProcessorID() << std::endl;

								try
								{
									__COUT__ << "Trying for DQMHistosConsumerBase." << __E__;
									theLiveDQMHistos_ = dynamic_cast<DQMHistosConsumerBase*>(itConsumer.get());
									
									
									__COUT__ << "Did we succeed? " << theLiveDQMHistos_ <<
											__E__;
								}
								catch(...){} //ignore failures

								if(!theLiveDQMHistos_)
								{
									__COUT__ << "Trying for raw data consumer." << __E__;

									try
									{
										theRawDataConsumer_ = dynamic_cast<RawDataVisualizerConsumer*>(itConsumer.get());
									}
									catch(...){}

									__COUT__ << "Did we succeed? " << theRawDataConsumer_ <<
											__E__;
								}
								
								
								if(!theLiveDQMHistos_ && !theRawDataConsumer_)
								{
								  __SS__ << "No valid visualizer consumer!" << __E__;
								  __COUT_ERR__ << ss.str();
								  throw std::runtime_error(ss.str());
								}
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
//========================================================================================================================
const std::string&	 VisualDataManager::getRawData(void)
{
  //__COUT__ << __E__;
  
  return theRawDataConsumer_->getLastRawDataBuffer();
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
