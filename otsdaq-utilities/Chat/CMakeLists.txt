include_directories($ENV{XERCES_C_INC})
cet_make(LIBRARY_NAME Chat
		 LIBRARIES
         artdaq-core_Utilities
		 ${XDAQ_BASIC_LIB_LIST}
		 FiniteStateMachine
		 CoreSupervisors
		 SupervisorInfo
		 CgiDataUtilities
		 XmlUtilities
		 WebUsersUtilities
         SOAPUtilities
#	 MessageFacility
         $ENV{XERCES_C_LIB}/libxerces-c.so
)

install_headers()
install_source()
