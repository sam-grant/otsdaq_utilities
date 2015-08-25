cet_make(LIBRARY_NAME ConfigurationGUI
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
