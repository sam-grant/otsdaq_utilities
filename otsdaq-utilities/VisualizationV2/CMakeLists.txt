include_directories(${MPI_INCLUDE_PATH})
include_directories($ENV{XERCES_C_INC})

cet_make(LIBRARY_NAME VisualizationV2
		 LIBRARIES
		 artdaq-core_Utilities
		 ${XDAQ_BASIC_LIB_LIST}
		 ${ROOT_GUI_LIB_LIST}
		 ${ROOTSYS}/lib/libRHTTP.so
         ${Boost_REGEX_LIBRARY}
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
		 TableCore
		 #otsdaq_TablePlugins_DataManagerConfiguration_table
		 #otsdaq_TablePlugins_DataBufferConfiguration_table
		 otsdaq_TablePlugins_XDAQContextTable_table
		 otsdaq_DataProcessorPlugins_RawDataVisualizerConsumer_processor
		 XmlUtilities
		 SOAPUtilities
		 MF_MessageLogger

)


install_headers()
install_source()
