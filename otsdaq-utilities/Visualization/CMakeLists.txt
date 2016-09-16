include_directories(${MPI_INCLUDE_PATH})
include_directories($ENV{XERCES_C_INC})

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
		 otsdaq-core_ConfigurationPluginDataFormats_DataManagerConfiguration_configuration
		 otsdaq-core_ConfigurationPluginDataFormats_DataBufferConfiguration_configuration
		 XmlUtilities
		 SOAPUtilities
		 MessageFacility
         $ENV{XERCES_C_LIB}/libxerces-c.so
         
)


install_headers()
install_source()
