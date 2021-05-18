include_directories($ENV{XERCES_C_INC})
cet_make(LIBRARY_NAME Chat
		 LIBRARIES
         artdaq_core::artdaq-core_Utilities
		 ${XDAQ_BASIC_LIB_LIST}
		 otsdaq::FiniteStateMachine
		 otsdaq::CoreSupervisors
		 otsdaq::SupervisorInfo
		 otsdaq::CgiDataUtilities
		 otsdaq::XmlUtilities
		 otsdaq::WebUsersUtilities
         otsdaq::SOAPUtilities
	 MF_MessageLogger
)

install_headers()
install_source()
