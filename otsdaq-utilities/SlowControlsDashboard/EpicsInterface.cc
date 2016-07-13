#include "EpicsInterface.h"
#include <iostream>
#include "alarm.h" //Holds strings that we can use to access the alarm status, severity, and parameters
//#include "/mu2e/ups/epics/v3_15_4/Linux64bit+2.6-2.12-e10/include/alarm.h"
//#include "alarmString.h"
#include "cadef.h" //EPICS Channel Access:
                        //http://www.aps.anl.gov/epics/base/R3-14/12-docs/CAref.html
                        //Example compile options:
//Compiling:
//Setup epics (See redmine wiki)
//g++ -std=c++11  EpicsCAMonitor.cpp EpicsCAMessage.cpp EpicsWebClient.cpp SocketUDP.cpp SocketTCP.cpp -L$EPICS_BASE/lib/linux-x86_64/ -Wl,-rpath,$EPICS_BASE/lib/linux-x86_64 -lca -lCom -I$EPICS_BASE//include -I$EPICS_BASE//include/os/Linux -I$EPICS_BASE/include/compiler/gcc -o EpicsWebClient

#define DEBUG false

using namespace ots;

EpicsInterface::EpicsInterface()
{
  SEVCHK(ca_context_create(ca_enable_preemptive_callback), "EpicsInterface::EpicsInterface() : ca_enable_preemptive_callback_init()");
}

EpicsInterface::~EpicsInterface()
{
  destroy();
}

void EpicsInterface::destroy()
{
  // std::cout << "mapOfPVInfo_.size() = " << mapOfPVInfo_.size() << std::endl;
  for(auto it = mapOfPVInfo_.begin(); it != mapOfPVInfo_.end(); it++)
    {
      cancelSubscriptionToChannel(it->first);
      destroyChannel(it->first);
      delete(it->second->parameterPtr);
      delete (it->second);
      mapOfPVInfo_.erase(it);
    }

  //std::cout << "mapOfPVInfo_.size() = " << mapOfPVInfo_.size() << std::endl; 
  SEVCHK(ca_poll(),"EpicsInterface::destroy() : ca_poll"); 
  return;
}

void EpicsInterface::initialize()
{
  destroy();
  loadListOfPVs();
  
  return;
}
std::string EpicsInterface::getList (std::string format)
{
  std::string pvList;

  if(format == "JSON")
    {
      pvList = "{\"PVList\" : [";
      for(auto it = mapOfPVInfo_.begin(); it != mapOfPVInfo_.end(); it++)
	pvList += "\"" +  it->first + "\", ";
      
      pvList.resize(pvList.size()-2);
      pvList += "]}";     
      return pvList;
    }
  return pvList;


}

void EpicsInterface::subscribe (std::string pvName)
{
  if(!checkIfPVExists(pvName))
    {
      std::cout << pvName << " doesn't exist!" << std::endl;
      return;
    }
  createChannel(pvName);
  sleep(2);
  subscribeToChannel(pvName, mapOfPVInfo_.find(pvName)->second->channelType);
  SEVCHK(ca_poll(), "EpicsInterface::subscribe() : ca_poll");

  return;
}
//{"PVList" : ["Mu2e_BeamData_IOC/CurrentTime"]}
void EpicsInterface::subscribeJSON (std::string pvList)
{
  // if(DEBUG){std::cout << pvList << std::endl;;}
  
  std::string JSON = "{\"PVList\" :";
  std::string pvName;
  if(pvList.find(JSON) != std::string::npos)
    {  
      pvList = pvList.substr(pvList.find(JSON) + JSON.length(), std::string::npos);
     do {
	pvList = pvList.substr(pvList.find("\"") + 1, std::string::npos);// eliminate up to the next "
	pvName = pvList.substr(0, pvList.find("\"")); //
	//if(DEBUG){std::cout << "Read PV Name:  " << pvName << std::endl;}
	pvList = pvList.substr( pvList.find("\"") + 1, std::string::npos);
	//if(DEBUG){std::cout << "pvList : " << pvList << std::endl;}

  if(checkIfPVExists(pvName))
    {
      createChannel(pvName);
      subscribeToChannel(pvName, mapOfPVInfo_.find(pvName)->second->channelType);
      SEVCHK(ca_poll(), "EpicsInterface::subscribeJSON : ca_poll");
    }
  else
    if(DEBUG){std::cout << pvName << " not found in file! Not subscribing!" << std::endl;}
  
     }while(pvList.find(",") != std::string::npos);
    }

  return;
}

