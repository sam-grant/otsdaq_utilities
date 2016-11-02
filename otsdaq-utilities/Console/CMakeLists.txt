cet_make_exec(mf_rcv_n_fwd SOURCE MF_ReceiveAndForward.cpp)


#For Console Supervisor
include_directories($ENV{XERCES_C_INC})
cet_make(LIBRARY_NAME Console
		 LIBRARIES
		 ${XDAQ_BASIC_LIB_LIST}
		 SupervisorDescriptorInfo
		 XmlUtilities
		 SOAPUtilities
		 CgiDataUtilities
		 WebUsersUtilities
		 MessageFacility
		 NetworkUtilities
         $ENV{XERCES_C_LIB}/libxerces-c.so
)

install_headers()
install_source()
