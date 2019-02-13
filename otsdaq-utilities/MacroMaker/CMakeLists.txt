include_directories($ENV{XERCES_C_INC})
cet_make(LIBRARY_NAME MacroMaker
		 LIBRARIES
		 artdaq-core_Utilities
		 ${XDAQ_BASIC_LIB_LIST}
		 ConfigurationInterface
		 ConfigurationDataFormats
		 CoreSupervisors
		 CgiDataUtilities
		 SOAPUtilities
		 FiniteStateMachine
		 XmlUtilities
         MF_MessageLogger
         CodeEditor
)

install_headers()
install_source()