void EpicsInterface::unsubscribe(std::string pvName)
{
  if(!checkIfPVExists(pvName))

    {
      std::cout << pvName << " doesn't exist!" << std::endl;
      return;
    }
 

  cancelSubscriptionToChannel(pvName);
  return;
}
//------------------------------------------------------------------------------------------------------------
//--------------------------------------PRIVATE FUNCTION--------------------------------------
//------------------------------------------------------------------------------------------------------------
void EpicsInterface::eventCallback(struct event_handler_args eha)
{
  chid chid = eha.chid;
  if(eha.status == ECA_NORMAL) {

    
    int i;
    union db_access_val *pBuf = (union db_access_val *)eha.dbr;
    printf("channel %s: ", ca_name(eha.chid));
    
    switch (eha.type){
    case DBF_DOUBLE:
      if(DEBUG){std::cout << "Response Type: DBR_DOUBLE" << std::endl;}
       ((EpicsInterface*) eha.usr)->writePVValueToRecord(ca_name(eha.chid),std::to_string(*((double*) eha.dbr))
); //write the PV's value to records 	 	
      break;
    case DBR_STS_STRING:
      if(DEBUG){std::cout << "Response Type: DBR_STS_STRING" << std::endl;}
      ((EpicsInterface*) eha.usr)->writePVAlertToQueue(ca_name(eha.chid), epicsAlarmConditionStrings[pBuf->sstrval.status], epicsAlarmSeverityStrings[pBuf->sstrval.severity]);
      /*if(DEBUG)
	{
	  printf("current %s:\n", eha.count > 1?"values":"value");
	  for (i = 0; i < eha.count; i++)
	    {
	      printf("%s\t", *(&(pBuf->sstrval.value) + i));
	      if ((i+1)%6 == 0) printf("\n");
	      }
	  printf("\n");
	  }*/
      break;
    case DBR_STS_SHORT:
      if(DEBUG){std::cout << "Response Type: DBR_STS_SHORT" << std::endl; }
      ((EpicsInterface*) eha.usr)->writePVAlertToQueue(ca_name(eha.chid), epicsAlarmConditionStrings[pBuf->sshrtval.status], epicsAlarmSeverityStrings[pBuf->sshrtval.severity]);
      /*if(DEBUG)
	  {
	    printf("current %s:\n", eha.count > 1?"values":"value");
	    for (i = 0; i < eha.count; i++){
	      printf("%-10d", *(&(pBuf->sshrtval.value) + i));
	      if ((i+1)%8 == 0) printf("\n");
	      }
	    printf("\n");
	  }*/
	break;
    case DBR_STS_FLOAT:
      if(DEBUG){std::cout << "Response Type: DBR_STS_FLOAT" << std::endl; }
      ((EpicsInterface*) eha.usr)->writePVAlertToQueue(ca_name(eha.chid), epicsAlarmConditionStrings[pBuf->sfltval.status], epicsAlarmSeverityStrings[pBuf->sfltval.severity]);
      /*if(DEBUG)
	  {
	    printf("current %s:\n", eha.count > 1?"values":"value");
	    for (i = 0; i < eha.count; i++){
	      printf("-10.4f", *(&(pBuf->sfltval.value) + i));
	      if ((i+1)%8 == 0) printf("\n");
	      }
	    printf("\n");
	  }*/
	break;
    case DBR_STS_ENUM:
      if(DEBUG){std::cout << "Response Type: DBR_STS_ENUM" << std::endl; }
      ((EpicsInterface*) eha.usr)->writePVAlertToQueue(ca_name(eha.chid), epicsAlarmConditionStrings[pBuf->senmval.status], epicsAlarmSeverityStrings[pBuf->senmval.severity]);
      /*if(DEBUG)
	  {
	    printf("current %s:\n", eha.count > 1?"values":"value");
	    for (i = 0; i < eha.count; i++){
	      printf("%d ", *(&(pBuf->senmval.value) + i));
	      }
	    printf("\n");
	  }*/
	break;
    case DBR_STS_CHAR:
      if(DEBUG){std::cout << "Response Type: DBR_STS_CHAR" << std::endl; }
      ((EpicsInterface*) eha.usr)->writePVAlertToQueue(ca_name(eha.chid), epicsAlarmConditionStrings[pBuf->schrval.status], epicsAlarmSeverityStrings[pBuf->schrval.severity]);
      /*if(DEBUG)
	  {
	    printf("current %s:\n", eha.count > 1?"values":"value");
	    for (i = 0; i < eha.count; i++){
	      printf("%-5", *(&(pBuf->schrval.value) + i));
	      if ((i+1)%15 == 0) printf("\n");
	    }
	    printf("\n");
	    }*/
	break;
    case DBR_STS_LONG:
      if(DEBUG){std::cout << "Response Type: DBR_STS_LONG" << std::endl; }
      ((EpicsInterface*) eha.usr)->writePVAlertToQueue(ca_name(eha.chid), epicsAlarmConditionStrings[pBuf->slngval.status], epicsAlarmSeverityStrings[pBuf->slngval.severity]);
      /*if(DEBUG)
	 {
	   printf("current %s:\n", eha.count > 1?"values":"value");
	   for (i = 0; i < eha.count; i++){
	     printf("%-15d", *(&(pBuf->slngval.value) + i));
	     if((i+1)%5 == 0) printf("\n");
	   }
	   printf("\n");
	   }*/
      break;
    case DBR_STS_DOUBLE:
      if(DEBUG){std::cout << "Response Type: DBR_STS_DOUBLE" << std::endl; }
      ((EpicsInterface*) eha.usr)->writePVAlertToQueue(ca_name(eha.chid), epicsAlarmConditionStrings[pBuf->sdblval.status], epicsAlarmSeverityStrings[pBuf->sdblval.severity]);
      /*if(DEBUG)
	  {
	    printf("current %s:\n", eha.count > 1?"values":"value");
	    for (i = 0; i < eha.count; i++){
	      printf("%-15.4f", *(&(pBuf->sdblval.value) + i));
	    }
	    printf("\n");
	    }*/
	break;
    default:
      if(ca_name(eha.chid))
	{
	    if(DEBUG)
	      {
		std::cout << " EpicsInterface::eventCallback: PV Name = " <<ca_name(eha.chid) << std::endl;
		std::cout << (char *)eha.dbr << std::endl;
	      }
	  ((EpicsInterface*) eha.usr)->writePVValueToRecord(ca_name(eha.chid),(char *)eha.dbr); //write the PV's value to records 	 	

	}
      
      break;
    }
    /* if get operation failed, print channel name and message */
  }
  else
    printf("channel %s: get operation failed\n", ca_name(eha.chid));  
  
  return;
}

