#ifndef OTSDAQ_UTILITIES_ECLWRITER_NOVAECLWRITER_H
#define OTSDAQ_UTILITIES_ECLWRITER_NOVAECLWRITER_H

#include <string>

class RCECLWriter
{

public:
	RCECLWriter(std::string _ECLUser, std::string _ECLHost, std::string _ECLPwd,
				bool atEnd, int run, int subrun, int detId, std::string cfgName,
				int partition, std::string shifter, int runType,
				int nActiveChan, std::string comment,
				std::string runDuration = "");
	~RCECLWriter() {};

	std::string EscapeECLString(std::string input = "");
	
	int Write();
	
public:
	bool _atEnd;

	std::string _eclUser;
	std::string _eclHost;
	std::string _eclPwd;
	std::string _cfgName;
	std::string _shifter;
	std::string _comment;
	std::string _duration;
	int _run;
	int _subrun;
};

#endif // OTSDAQ_UTILITIES_LOGBOOK_NOVAECLWRITER_H
