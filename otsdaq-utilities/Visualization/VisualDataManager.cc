#include "otsdaq-utilities/Visualization/VisualDataManager.h"
#include "otsdaq/ConfigurationInterface/ConfigurationManager.h"
#include "otsdaq/DataManager/DQMHistosConsumerBase.h"
#include "otsdaq/DataManager/DataManager.h"
#include "otsdaq/DataManager/DataProcessor.h"

#include "otsdaq/DataProcessorPlugins/RawDataVisualizerConsumer.h"

#include <cassert>
#include <chrono>  // std::chrono::seconds
#include <iostream>
#include <sstream>
#include <thread>  // std::this_thread::sleep_for

using namespace ots;

//==============================================================================
VisualDataManager::VisualDataManager(const ConfigurationTree& theXDAQContextConfigTree,
                                     const std::string&       supervisorConfigurationPath)
    : DataManager(theXDAQContextConfigTree, supervisorConfigurationPath)
    , theLiveDQMHistos_(nullptr)
    , theRawDataConsumer_(nullptr)
//, theFileDQMHistos_ (supervisorType, supervisorInstance, "VisualBuffer",
//"FileDQMHistos") , theFileDQMHistos_ (supervisorType, supervisorInstance,
//"VisualBuffer", "FileDQMHistos",0) , theFileDQMHistos_ ()
{
}

//==============================================================================
VisualDataManager::~VisualDataManager(void) {}

//==============================================================================
void VisualDataManager::configure(void) { DataManager::configure(); }

//==============================================================================
void VisualDataManager::halt(void)
{
	theLiveDQMHistos_ = nullptr;
	DataManager::halt();
}

//==============================================================================
void VisualDataManager::pause(void)
{
	__CFG_COUT__ << "Pausing..." << std::endl;
	DataManager::pause();
}

//==============================================================================
void VisualDataManager::resume(void) { DataManager::resume(); }

//==============================================================================
void VisualDataManager::start(std::string runNumber)
{
	__CFG_COUT__ << "Start!" << __E__;

	theLiveDQMHistos_   = nullptr;
	theRawDataConsumer_ = nullptr;

	DataManager::start(runNumber);

	auto buffers = theXDAQContextConfigTree_
	                   .getNode(theConfigurationPath_ + "/LinkToDataBufferTable")
	                   .getChildren();

	__CFG_COUT__ << "Buffer count " << buffers.size() << __E__;

	for(const auto& buffer : buffers)
	{
		__CFG_COUT__ << "Data Buffer Name: " << buffer.first << std::endl;
		if(buffer.second.getNode(TableViewColumnInfo::COL_NAME_STATUS).getValue<bool>())
		{
			std::vector<std::string> producers;
			std::vector<std::string> consumers;
			auto                     bufferConfigurationMap =
			    buffer.second.getNode("LinkToDataProcessorTable").getChildren();
			for(const auto& bufferConfiguration : bufferConfigurationMap)
			{
				__CFG_COUT__ << "Processor id: " << bufferConfiguration.first
				             << std::endl;
				if(bufferConfiguration.second
				       .getNode(TableViewColumnInfo::COL_NAME_STATUS)
				       .getValue<bool>() &&
				   (bufferConfiguration.second.getNode("ProcessorType")
				        .getValue<std::string>() == "Consumer"))
				{
					__CFG_COUT__
					    << "Consumer Plugin Type = "
					    << bufferConfiguration.second.getNode("ProcessorPluginName")
					    << __E__;

					auto bufferIt = buffers_.at(buffer.first);
					for(const auto& consumer : bufferIt.consumers_)
					{
						__CFG_COUT__
						    << "CONSUMER PROCESSOR: " << consumer->getProcessorID()
						    << std::endl;
						if(consumer->getProcessorID() ==
						   bufferConfiguration.second.getNode("ProcessorUID")
						       .getValue<std::string>())
						{
							__CFG_COUT__ << "CONSUMER: " << consumer->getProcessorID()
							             << std::endl;

							try
							{
								__CFG_COUT__ << "Trying for DQMHistosConsumerBase."
								             << __E__;
								theLiveDQMHistos_ =
								    dynamic_cast<DQMHistosConsumerBase*>(consumer);

								__CFG_COUT__ << "Did we succeed? " << theLiveDQMHistos_
								             << __E__;
							}
							catch(...)
							{
							}  // ignore failures

							if(theLiveDQMHistos_ == nullptr)
							{
								__CFG_COUT__ << "Trying for raw data consumer." << __E__;

								try
								{
									theRawDataConsumer_ =
									    dynamic_cast<RawDataVisualizerConsumer*>(
									        consumer);
								}
								catch(...)
								{
								}

								__CFG_COUT__ << "Did we succeed? " << theRawDataConsumer_
								             << __E__;
							}

							if(!theLiveDQMHistos_ && !theRawDataConsumer_)
							{
								__CFG_SS__ << "No valid visualizer consumer!" << __E__;
								__CFG_SS_THROW__;
							}
						}
					}
				}
			}
		}
	}
}

//==============================================================================
void VisualDataManager::stop(void)
{
	theLiveDQMHistos_ = nullptr;
	DataManager::stop();
}

//==============================================================================
void VisualDataManager::load(std::string fileName, std::string type)
{
	if(type == "Histograms")
		theFileDQMHistos_.load(fileName);
	//	else if(type == "Monicelli")
	//		theMonicelliEventAnalyzer_.load(fileName);
	//	else if(type == "Geometry")
	//		theMonicelliGeometryConverter_.loadGeometry(fileName);
}

//==============================================================================
DQMHistosBase* VisualDataManager::getLiveDQMHistos(void) { return theLiveDQMHistos_; }

//==============================================================================
DQMHistosBase& VisualDataManager::getFileDQMHistos(void) { return theFileDQMHistos_; }
//==============================================================================
const std::string& VisualDataManager::getRawData(void)
{
	//__CFG_COUT__ << __E__;

	return theRawDataConsumer_->getLastRawDataBuffer();
}

////==============================================================================
// const Visual3DEvents& VisualDataManager::getVisual3DEvents(void)
//{
//	return theMonicelliEventAnalyzer_.getEvents();
//}
//
////==============================================================================
// const Visual3DGeometry& VisualDataManager::getVisual3DGeometry(void)
//{
//	return theMonicelliGeometryConverter_.getGeometry();
//}
