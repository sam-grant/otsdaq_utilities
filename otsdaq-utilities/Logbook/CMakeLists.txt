include_directories($ENV{XERCES_C_INC})

cet_make(LIBRARY_NAME Logbook
		 LIBRARIES
		 artdaq-core_Utilities
		 ${XDAQ_BASIC_LIB_LIST}
		 CoreSupervisors
		 CgiDataUtilities
		 XmlUtilities
		 SOAPUtilities
		 SupervisorInfo
		 WebUsersUtilities
		 FiniteStateMachine
)

install_headers()
install_source()