void EpicsInterface::staticChannelCallbackHandler(struct connection_handler_args cha)
{
  std::cout << "webClientChannelCallbackHandler" << std::endl;

  ((PVHandlerParameters *) ca_puser(cha.chid))->webClient->channelCallbackHandler(cha);
  return;
}

void EpicsInterface::channelCallbackHandler(struct connection_handler_args &cha)
{
  std::string pv = ((PVHandlerParameters*)ca_puser(cha.chid))->pvName;
  if(cha.op == CA_OP_CONN_UP)
    {
      std::cout << pv  << cha.chid << " connected! " << std::endl;
      

      mapOfPVInfo_.find(pv)->second->channelType = ca_field_type(cha.chid);
      readPVRecord(pv);

      /*status_ = ca_array_get_callback(dbf_type_to_DBR_STS(mapOfPVInfo_.find(pv)->second->channelType), ca_element_count(cha.chid), cha.chid, eventCallback, this);
	SEVCHK(status_, "ca_array_get_callback");*/
      
    }
  else
    std::cout << pv << " disconnected!" << std::endl;


  return;
}


bool EpicsInterface::checkIfPVExists (std::string pvName)
{
  if(mapOfPVInfo_.find(pvName) != mapOfPVInfo_.end())
    return true;
  
  return false;
}

void EpicsInterface::loadListOfPVs()
{
   // Initialize Channel Access
  status_ = ca_task_initialize();
  SEVCHK(status_, "EpicsInterface::loadListOfPVs() : Unable to initialize");
  if (status_ != ECA_NORMAL)
    exit(-1);
  
  //read file 
  //for each line in file
  std::ifstream infile("./PV_list.txt");
  std::cout << "Reading file" << std::endl;
  for(std::string line; getline(infile, line);)
    {
      //std::cout << line << std::endl;
      mapOfPVInfo_[line] = new PVInfo(DBR_STRING);
    }
  std::cout << "Finished reading file" << std::endl;
  return; 
}

