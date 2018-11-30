include_directories($ENV{XERCES_C_INC})
cet_make(LIBRARY_NAME MacroMaker
		 LIBRARIES
		 artdaq-core_Utilities
		 ${XDAQ_BASIC_LIB_LIST}
		 CoreSupervisors
		 CgiDataUtilities
		 SOAPUtilities
		 FiniteStateMachine
		 XmlUtilities
         
         CodeEditor
)

install_headers()
install_source()
