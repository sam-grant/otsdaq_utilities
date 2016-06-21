include_directories($ENV{XERCES_C_INC})
cet_make(LIBRARY_NAME Logbook
		 LIBRARIES
		 ${XDAQ_BASIC_LIB_LIST}
		 SupervisorConfigurations
		 XmlUtilities
		 SOAPUtilities
		 CgiDataUtilities
		 WebUsersUtilities
		 MessageFacility
         $ENV{XERCES_C_LIB}/libxerces-c.so
)

install_headers()
install_source()
