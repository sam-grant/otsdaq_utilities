#include "otsdaq-utilities/ECLWriter/NovaECLWriter.h"
#include "otsdaq-utilities/ECLWriter/ECLConnection.h"

RCECLWriter::RCECLWriter(std::string _ECLUser, std::string _ECLHost,
						 std::string _ECLPwd,
						 bool atEnd, int run, int subrun, int detId,
						 std::string cfgName,
						 int partition,
						 std::string shifter, int runType, int nActiveChan,
						 std::string comment, std::string runDuration) :

	_atEnd(atEnd), _eclUser(_ECLUser), _eclHost(_ECLHost), _eclPwd(_ECLPwd),
	_cfgName(cfgName), _shifter(shifter), _comment(comment), _duration(runDuration),
	_run(run), _subrun(subrun)
{

}

//************************************************************

int RCECLWriter::Write()
{
	ECLEntry_t eclEntry;
	eclEntry.author(_eclUser);
	eclEntry.category("Run History");
	char buff[256];
	Form_t form;
	Field_t field;
	Form_t::field_sequence fields;

	if (!_atEnd) {
		form.name("RC Start Run");

		sprintf(buff, "%d", _run);
		field = Field_t(buff, "RunNumber");
		fields.push_back(field);

		field = Field_t(_cfgName, "DAQConfig");
		fields.push_back(field);
		
		field = Field_t(EscapeECLString(_shifter), "Shifter");
		fields.push_back(field);
		
		field = Field_t(EscapeECLString(_comment), "Comments");
		fields.push_back(field);

		form.field(fields);

		eclEntry.form(form);

	}
	else {
		form.name("RC End Run");

		sprintf(buff, "%d", _run);
		field = Field_t(buff, "RunNumber");
		fields.push_back(field);

		sprintf(buff, "%d", _subrun);
		field = Field_t(buff, "SubrunNumber");
		fields.push_back(field);


		field = Field_t(EscapeECLString(_duration), "Duration");
		fields.push_back(field);

		field = Field_t(EscapeECLString(_shifter), "Shifter");
		fields.push_back(field);

		field = Field_t(EscapeECLString(_comment), "Comments");
		fields.push_back(field);

		form.field(fields);

		eclEntry.form(form);
	}

	ECLConnection eclConn(_eclUser, _eclPwd, _eclHost);
	if (!eclConn.Post(eclEntry)) {
		return -1;
	}

	return 0;
}

//************************************************************

std::string RCECLWriter::EscapeECLString(std::string input)
{
	std::string output = input;
	size_t pos = output.find('&');
	while (pos != std::string::npos)
	{
		output = output.replace(pos, 1, "&amp;");
		pos = output.find('&', pos + 2);
	}

	pos = output.find('"');
	while (pos != std::string::npos)
	{
		output = output.replace(pos, 1, "&quot;");
		pos = output.find('"', pos + 1);
	}

	pos = output.find('\'');
	while (pos != std::string::npos)
	{
		output = output.replace(pos, 1, "&apos;");
		pos = output.find('\'', pos + 1);
	}

	pos = output.find('<');
	while (pos != std::string::npos)
	{
		output = output.replace(pos, 1, "&lt;");
		pos = output.find('<', pos + 1);
	}

	pos = output.find('>');
	while (pos != std::string::npos)
	{
		output = output.replace(pos, 1, "&gt;");
		pos = output.find('>', pos + 1);
	}

	return output;
}
