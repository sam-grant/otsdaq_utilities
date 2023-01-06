#ifndef _ots_ECLSupervisor_h
#define _ots_ECLSupervisor_h

#include "otsdaq/FiniteStateMachine/RunControlStateMachine.h"
#include "otsdaq/SOAPUtilities/SOAPMessenger.h"
#include "otsdaq/WebUsersUtilities/RemoteWebUsers.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#if __GNUC__ >= 8
#pragma GCC diagnostic ignored "-Wcatch-value"
#endif

#include <xdaq/Application.h>
#pragma GCC diagnostic pop
#include "otsdaq/Macros/XDAQApplicationMacros.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#include <cgicc/HTMLClasses.h>
#include <cgicc/HTMLDoctype.h>
#include <cgicc/HTTPCookie.h>
#include <cgicc/HTTPHeader.h>
#include <xgi/Method.h>  //for cgicc::Cgicc
#pragma GCC diagnostic pop

#include <chrono>
#include <map>
#include <string>
#include "otsdaq/CoreSupervisors/CoreSupervisorBase.h"

// clang-format off
namespace ots
{
class ConfigurationManager;
class TableGroupKey;

class ECLSupervisor : public CoreSupervisorBase
{
  public:
	XDAQ_INSTANTIATOR();

											ECLSupervisor			(xdaq::ApplicationStub* s);
	virtual 								~ECLSupervisor			(void);
	void 									init					(void);
	void 									destroy					(void);

	xoap::MessageReference 					MakeSystemLogEntry		(xoap::MessageReference msg);

  private:

	std::string                           	ECLUser_;
	std::string                           	ECLHost_;
	std::string                           	ECLPwd_;
	std::string                           	ECLCategory_;
	std::string                           	ExperimentName_;

	const std::string						EscapeECLString			(const std::string& input = "");

};
}
// clang-format on

#endif
