include_directories($ENV{XERCES_C_INC})
cet_make(LIBRARY_NAME MacroMaker
		 LIBRARIES
		 artdaq_core::artdaq-core_Utilities
		 artdaq_DAQdata
		 ${XDAQ_BASIC_LIB_LIST}
		otsdaq::ConfigurationInterface
		 otsdaq::TableCore
		 otsdaq::CoreSupervisors
		 otsdaq::CgiDataUtilities
		 otsdaq::SOAPUtilities
		 otsdaq::FiniteStateMachine
		 otsdaq::XmlUtilities
         MF_MessageLogger
         otsdaq::CodeEditor
		 otsdaq::WebUsersUtilities
		 otsdaq::SupervisorInfo
		 otsdaq::FECore
         otsdaq::Macros
         ${Boost_SYSTEM_LIBRARY}
)

install_headers()
install_source()
