include_directories($ENV{XERCES_C_INC})
cet_make(LIBRARY_NAME MacroMaker
		 LIBRARIES
		 artdaq-core_Utilities
		 artdaq_DAQdata
		 ${XDAQ_BASIC_LIB_LIST}
		 ConfigurationInterface
		 TableCore
		 CoreSupervisors
		 CgiDataUtilities
		 SOAPUtilities
		 FiniteStateMachine
		 XmlUtilities
         MF_MessageLogger
         CodeEditor
		 FECore
         Macros
         ${Boost_SYSTEM_LIBRARY}
)

install_headers()
install_source()
