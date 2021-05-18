include_directories(${MPI_INCLUDE_PATH})
include_directories($ENV{XERCES_C_INC})

cet_make(LIBRARY_NAME Visualization
		 LIBRARIES
		 artdaq_core::artdaq-core_Utilities
		 ${XDAQ_BASIC_LIB_LIST}
		 ${ROOT_GUI_LIB_LIST}
		 ${ROOTSYS}/lib/libRHTTP.so
         ${Boost_REGEX_LIBRARY}
		 otsdaq::CoreSupervisors
		 otsdaq::RootUtilities
		 otsdaq::CgiDataUtilities
		 otsdaq::FiniteStateMachine
		 otsdaq::WebUsersUtilities
		 #MonicelliInterface
		 otsdaq::SupervisorInfo
		 otsdaq::ConfigurationInterface
		 otsdaq::Macros
		 otsdaq::TableCore
		 #otsdaq_TablePlugins_DataManagerConfiguration_table
		 #otsdaq_TablePlugins_DataBufferConfiguration_table
		 otsdaq_TablePlugins_XDAQContextTable_table
		 otsdaq_DataProcessorPlugins_RawDataVisualizerConsumer_processor
		 otsdaq::XmlUtilities
		 otsdaq::SOAPUtilities
		 MF_MessageLogger

)


install_headers()
install_source()
