
include_directories($ENV{XERCES_C_INC})

cet_make(LIBRARY_NAME ControlsDashboard
		 LIBRARIES
		 artdaq-core_Utilities
		 ${XDAQ_BASIC_LIB_LIST}
		 ConfigurationInterface
		 TableCore		 
		 PluginMakers
		 XmlUtilities
		 CgiDataUtilities
		 FiniteStateMachine
		 
		 CoreSupervisors
         
	     ${MF_MESSAGELOGGER}
	     ${MF_UTILITIES}
)

install_headers()
install_source()

