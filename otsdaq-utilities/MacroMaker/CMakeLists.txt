cet_make_library(LIBRARY_NAME MacroMaker
SOURCE MacroMakerSupervisor.cc version.cc
		 LIBRARIES PUBLIC
		 otsdaq::CoreSupervisors
		 PRIVATE
		 artdaq::DAQdata
		 otsdaq::CodeEditor
)

install_headers()
install_source()
