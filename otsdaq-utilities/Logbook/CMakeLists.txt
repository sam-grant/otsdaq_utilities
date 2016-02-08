cet_make(LIBRARY_NAME Logbook
		 LIBRARIES
		 ${XDAQ_BASIC_LIB_LIST}
		 SupervisorConfigurations
		 XmlUtilities
		 SOAPUtilities
		 CgiDataUtilities
		 WebUsersUtilities
         $ENV{XERCES_C_LIB}/libxerces-c.so
)

install_headers()
install_source()
