include_directories($ENV{XERCES_C_INC})
cet_make(LIBRARY_NAME ConfigurationGUI
		 LIBRARIES
		 artdaq-core_Utilities
		 ${XDAQ_BASIC_LIB_LIST}
		 ${CETLIB_EXCEPT}
		 ConfigurationInterface
		 TableCore
		 otsdaq_TablePlugins_IterateTable_table
		 FiniteStateMachine
		 CoreSupervisors
		 SupervisorInfo
		 GatewaySupervisor
		 CgiDataUtilities
		 WebUsersUtilities
		 XmlUtilities
		 Macros
		 cetlib
	 	 ${MF_MESSAGELOGGER}
)

install_headers()
install_source()
