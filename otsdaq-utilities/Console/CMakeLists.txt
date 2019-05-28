cet_make_exec(otsConsoleFwd SOURCE MF_ReceiveAndForward.cpp)


#For Console Supervisor
include_directories($ENV{XERCES_C_INC})
cet_make(LIBRARY_NAME Console
		 LIBRARIES
		 artdaq-core_Utilities
		 ${XDAQ_BASIC_LIB_LIST}
		 SupervisorInfo
		${MF_MESSAGELOGGER}
		${MF_UTILITIES}
		FiniteStateMachine
		CoreSupervisors
		 XmlUtilities
		 SOAPUtilities
		 CgiDataUtilities
		 WebUsersUtilities
#		 MessageFacility
		 NetworkUtilities
         Macros
)

install_headers()
install_source()
