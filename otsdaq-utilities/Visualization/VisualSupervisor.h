#ifndef _ots_VisualSupervisor_h
#define _ots_VisualSupervisor_h


#include "otsdaq-core/CoreSupervisors/CoreSupervisorBase.h"
#include "otsdaq-utilities/Visualization/VisualDataManager.h"

#include <string>
#include <map>

namespace ots
{

class VisualSupervisor: public CoreSupervisorBase
{

public:

    XDAQ_INSTANTIATOR();

    VisualSupervisor            	(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);
    virtual ~VisualSupervisor   	(void);


    void 					destroy               			(void);

    virtual void			setSupervisorPropertyDefaults	(void);
    virtual void 			Default               			(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    virtual void 			request                     	(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);

    void 					dataRequest                 	(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
    void 					safari               			(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
                              
    void 					stateRunning         			(toolbox::fsm::FiniteStateMachine& fsm) throw (toolbox::fsm::exception::Exception);

    virtual void 			transitionConfiguring 			(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
    virtual void 			transitionHalting     			(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
    virtual void 			transitionInitializing			(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
    virtual void 			transitionPausing     			(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
    virtual void 			transitionResuming    			(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
    virtual void 			transitionStarting    			(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
    virtual void 			transitionStopping    			(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
    virtual void 			enteringError         			(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);

private:

    void						binaryBufferToHexString		(char *buff, unsigned int len, std::string& dest);

    VisualDataManager*          theDataManager_;
    unsigned int 			    loadedRunNumber_;

};

}

#endif
