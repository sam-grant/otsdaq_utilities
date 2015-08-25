cet_make(LIBRARY_NAME Chat
		 LIBRARIES
		 ${XDAQ_BASIC_LIB_LIST}
		 SupervisorConfigurations
		 CgiDataUtilities
		 XmlUtilities
		 WebUsersUtilities
         SOAPUtilities
)

install_headers()
install_source()
