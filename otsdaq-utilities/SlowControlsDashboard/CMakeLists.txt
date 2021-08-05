
cet_make(LIBRARY_NAME SlowControlsDashboard
		 LIBRARIES
		 artdaq_core::artdaq-core_Utilities
		 ${XDAQ_BASIC_LIB_LIST}
		 otsdaq::ConfigurationInterface
		 otsdaq::TableCore		 
		 otsdaq::PluginMakers
		 otsdaq::XmlUtilities
		 otsdaq::NetworkUtilities
		 otsdaq::CgiDataUtilities
		 otsdaq::FiniteStateMachine
		 otsdaq::CoreSupervisors
        otsdaq::Macros
		 otsdaq::WebUsersUtilities
		 otsdaq_TablePlugins_XDAQContextTable_table
	     MF_MessageLogger
	     ${MF_UTILITIES}
		 ${Boost_REGEX_LIBRARY}
)

install_headers()
install_source()

