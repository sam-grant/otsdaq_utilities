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
		 TableCore
		 #otsdaq-core_TablePlugins_DataManagerConfiguration_table
		 #otsdaq-core_TablePlugins_DataBufferConfiguration_table
		 otsdaq-core_TablePlugins_XDAQContextTable_table
		 otsdaq-core_DataProcessorPlugins_RawDataVisualizerConsumer_processor
		 XmlUtilities
		 SOAPUtilities
		 MF_MessageLogger

)


install_headers()
install_source()
