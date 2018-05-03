
include_directories($ENV{XERCES_C_INC})

cet_make(LIBRARY_NAME ControlsDashboard
		 LIBRARIES
		 artdaq-core_Utilities
		 ${XDAQ_BASIC_LIB_LIST}
		 ConfigurationInterface
		 ConfigurationDataFormats		 
		 PluginMakers
		 
		 CoreSupervisors
         $ENV{XERCES_C_LIB}/libxerces-c.so     
         
	     ${MF_MESSAGELOGGER}
	     ${MF_UTILITIES}
)

install_headers()
install_source()

