cet_make_library(LIBRARY_NAME Chat
SOURCE ChatSupervisor.cc version.cc
		 LIBRARIES PUBLIC
		 otsdaq::CoreSupervisors
)

install_headers()
install_source()
