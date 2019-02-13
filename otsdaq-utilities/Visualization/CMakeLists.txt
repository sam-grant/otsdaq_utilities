include_directories(${MPI_INCLUDE_PATH})
include_directories($ENV{XERCES_C_INC})

cet_make(LIBRARY_NAME Visualization
		 LIBRARIES
		 artdaq-core_Utilities
		 ${XDAQ_BASIC_LIB_LIST}
		 ${ROOT_GUI_LIB_LIST}
		 ${ROOTSYS}/lib/libRHTTP.so
		 CoreSupervisors
		 RootUtilities
		 DataManager
		 CgiDataUtilities
		 FiniteStateMachine
		 WebUsersUtilities
		 #MonicelliInterface
		 SupervisorInfo
		 ConfigurationInterface
		 Macros
		 ConfigurationDataFormats
		 #otsdaq-core_ConfigurationPluginDataFormats_DataManagerConfiguration_configuration
		 #otsdaq-core_ConfigurationPluginDataFormats_DataBufferConfiguration_configuration
		 otsdaq-core_ConfigurationPluginDataFormats_XDAQContextConfiguration_configuration
		 otsdaq-core_DataProcessorPlugins_RawDataVisualizerConsumer_processor
		 XmlUtilities
		 SOAPUtilities
		 MF_MessageLogger

)


install_headers()
install_source()
