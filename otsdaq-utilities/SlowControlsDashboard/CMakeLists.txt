
include_directories($ENV{XERCES_C_INC})

cet_make(LIBRARY_NAME SlowControlsDashboard
		 LIBRARIES
		 artdaq-core_Utilities
		 ${XDAQ_BASIC_LIB_LIST}
		 ConfigurationInterface
		 TableCore		 
		 PluginMakers
		 XmlUtilities
		 NetworkUtilities
		 CgiDataUtilities
		 FiniteStateMachine
		 CoreSupervisors
         Macros
		 WebUsersUtilities
		 otsdaq_TablePlugins_XDAQContextTable_table
	     ${MF_MESSAGELOGGER}
	     ${MF_UTILITIES}
		 ${Boost_REGEX_LIBRARY}
)

install_headers()
install_source()

