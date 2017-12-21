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
		 SupervisorDescriptorInfo
		 ConfigurationInterface
		 ConfigurationDataFormats
		 #otsdaq-core_ConfigurationPluginDataFormats_DataManagerConfiguration_configuration
		 #otsdaq-core_ConfigurationPluginDataFormats_DataBufferConfiguration_configuration
		 otsdaq-core_ConfigurationPluginDataFormats_XDAQContextConfiguration_configuration
		 otsdaq-demo_DataProcessorPlugins_RawDataVisualizerConsumer_processor
		 XmlUtilities
		 SOAPUtilities
		 MessageFacility
         $ENV{XERCES_C_LIB}/libxerces-c.so
         
)


install_headers()
install_source()
