include_directories($ENV{XERCES_C_INC})
cet_make(LIBRARY_NAME OtsConfigurationWizard
		 LIBRARIES
		 ${XDAQ_BASIC_LIB_LIST}
		 ConfigurationInterface
		 ConfigurationDataFormats
		 SupervisorConfigurations
		 CgiDataUtilities
		 WebUsersUtilities
		 SOAPUtilities
		 XmlUtilities
         $ENV{XERCES_C_LIB}/libxerces-c.so
         MessageFacility
         ${MF_UTILITIES}
)

install_headers()
install_source()