void EpicsInterface::createChannel(std::string pvName)
{
  if(!checkIfPVExists(pvName))
    {
      std::cout << pvName << " doesn't exist!" << std::endl;
      return;
    }
  std::cout << "Trying to create channel to " << pvName << ":" << mapOfPVInfo_.find(pvName)->second->channelID << std::endl;
  
  
  if(mapOfPVInfo_.find(pvName)->second != NULL) // Check to see if the pvName maps to a null pointer so we don't have any errors
    if (mapOfPVInfo_.find(pvName)->second->channelID != NULL) //channel might exist, subscription doesn't so create a subscription
      {
	if(ca_state(mapOfPVInfo_.find(pvName)->second->channelID) == cs_conn)
	  {
	    if(DEBUG){std::cout << "Channel to " << pvName << " already exists!" << std::endl;}
	    return;
	  }
	if(DEBUG){std::cout << "Channel to " << pvName << " exists, but is not connected! Destroying current channel." <<  std::endl;}   
	destroyChannel(pvName);	         
      } 
  
  if(mapOfPVInfo_.find(pvName)->second->parameterPtr == NULL)
    {
      mapOfPVInfo_.find(pvName)->second->parameterPtr = new PVHandlerParameters(pvName, this);
    }
  SEVCHK(ca_create_channel(pvName.c_str(), staticChannelCallbackHandler,mapOfPVInfo_.find(pvName)->second->parameterPtr ,0, &(mapOfPVInfo_.find(pvName)->second->channelID)), "EpicsInterface::createChannel() : ca_create_channel") ;
  std::cout << "channelID: " << pvName << mapOfPVInfo_.find(pvName)->second->channelID << std::endl;
  SEVCHK(ca_poll(), "EpicsInterface::createChannel() : ca_poll");
   
  return;
}
void EpicsInterface::destroyChannel (std::string pvName)
{
  if(mapOfPVInfo_.find(pvName)->second != NULL)
    {
      if(mapOfPVInfo_.find(pvName)->second->channelID != NULL) 
	{
	  status_ = ca_clear_channel(mapOfPVInfo_.find(pvName)->second->channelID);
	  SEVCHK(status_, "EpicsInterface::destroyChannel() : ca_clear_channel");
	  if (status_ == ECA_NORMAL)
	    {
	      mapOfPVInfo_.find(pvName)->second->channelID = NULL;	
	      if(DEBUG){std::cout << "Killed channel to " << pvName << std::endl;}
	    }
	  SEVCHK(ca_poll(), "EpicsInterface::destroyChannel() : ca_poll");
	}
      else
	{
	  if(DEBUG) {std::cout << "No channel to " << pvName << " exists" << std::endl;}
	}
    }
      return;
}


void EpicsInterface::subscribeToChannel (std::string pvName, chtype subscriptionType)
{
   if(!checkIfPVExists(pvName))
    {
      std::cout << pvName << " doesn't exist!" << std::endl;
      return;
    }
   if(DEBUG){std::cout << "Trying to subscribe to " << pvName << ":" << mapOfPVInfo_.find(pvName)->second->channelID << std::endl;}


  if(mapOfPVInfo_.find(pvName)->second != NULL) // Check to see if the pvName maps to a null pointer so we don't have any errors
    {
      if(mapOfPVInfo_.find(pvName)->second->eventID != NULL) //subscription already exists
	{
	  if(DEBUG){std::cout << "Already subscribed to " << pvName << "!" << std::endl;}
	  //FIXME No way to check if the event ID is valid
	  //Just cancel the subscription if it already exists?
	}    
    }
  SEVCHK(ca_create_subscription(dbf_type_to_DBR(mapOfPVInfo_.find(pvName)->second->channelType),1,mapOfPVInfo_.find(pvName)->second->channelID, DBE_VALUE | DBE_ALARM | DBE_PROPERTY, eventCallback, this, &(mapOfPVInfo_.find(pvName)->second->eventID)),"EpicsInterface::subscribeToChannel() : ca_create_subscription"); 
  if(DEBUG){std::cout << "EpicsInterface::subscribeToChannel: Created Subscription to "<< mapOfPVInfo_.find(pvName)->first << "!\n" << std::endl;}
  SEVCHK(ca_poll(), "EpicsInterface::subscribeToChannel() : ca_poll");
  
  return;
}
void EpicsInterface::cancelSubscriptionToChannel(std::string pvName)
{
  if(mapOfPVInfo_.find(pvName)->second != NULL)
    if(mapOfPVInfo_.find(pvName)->second->eventID != NULL) 
      {
	status_ = ca_clear_subscription(mapOfPVInfo_.find(pvName)->second->eventID);
	SEVCHK(status_, "EpicsInterface::cancelSubscriptionToChannel() : ca_clear_subscription");
	if (status_ == ECA_NORMAL)
	  {
	    mapOfPVInfo_.find(pvName)->second->eventID = NULL; 
	    if(DEBUG){std::cout << "Killed subscription to " << pvName << std::endl;}
	  }
	SEVCHK(ca_poll(), "EpicsInterface::cancelSubscriptionToChannel() : ca_poll");
      }else
      {
	if(DEBUG) {std::cout << pvName << "does not have a subscription!" << std::endl;}
      }
  else
    {
      //std::cout << pvName << "does not have a subscription!" << std::endl;
    }
   //  SEVCHK(ca_flush_io(),"ca_flush_io"); 
  return; 
}

