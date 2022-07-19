
cet_make_library(LIBRARY_NAME Logbook
SOURCE LogbookSupervisor.cc version.cc
		 LIBRARIES PUBLIC
		 otsdaq::CoreSupervisors
)

install_headers()
install_source()
