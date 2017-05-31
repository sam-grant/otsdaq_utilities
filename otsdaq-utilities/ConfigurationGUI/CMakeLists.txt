include_directories($ENV{XERCES_C_INC})
cet_make(LIBRARY_NAME ConfigurationGUI
		 LIBRARIES
		 ${XDAQ_BASIC_LIB_LIST}
		 ${CETLIB_EXCEPT}
		 ConfigurationInterface
		 ConfigurationDataFormats
		 #otsdaq-core_ConfigurationPluginDataFormats_ConfigurationAliases_configuration
		 #otsdaq-core_ConfigurationPluginDataFormats_Configurations_configuration
		 SupervisorDescriptorInfo
		 CgiDataUtilities
		 WebUsersUtilities
		 SOAPUtilities
		 XmlUtilities
		 MessageFacility
		 cetlib
         $ENV{XERCES_C_LIB}/libxerces-c.so
	 ${MF_MESSAGELOGGER}
)

install_headers()
install_source()
