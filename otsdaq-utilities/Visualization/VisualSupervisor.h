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

    VisualSupervisor            	(xdaq::ApplicationStub * s) ;
    virtual ~VisualSupervisor   	(void);


    void 					destroy               			(void);

    virtual void			setSupervisorPropertyDefaults	(void);
    virtual void 			Default               			(xgi::Input* in, xgi::Output* out) ;
    virtual void 			request                     	(xgi::Input* in, xgi::Output* out) ;

    void 					dataRequest                 	(xgi::Input* in, xgi::Output* out) ;
    void 					safari               			(xgi::Input* in, xgi::Output* out) ;

    void 					stateRunning         			(toolbox::fsm::FiniteStateMachine& fsm) ;

    virtual void 			transitionConfiguring 			(toolbox::Event::Reference e) ;
    virtual void 			transitionHalting     			(toolbox::Event::Reference e) ;
    virtual void 			transitionInitializing			(toolbox::Event::Reference e)  {}
    virtual void 			transitionPausing     			(toolbox::Event::Reference e) ;
    virtual void 			transitionResuming    			(toolbox::Event::Reference e) ;
    virtual void 			transitionStarting    			(toolbox::Event::Reference e) ;
    virtual void 			transitionStopping    			(toolbox::Event::Reference e) ;
	virtual void 			enteringError                   (toolbox::Event::Reference e)  {}

private:

    void						binaryBufferToHexString		(char *buff, unsigned int len, std::string& dest);

    VisualDataManager*          theDataManager_;
    unsigned int 			    loadedRunNumber_;

};

}

#endif
