include_directories($ENV{XERCES_C_INC})
cet_make(LIBRARY_NAME Chat
		 LIBRARIES
		 ${XDAQ_BASIC_LIB_LIST}
		 SupervisorConfigurations
		 CgiDataUtilities
		 XmlUtilities
		 WebUsersUtilities
         SOAPUtilities
	 MessageFacility
         $ENV{XERCES_C_LIB}/libxerces-c.so
)

install_headers()
install_source()