void EpicsInterface::readValueFromPV (std::string pvName)
{
  //SEVCHK(ca_get(DBR_String, 0, mapOfPVInfo_.find(pvName)->second->channelID, &(mapOfPVInfo_.find(pvName)->second->pvValue), eventCallback, &(mapOfPVInfo_.find(pvName)->second->callbackPtr)), "ca_get");

  return;
}

//Enforces the circular buffer
void EpicsInterface::writePVValueToRecord(std::string pvName, std::string  pdata)
{
  std::pair<time_t, std::string> currentRecord(time(0), pdata);

   if(!checkIfPVExists(pvName))
    {
      std::cout << pvName << " doesn't exist!" << std::endl;
      return;
      }
   std::cout << pdata << std::endl;

   if(mapOfPVInfo_.find(pvName)->second->mostRecentBufferIndex != mapOfPVInfo_.find(pvName)->second->dataCache.size())
    {
      mapOfPVInfo_.find(pvName)->second->dataCache[mapOfPVInfo_.find(pvName)->second->mostRecentBufferIndex] = currentRecord;
      ++mapOfPVInfo_.find(pvName)->second->mostRecentBufferIndex;
    }
   else
    {
      mapOfPVInfo_.find(pvName)->second->dataCache[0] = currentRecord;
      mapOfPVInfo_.find(pvName)->second->mostRecentBufferIndex = 1;
    }
   
   debugConsole(pvName);
   
  return;
}
void EpicsInterface::writePVAlertToQueue (std::string pvName, const char * status, const char * severity)
{
  if(!checkIfPVExists(pvName))
    {
      std::cout << pvName << " doesn't exist!" << std::endl;
      return;
    }
  PVAlerts alert(time(0), status, severity);
  mapOfPVInfo_.find(pvName)->second->alerts.push(alert);
  
  debugConsole(pvName);  
  
  return;
}
void EpicsInterface::readPVRecord (std::string pvName)
{
  status_ = ca_array_get_callback(dbf_type_to_DBR_STS(mapOfPVInfo_.find(pvName)->second->channelType), ca_element_count(mapOfPVInfo_.find(pvName)->second->channelID), mapOfPVInfo_.find(pvName)->second->channelID, eventCallback, this);
  SEVCHK(status_, "EpicsInterface::readPVRecord(): ca_array_get_callback");
  return;
}

void EpicsInterface::debugConsole (std::string pvName)
{
  
  std::cout << "==============================================================================" << std::endl;
  for(unsigned int it = 0; it < mapOfPVInfo_.find(pvName)->second->dataCache.size(); it++)
    {
      if(it == mapOfPVInfo_.find(pvName)->second->mostRecentBufferIndex-1){   std::cout << "---------------------------------------------------------------------" << std::endl;}
      std::cout << "Iteration: " << it << " | " << mapOfPVInfo_.find(pvName)->second->mostRecentBufferIndex -1<< " | " <<  mapOfPVInfo_.find(pvName)->second->dataCache[it].second << std::endl;
      if(it == mapOfPVInfo_.find(pvName)->second->mostRecentBufferIndex -1){   std::cout << "---------------------------------------------------------------------" << std::endl;}  
    }
  std::cout << "==============================================================================" << std::endl;
  std::cout << "Status:     " << " | " <<  mapOfPVInfo_.find(pvName)->second->alerts.size()  << " | " << mapOfPVInfo_.find(pvName)->second->alerts.front().status << std::endl;
  std::cout << "Severity:   " << " | " <<  mapOfPVInfo_.find(pvName)->second->alerts.size()  << " | " << mapOfPVInfo_.find(pvName)->second->alerts.front().severity << std::endl;
  std::cout << "==============================================================================" << std::endl;
  
  return;
}
void EpicsInterface::popQueue (std::string pvName)
{
  
  if(DEBUG){std::cout << "EpicsInterface::popQueue() " << std::endl;}
  mapOfPVInfo_.find(pvName)->second->alerts.pop();
  
  if(mapOfPVInfo_.find(pvName)->second->alerts.empty())
    {
      readPVRecord(pvName);
      SEVCHK(ca_poll(), "EpicsInterface::popQueue() : ca_poll");
    }
  return;
}
