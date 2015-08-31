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
)

install_headers()
install_source()


