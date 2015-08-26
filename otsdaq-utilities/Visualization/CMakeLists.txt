cet_make(LIBRARY_NAME Visualization
		 LIBRARIES
		 ${XDAQ_BASIC_LIB_LIST}
		 ${ROOT_GUI_LIB_LIST}
		 ${ROOTSYS}/lib/libRHTTP.so
		 RootUtilities
		 DataManager
		 CgiDataUtilities
		 FiniteStateMachine
		 WebUsersUtilities
		 MonicelliInterface
		 SupervisorConfigurations
		 ConfigurationInterface
		 XmlUtilities
		 SOAPUtilities
)


install_headers()
install_source()
