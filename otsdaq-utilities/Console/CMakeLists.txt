cet_make_exec(otsConsoleFwd SOURCE MF_ReceiveAndForward.cpp)


#For Console Supervisor
cet_make(LIBRARY_NAME Console
		 LIBRARIES
		 artdaq_core::artdaq-core_Utilities
		 ${XDAQ_BASIC_LIB_LIST}
		 otsdaq::SupervisorInfo
		MF_MessageLogger
		${MF_UTILITIES}
		otsdaq::FiniteStateMachine
		otsdaq::CoreSupervisors
		 otsdaq::XmlUtilities
		 otsdaq::SOAPUtilities
		 otsdaq::CgiDataUtilities
		 otsdaq::WebUsersUtilities
#		 MessageFacility
		 otsdaq::NetworkUtilities
         otsdaq::Macros
		 ${Boost_REGEX_LIBRARY}
)

install_headers()
install_source()
