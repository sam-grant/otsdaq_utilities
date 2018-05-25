include_directories($ENV{XERCES_C_INC})
cet_make(LIBRARY_NAME ConfigurationGUI
		 LIBRARIES
		 artdaq-core_Utilities
		 ${XDAQ_BASIC_LIB_LIST}
		 ${CETLIB_EXCEPT}
		 ConfigurationInterface
		 ConfigurationDataFormats
		 otsdaq-core_ConfigurationPluginDataFormats_IterateConfiguration_configuration
		 FiniteStateMachine
		 CoreSupervisors
		 SupervisorInfo
		 CgiDataUtilities
		 WebUsersUtilities
		 #SOAPUtilities
		 XmlUtilities
         Macros
		 #MessageFacility
		 cetlib
         $ENV{XERCES_C_LIB}/libxerces-c.so
	 ${MF_MESSAGELOGGER}
)

install_headers()
install_source()
