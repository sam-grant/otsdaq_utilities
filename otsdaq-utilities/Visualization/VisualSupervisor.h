#ifndef _ots_VisualSupervisor_h
#define _ots_VisualSupervisor_h


#include "otsdaq-core/CoreSupervisors/CoreSupervisorBase.h"
#include "otsdaq-utilities/Visualization/VisualDataManager.h"

namespace ots
{

class VisualSupervisor: public CoreSupervisorBase
{

public:

    XDAQ_INSTANTIATOR();

    VisualSupervisor            	(xdaq::ApplicationStub * s) ;
    virtual ~VisualSupervisor   	(void);


    void 					destroy               			(void);


    virtual void 			defaultPage      				(xgi::Input* in, xgi::Output* out) override;
    void 					safariDefaultPage     			(xgi::Input* in, xgi::Output* out) ;

    virtual void			request         	 			(const std::string& requestType, cgicc::Cgicc& cgiIn, HttpXmlDocument& xmlOut, const WebUsers::RequestUserInfo& userInfo)  override;

    virtual void			setSupervisorPropertyDefaults	(void) override;
    virtual void			forceSupervisorPropertyValues	(void) override; //override to force supervisor property values (and ignore user settings)


                              
    void 					stateRunning         			(toolbox::fsm::FiniteStateMachine& fsm) ;

    virtual void 			transitionConfiguring 			(toolbox::Event::Reference e) ;
    virtual void 			transitionHalting     			(toolbox::Event::Reference e) ;
    //virtual void 			transitionInitializing			(toolbox::Event::Reference e) ;
    virtual void 			transitionPausing     			(toolbox::Event::Reference e) ;
    virtual void 			transitionResuming    			(toolbox::Event::Reference e) ;
    virtual void 			transitionStarting    			(toolbox::Event::Reference e) ;
    virtual void 			transitionStopping    			(toolbox::Event::Reference e) ;
    //virtual void 			enteringError         			(toolbox::Event::Reference e) ;

private:

    void						binaryBufferToHexString		(char *buff, unsigned int len, std::string& dest);

    VisualDataManager*          theDataManager_;
    unsigned int 			    loadedRunNumber_;

};

}

#endif
