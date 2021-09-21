cet_make(LIBRARY_NAME ConfigurationGUI
		 LIBRARIES
		 artdaq_core::artdaq-core_Utilities
		 ${XDAQ_BASIC_LIB_LIST}
		 cetlib_except
		 otsdaq::ConfigurationInterface
		 otsdaq::TableCore
		 otsdaq::TablePlugins_IterateTable_table
		 otsdaq::TablePlugins_XDAQContextTable_table
		 otsdaq::ARTDAQTableBase
		 otsdaq::FiniteStateMachine
		 otsdaq::CoreSupervisors
		 otsdaq::SupervisorInfo
		 otsdaq::GatewaySupervisor
		 otsdaq::ARTDAQSupervisor
		 otsdaq::CgiDataUtilities
		 otsdaq::WebUsersUtilities
		 otsdaq::XmlUtilities
		 otsdaq::Macros
		cetlib
	 	 MF_MessageLogger
)

install_headers()
install_source()
