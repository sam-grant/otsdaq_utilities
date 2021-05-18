include_directories($ENV{XERCES_C_INC})

cet_make(LIBRARY_NAME Logbook
		 LIBRARIES
		 artdaq_core::artdaq-core_Utilities
		 ${XDAQ_BASIC_LIB_LIST}
        otsdaq::Macros
		 otsdaq::CoreSupervisors
		 otsdaq::CgiDataUtilities
		 otsdaq::XmlUtilities
		 otsdaq::SOAPUtilities
		 otsdaq::SupervisorInfo
		 otsdaq::WebUsersUtilities
		 otsdaq::FiniteStateMachine
		 MF_MessageLogger
)

install_headers()
install_source()
