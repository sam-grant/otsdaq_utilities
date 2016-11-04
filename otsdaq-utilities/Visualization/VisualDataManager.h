#ifndef _ots_VisualDataManager_h_
#define _ots_VisualDataManager_h_

#include "otsdaq-core/DataManager/DataManager.h"
#include "otsdaq-core/MonicelliInterface/Visual3DEvent.h"
#include "otsdaq-core/MonicelliInterface/Visual3DGeometry.h"
#include "otsdaq-core/MonicelliInterface/MonicelliEventAnalyzer.h"
#include "otsdaq-core/MonicelliInterface/MonicelliGeometryConverter.h"
#include "otsdaq-core/RootUtilities/DQMHistosBase.h"

#include <map>
#include <string>
#include <vector>

namespace ots
{

class ConfigurationManager;

class VisualDataManager : public DataManager
{
public:
    VisualDataManager(ConfigurationManager* configurationManager, std::string supervisorContextUID, std::string supervisorApplicationUID);
    virtual ~VisualDataManager(void);


    void configure(void);


    void load(std::string fileName, std::string type);
    //Getters
    DQMHistosBase*          getLiveDQMHistos    (void);
    DQMHistosBase&          getFileDQMHistos    (void);
    const Visual3DEvents&   getVisual3DEvents   (void);
    const Visual3DGeometry& getVisual3DGeometry (void);

private:
    ConfigurationManager*      theConfigurationManager_;
    DQMHistosBase*             theLiveDQMHistos_;
    DQMHistosBase              theFileDQMHistos_;
    MonicelliEventAnalyzer     theMonicelliEventAnalyzer_;
    MonicelliGeometryConverter theMonicelliGeometryConverter_;
    //Visual3DData           the3DData_;
};

}

#endif
